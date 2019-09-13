#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>

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
	return memcmp(me.key, other.key, SIZE_KEY) < 0;
}

vector<Key> v;
mutex g_key_mutex;

void file_read_and_append(string file_name, int start, int cnt) {
	ifstream ifs(file_name, ios::binary | ios::in);
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

int main() {

	int cnt = 200000;

	string file_name = "./dataset/test_" + to_string(cnt) +".data";

	bool POLICY_FILE_LOAD_THREAD = true;
	if (!POLICY_FILE_LOAD_THREAD) {
		int idx = 0;
		struct KeyValue kv;
		struct Key k;
		ifstream ifs(file_name, ios::binary | ios::in);
		if (ifs.is_open()) {
			while (ifs.good() && ifs.peek() != EOF) {
				ifs.read(&kv.key[0], sizeof(struct KeyValue));
				memcpy(&k.key[0], &kv.key[0], sizeof(struct Key));
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
			t[i] = thread(file_read_and_append, file_name, i * block_size, block_size);
		}
		for (int i = 0; i < NUM_THREADS; i++) {
			t[i].join();
		}
	}

	cout << v.size() << '\n';
	sort(v.begin(), v.end());

	bool POLICY_FILE_WRITE_THREAD = false;
	string file_write_name = "./dataset/test_" + to_string(cnt) + "_result.data";
	if (!POLICY_FILE_WRITE_THREAD) {
		struct KeyValue kv;
		ofstream ofs(file_write_name, ios::binary | ios::out);
		ifstream ifs(file_name, ios::binary | ios::in);
		for (auto iter = v.begin(); iter != v.end(); ++iter) {
			ifs.seekg((*iter).idx * 100);
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			ofs.write((char*)&kv, sizeof(kv));
		}
		ifs.close();
		ofs.close();
	} else {

	}
}

