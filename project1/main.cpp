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

#define NUM_THREADS 4

using namespace std;

struct KeyValue {
	char key[10];
	char payload[90];
};

struct Key {
	char key[10];
	int idx;
};


const int SIZE_KEY = 10;
bool operator< (const Key& me, const Key &other) {
	return memcmp(me.key, other.key, SIZE_KEY) <= 0;
}

vector<Key> v;
mutex g_key_mutex;

void file_read_and_append(string file_read_name, int start, int cnt) {
	ifstream ifs(file_read_name, ios::binary | ios::in);
	vector<Key> tmp;
	int idx = 0;
	struct KeyValue kv;
	struct Key k;
	if (ifs.is_open()) {
		ifs.seekg(start * 100);
		for (int i = 0; i < cnt; i++) {
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			memcpy(&k.key[0], &kv.key[0], sizeof(struct Key));
			k.idx = start + idx++;
			tmp.push_back(k);
		}
	}
	ifs.close();
	lock_guard<mutex> guard(g_key_mutex);
	v.insert(v.end(), tmp.begin(), tmp.end());
}

void file_write_at(string file_write_name, string file_read_name, int start, int cnt) {
	struct KeyValue kv;

	g_key_mutex.lock();
	ofstream ofs(file_write_name, ios::binary | ios::out);	
	// ofs.seekp(0);
	g_key_mutex.unlock();
	ifstream ifs(file_read_name, ios::binary | ios::in);
	vector<Key>::iterator from = v.begin() + start;
	vector<Key>::iterator to = v.begin() + start + cnt;
	for (auto iter = from; iter != to; ++iter) {
		ifs.seekg((*iter).idx * 100);
		ifs.read(&kv.key[0], sizeof(struct KeyValue));
		ofs.write((char*)&kv, sizeof(struct KeyValue));
	}
	ifs.close();
	ofs.close();
}

string gen_tmp_file_name(string s, int i) {
	return s + to_string(i) + ".data";
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
	int cnt = ifs.tellg() / sizeof(struct KeyValue);
	ifs.close();

	bool POLICY_FILE_LOAD_THREAD = true;
	if (!POLICY_FILE_LOAD_THREAD) {
		int idx = 0;
		struct KeyValue kv;
		struct Key k;
		ifstream ifs(file_read_name, ios::binary | ios::in);
		if (ifs.is_open()) {
			while (ifs.good() && ifs.peek() != EOF) {
				ifs.read(&kv.key[0], sizeof(struct KeyValue));
				memcpy(&k.key[0], &kv.key[0], sizeof(struct KeyValue));
				k.idx = idx++;
				v.push_back(k);
			}
		}
		ifs.close();
	} else {
		int thread_cnt = NUM_THREADS;
		thread t[NUM_THREADS];
		int block_size = cnt / NUM_THREADS;

		for (int i = 0; i < NUM_THREADS; i++) {
			t[i] = thread(file_read_and_append, file_read_name, i * block_size, block_size);
		}
		for (int i = 0; i < NUM_THREADS; i++) {
			t[i].join();
		}
	}

	sort(v.begin(), v.end());

	bool POLICY_FILE_WRITE_THREAD = true;
	
	if (!POLICY_FILE_WRITE_THREAD) {
		struct KeyValue kv;
		ofstream ofs(file_write_name, ios::binary | ios::out);
		ifstream ifs(file_read_name, ios::binary | ios::in);
		for (auto iter = v.begin(); iter != v.end(); ++iter) {
			ifs.seekg((*iter).idx * 100);
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			ofs.write((char*)&kv, sizeof(kv));
		}
		ifs.close();
		ofs.close();
	} else {
		int thread_cnt = NUM_THREADS;
		thread t[NUM_THREADS];
		int block_size = cnt / NUM_THREADS;
		
		for (int i = 0; i < NUM_THREADS; i++) {
			// t[i] = thread(file_write_at, file_write_name, file_name, i * block_size, block_size);
			t[i] = thread(file_write_at, gen_tmp_file_name("tmp", i), file_read_name, i * block_size, block_size);
		}
		ofstream ofs(file_write_name, ios::binary | ios::out);
		for (int i = 0; i < NUM_THREADS; i++) {
			t[i].join();
			ifstream ifs(gen_tmp_file_name("tmp", i), ios::binary | ios::in);
			ofs << ifs.rdbuf();
			ifs.close();
			remove(gen_tmp_file_name("tmp", i).c_str());
		}
		ofs.close();
	}
}

