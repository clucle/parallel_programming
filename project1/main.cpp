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
	return memcmp(me.key, other.key, SIZE_KEY) <= 0;
}

struct KeyValueNode {
	char key[10];
	char payload[90];
	int file_idx = 0;
	int cur_g = 0;
	int end_g = 0;
	ifstream* ifs;
};

bool operator<(const KeyValueNode &me, const KeyValueNode &other) {
	return memcmp(me.key, other.key, SIZE_KEY) > 0;
}

const int LIMIT_BY_RAM_BLOCK_SIZE = 19000000;

mutex g_key_mutex;
sem_t sem;

string gen_tmp_name(int idx) {
	return "tmp" + to_string(idx) + ".data";
}

void gen_divided_sort_file(string file_read_name, int idx, unsigned int start, unsigned int end) {

	sem_wait(&sem);
	struct KeyValue kv;
	vector<KeyValue> v;
	ifstream ifs(file_read_name, ios::binary | ios::in);
	if (ifs.is_open()) {
		for (unsigned i = start; i < end; i++) {
			unsigned int offset = i * 100;
			ifs.seekg(offset);
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			v.push_back(kv);
		}
	}
	ifs.close();

	sort(v.begin(), v.end(), cmp);

	remove(gen_tmp_name(idx).c_str());
	ofstream ofs(gen_tmp_name(idx), ios::binary | ios::out);
	for (auto e: v) {
		ofs.write((char*)&e, sizeof(struct KeyValue));
	}
	ofs.close();

	v.clear();

	sem_post(&sem);
}

int main(int argc, char* argv[]) {
	ios::sync_with_stdio(false);

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
	int block_size = cnt_keyvalue / MAX_NUM_THREADS;

	while (block_size * MAX_NUM_THREADS >= LIMIT_BY_RAM_BLOCK_SIZE) {
		block_size /= 2;
		block_size -= block_size % 100;
	}

	// generate divdided file
	sem_init(&sem, 0, MAX_NUM_THREADS);

	vector<thread> vt;
	for (int i = 0; ; i++) {
		unsigned int start = i * block_size;
		unsigned int end = min((i + 1) * block_size, cnt_keyvalue);
		thread t = thread(gen_divided_sort_file, file_read_name, i, i * block_size, end);
		vt.push_back(std::move(t));
		if (end == cnt_keyvalue) break;
	}


	int cnt_file = vt.size();

	for (unsigned int i = 0; i < vt.size(); ++i) {
		vt.at(i).join();
	}

	sem_destroy(&sem);

	// merge sort by divided file
	priority_queue<KeyValueNode> pq;

	ofstream ofs(file_write_name, ios::binary | ios::out);
	for (int i = 0; i < cnt_file; i++) {
		KeyValueNode kvn;
		kvn.file_idx = i;
		kvn.ifs = new ifstream(gen_tmp_name(i), ios::binary | ios::in);
		kvn.ifs->seekg(0, ios::end);
		kvn.cur_g = 0;
		kvn.end_g = (long long)kvn.ifs->tellg() / (long long)sizeof(struct KeyValue);
		kvn.ifs->seekg(0);
		kvn.ifs->read(&kvn.key[0], sizeof(struct KeyValue));
		pq.push(std::move(kvn));
	}

	while (!pq.empty()) {
		auto kvn = pq.top();
		pq.pop();
		ofs.write((char*)&kvn, sizeof(struct KeyValue));
		kvn.cur_g++;
		if (kvn.cur_g == kvn.end_g) {
			delete kvn.ifs;
			remove(gen_tmp_name(kvn.file_idx).c_str());
			continue;
		}
		unsigned offset = kvn.cur_g * 100;
		kvn.ifs->seekg(offset);
		kvn.ifs->read(&kvn.key[0], sizeof(struct KeyValue));
		pq.push(std::move(kvn));
	}

	ofs.close();
	return 0;
}

