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
	ifstream* ifs;
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

void gen_divided_sort_file(string file_read_name, string file_write_name, unsigned int start, unsigned int end) {

	ifstream ifs(file_read_name, ios::binary | ios::in);
	int len = end - start;
	if (ifs.is_open()) {
		unsigned int offset = start * 100;
		ifs.seekg(offset);
		ifs.read(&kv[0].key[0], sizeof(struct KeyValue) * len);
	}
	ifs.close();

	sort(kv, kv + len, cmp);

	ofstream ofs(file_write_name, ios::binary | ios::out);
	ofs.write((char*)&kv[0].key[0], sizeof(struct KeyValue) * len);
	ofs.close();
}

void load_data_at() {

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

	for (int i = 0; i < n_threads; i++) {
		unsigned int start = i * block_size;
		unsigned int end = min((i + 1) * block_size, cnt_keyvalue);
		string file_write_name = gen_tmp_name(i);
		remove(file_write_name.c_str());
		gen_divided_sort_file(file_read_name, file_write_name, i * block_size, end);
		thread t = thread(load_data_at /* todo */);
		vt.push_back(move(t));
	}

	for (unsigned int i = 0; i < vt.size(); i++) {
		vt.at(i).join();
	}

	int cnt_file = n_threads;
	// merge sort by divided file
	priority_queue<KeyValueNode> pq;

	int each_space = LIMIT_BY_RAM_BLOCK_SIZE / (cnt_file + 1);

	ofstream ofs(file_write_name, ios::binary | ios::out);
	for (int i = 0; i < cnt_file; i++) {
		KeyValueNode kvn;
		kvn.file_idx = i;
		kvn.ifs = new ifstream(gen_tmp_name(i), ios::binary | ios::in);
		kvn.ifs->seekg(0, ios::end);
		kvn.cur_g = 0;
		kvn.end_g = (long long)kvn.ifs->tellg() / (long long)sizeof(struct KeyValue);

		int read_size =  min(each_space, kvn.end_g - kvn.cur_g) * sizeof(struct KeyValue);
		kvn.offset = kvn.cur_g + min(each_space, kvn.end_g - kvn.cur_g);	
		kvn.ifs->seekg(0);
# if 1
		kvn.ifs->read(&kv[i * each_space].key[0], read_size);
		kvn.kv = kv + (i * each_space);
#else
		kvn.ifs->read(&kvn.key[0], sizeof(struct KeyValue));
#endif

		pq.push(std::move(kvn));
	}

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
			delete kvn.ifs;
			remove(gen_tmp_name(kvn.file_idx).c_str());
			continue;
		}
#if 1
		if (kvn.cur_g == kvn.offset) {
			int read_size = min(each_space, kvn.end_g - kvn.cur_g) * sizeof(struct KeyValue);
			kvn.offset = kvn.cur_g + min(each_space, kvn.end_g - kvn.cur_g);
			kvn.ifs->seekg(kvn.cur_g * sizeof(struct KeyValue));
			kvn.ifs->read(&kv[kvn.file_idx * each_space].key[0], read_size);
		}
		kvn.kv = kv + (kvn.file_idx * each_space + kvn.cur_g % each_space);
#else
		unsigned offset = kvn.cur_g * 100;
		kvn.ifs->seekg(offset);
		kvn.ifs->read(&kvn.key[0], sizeof(struct KeyValue));
#endif
		pq.push(std::move(kvn));
	}
	if (output_g > 0) {
		ofs.write((char*)&kv[output_idx].key[0], sizeof(struct KeyValue) * output_g);
	}

	ofs.close();
	return 0;
}

