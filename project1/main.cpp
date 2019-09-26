#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <cstdio>
#include <chrono>
#include <queue>
#include <condition_variable>

#define MAX_NUM_THREADS 40
#define SIZE_KEY 10

using namespace std;

struct KeyValue {
	char key[10];
	char payload[90];
};


bool cmp(const KeyValue& me, const KeyValue &other) {
	return memcmp(me.key, other.key, SIZE_KEY) < 0;
}

struct KeyValueNode {
	struct KeyValue *kv;
	int file_idx = 0;
	int cur_g = 0;
	int end_g = 0;
	int offset = 0;
};

bool operator<(const KeyValueNode &me, const KeyValueNode &other) {
	return memcmp(me.kv->key, other.kv->key, SIZE_KEY) > 0;
}

const int LIMIT_BY_RAM_BLOCK_SIZE = 19000000;


mutex g_main_mutex;
condition_variable g_main_cv;
thread g_load_thread[MAX_NUM_THREADS];
mutex g_load_mutex;
condition_variable g_load_cv;
string g_file_read_name;
unsigned int g_kv_idx[MAX_NUM_THREADS];
unsigned int g_kv_idx_file[MAX_NUM_THREADS];
unsigned int g_kv_cnt[MAX_NUM_THREADS];
int g_kv_worked;

string gen_tmp_name(int idx) {
	return "tmp" + to_string(idx) + ".data";
}

struct KeyValue kv[19000001];
priority_queue<KeyValueNode> pq;

void load_file(int tid) {
	while (1) {
		{	
			unique_lock<mutex> lk(g_load_mutex);
			g_load_cv.wait(lk);
			lk.unlock();
			if (g_kv_idx[0] == -1) return ;
		}
		// init
		string file_read_name = g_file_read_name;
		unsigned int idx = g_kv_idx[tid];
		unsigned int idx_file = g_kv_idx_file[tid];
		unsigned int cnt = g_kv_cnt[tid];

		// load file
		ifstream ifs(file_read_name, ios::binary | ios::in);
		unsigned int offset = idx_file * sizeof(KeyValue);
		ifs.seekg(offset);
		ifs.read(&kv[idx].key[0], sizeof(struct KeyValue) * cnt);
		ifs.close();
		__atomic_fetch_add(&g_kv_worked, 1, __ATOMIC_SEQ_CST);
		if (g_kv_worked == MAX_NUM_THREADS) {
			unique_lock<mutex> lk(g_main_mutex);
			g_main_cv.notify_one();
			lk.unlock();
		}
	}
}

void load_file_multi_thread(string file_read_name, unsigned int idx, unsigned int idx_file, unsigned int cnt) {	
	unique_lock<mutex> lk(g_main_mutex);
	int cnt_per_thread = cnt / MAX_NUM_THREADS;
	g_file_read_name = file_read_name;
	for (int i = 0; i < MAX_NUM_THREADS - 1; i++) {
		int pos = i * cnt_per_thread;
		g_kv_idx[i] = idx + pos;
		g_kv_idx_file[i] = idx_file + pos;
		g_kv_cnt[i] = cnt_per_thread;
	}
	int pos = (MAX_NUM_THREADS - 1) * cnt_per_thread;
	int last_cnt = cnt - cnt_per_thread * (MAX_NUM_THREADS - 1);
	g_kv_idx[MAX_NUM_THREADS - 1] = idx + pos;
	g_kv_idx_file[MAX_NUM_THREADS - 1] = idx_file + pos;
	g_kv_cnt[MAX_NUM_THREADS - 1] = last_cnt;

	g_kv_worked = 0;
	g_load_cv.notify_all();
	g_main_cv.wait(lk);
	lk.unlock();
}

void gen_divided_sort_file(string file_read_name, string file_write_name, unsigned int start, unsigned int end) {
	int cnt = end - start;
	load_file_multi_thread(file_read_name, 0, start, cnt);
	sort(kv, kv + cnt, cmp);
	ofstream ofs(file_write_name, ios::binary | ios::out);
	ofs.write((char*)&kv[0].key[0], sizeof(struct KeyValue) * cnt);
	ofs.close();
}

