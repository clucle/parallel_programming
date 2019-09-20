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

#define MAX_NUM_THREADS 40
#define SIZE_KEY 10

using namespace std;

struct KeyValue {
	char key[10];
	char payload[90];
};


int cmp(const KeyValue& me, const KeyValue &other) {
	return memcmp(me.key, other.key, SIZE_KEY) <= 0;
}

const int LIMIT_BY_RAM_BLOCK_SIZE = 1900000;

mutex g_key_mutex;
sem_t sem;

string gen_tmp_name(int idx) {
	return "tmp" + to_string(idx) + ".data";
}

void gen_divided_sort_file(string file_read_name, int idx, int start, int end) {

	sem_wait(&sem);
	struct KeyValue kv;
	vector<KeyValue> v;
		
	ifstream ifs(file_read_name, ios::binary | ios::in);
	if (ifs.is_open()) {
		ifs.seekg(start);
		for (int i = start; i < end; i+=sizeof(struct KeyValue)) {
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			v.push_back(kv);
		}
	}
	ifs.close();

	sort(v.begin(), v.end(), cmp);
	
	ofstream ofs(gen_tmp_name(idx), ios::binary | ios::out);
	for (auto e: v) {
		ofs.write((char*)&kv, sizeof(struct KeyValue));
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
	
		cout << block_size << " bszie " << '\n';
	while (block_size * MAX_NUM_THREADS >= LIMIT_BY_RAM_BLOCK_SIZE) {

		cout << block_size << " bszie " << '\n';
		block_size /= 2;
		block_size -= block_size % 100;
	}
	cout << block_size << " bszie " << '\n';
	// generate divdided file
	sem_init(&sem, 0, MAX_NUM_THREADS);

	vector<thread> vt;
	for (int i = 0; ; i++) {
		int end = min((i + 1) * block_size, cnt_keyvalue);
		thread t = thread(gen_divided_sort_file, file_read_name, i, i * block_size, end);
		vt.push_back(std::move(t));
		cout << i * block_size << ' ' << end << '\n';
		if (end == cnt_keyvalue) break;
	}

	int cnt_file = vt.size();

	for (unsigned int i = 0; i < vt.size(); ++i) {
		vt.at(i).join();
	}


	sem_destroy(&sem);

	// merge sort by divided file

}

