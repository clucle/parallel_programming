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
#include <semaphore.h>
#include <queue>

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

mutex g_key_mutex;
sem_t sem;

string gen_tmp_name(int idx) {
	return "tmp" + to_string(idx) + ".data";
}

struct KeyValue kv[19000001];
priority_queue<KeyValueNode> pq;

void load_file(string file_read_name, unsigned int idx, unsigned int idx_file, unsigned int cnt) {
	ifstream ifs(file_read_name, ios::binary | ios::in);
	unsigned int offset = idx_file * sizeof(KeyValue);
	ifs.seekg(offset);
	ifs.read(&kv[idx].key[0], sizeof(struct KeyValue) * cnt);
	ifs.close();
}

void load_file_multi_thread(string file_read_name, unsigned int idx, unsigned int idx_file, unsigned int cnt) {
	thread t[MAX_NUM_THREADS];
	int cnt_per_thread = cnt / MAX_NUM_THREADS;
	for (int i = 0; i < MAX_NUM_THREADS - 1; i++) {
		int pos = i * cnt_per_thread;
		t[i] = thread(load_file, file_read_name, idx + pos, idx_file + pos, cnt_per_thread);
	}
	int pos = (MAX_NUM_THREADS - 1) * cnt_per_thread;
	int last_cnt = cnt - cnt_per_thread * (MAX_NUM_THREADS - 1);
	t[MAX_NUM_THREADS - 1] = thread(load_file, file_read_name, idx + pos, idx_file + pos, last_cnt);

	for (int i = 0; i < MAX_NUM_THREADS; i++) {
		t[i].join();
	}
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

int main(int argc, char* argv[]) {

	if (argc < 3) {
		cout << "usage: ./run InputFile OutputFile\n";
		return 0;
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
	return 0;
}