void load_data_at_and_push_pq(int file_idx, int each_space) {
	KeyValueNode kvn;
	kvn.file_idx = file_idx;
	ifstream ifs(gen_tmp_name(kvn.file_idx), ios::binary | ios::in);
	ifs.seekg(0, ios::end);
	kvn.cur_g = 0;
	kvn.end_g = (long long)ifs.tellg() / (long long)sizeof(struct KeyValue);
	ifs.close();
	int cnt =  min(each_space, kvn.end_g - kvn.cur_g);
	kvn.offset = kvn.cur_g + min(each_space, kvn.end_g - kvn.cur_g);
	load_file_multi_thread(gen_tmp_name(kvn.file_idx), kvn.file_idx * each_space, 0, cnt);
	kvn.kv = kv + (kvn.file_idx * each_space);
	pq.push(std::move(kvn));
}

void join_global_load_thread() {
	g_kv_idx[0] = -1;
	g_load_cv.notify_all();
	for (int i = 0; i < MAX_NUM_THREADS; i++) {
		g_load_thread[i].join();
	}
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		cout << "usage: ./run InputFile OutputFile\n";
		return 0;
	}

	for (int i = 0; i < MAX_NUM_THREADS; i++) {
		g_load_thread[i] = thread(load_file, i);
	}
	string file_read_name = argv[1];
	string file_write_name = argv[2];
	remove(file_write_name.c_str());

	ifstream ifs(file_read_name, ios::binary | ios::ate);
	ifs.seekg(0, ios::end);
	int cnt_keyvalue = (long long)ifs.tellg() / (long long)sizeof(struct KeyValue);
	ifs.close();

	if (cnt_keyvalue <= 15000000) {
		gen_divided_sort_file(file_read_name, file_write_name, 0, cnt_keyvalue);
		join_global_load_thread();
		return 0;
	}

	int n_threads = 2;
	int block_size = cnt_keyvalue / n_threads;
	while (block_size >= LIMIT_BY_RAM_BLOCK_SIZE) {
		n_threads *= 2;
		block_size = cnt_keyvalue / n_threads;
	}


	vector<thread> vt;
	int cnt_file = n_threads;
	int each_space = LIMIT_BY_RAM_BLOCK_SIZE / (cnt_file + 1);

	for (int i = 0; i < cnt_file; i++) {
		unsigned int start = i * block_size;
		unsigned int end = min((i + 1) * block_size, cnt_keyvalue);
		string file_write_name = gen_tmp_name(i);
		remove(file_write_name.c_str());
		gen_divided_sort_file(file_read_name, file_write_name, start, end);
	}

	for (int i = 0; i < cnt_file; i++) {
		load_data_at_and_push_pq(i, each_space);
	}

	// merge sort by divided file

	ofstream ofs(file_write_name, ios::binary | ios::out);
	int output_g = 0;
	int output_idx = each_space * cnt_file;

	while (!pq.empty()) {
		auto kvn = pq.top();
		pq.pop();
		memcpy(&kv[output_idx + output_g], &(*(kvn.kv)), sizeof(struct KeyValue));
		output_g++;
		if (output_g == each_space) {
			ofs.write((char*)&kv[output_idx].key[0], sizeof(struct KeyValue) * output_g);
			output_g = 0;
		}
		kvn.cur_g++;
		if (kvn.cur_g == kvn.end_g) {
			// delete kvn.ifs;
			remove(gen_tmp_name(kvn.file_idx).c_str());
			continue;
		}
		if (kvn.cur_g == kvn.offset) {
			int cnt = min(each_space, kvn.end_g - kvn.cur_g);
			kvn.offset = kvn.cur_g + min(each_space, kvn.end_g - kvn.cur_g);
			load_file_multi_thread(gen_tmp_name(kvn.file_idx), kvn.file_idx * each_space, kvn.cur_g, cnt);
		}
		kvn.kv = kv + (kvn.file_idx * each_space + kvn.cur_g % each_space);
		pq.push(std::move(kvn));
	}
	if (output_g > 0) {
		ofs.write((char*)&kv[output_idx].key[0], sizeof(struct KeyValue) * output_g);
	}

	ofs.close();
	join_global_load_thread();
	return 0;
}

