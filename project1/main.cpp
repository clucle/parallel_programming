#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>

using namespace std;

const unsigned int SIZE_KEY			= 10;
const unsigned int MAX_NUM_THREADS	= 40;
const unsigned int MAX_KV_IN_SIZE	= 12000000;
const unsigned int MAX_KV_OUT_SIZE	=  5000000;
const unsigned int MAX_RAM_SIZE		= 1000000000;

struct KeyValue {
	char key[10];
	char payload[90];
};

bool cmp(const KeyValue& me, const KeyValue &other) {
	return memcmp(me.key, other.key, SIZE_KEY) < 0;
}

struct KeyValueNode {
	struct KeyValue *kv;
	int idx = 0;
	int end = 0;
};


struct KeyValueNodeFile {
	struct KeyValue *kv;
	unsigned int idx = 0;
	unsigned int offset = 0;
	unsigned int end = 0;
	unsigned int idx_file = 0;
};

bool operator<(const KeyValueNode& me, const KeyValueNode &other) {
	return memcmp(me.kv->key, other.kv->key, SIZE_KEY) > 0;
}


bool operator<(const KeyValueNodeFile& me, const KeyValueNodeFile &other) {
	return memcmp(me.kv->key, other.kv->key, SIZE_KEY) > 0;
}

chrono::system_clock::time_point start;

void print_duration() {
	chrono::duration<double> sec = chrono::system_clock::now() - start;
	printf("%lf\n", sec.count());
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		printf("usage: ./run InputFile OutputFile\n");
		return 0;
	}

	start = chrono::system_clock::now();

	ifstream ifs(argv[1], ios::binary | ios::ate);
	ifs.seekg(0, ios::end);
	struct KeyValue* g_kv = (KeyValue*)malloc(sizeof(KeyValue) * MAX_KV_IN_SIZE);
	struct KeyValue* g_out_kv = (KeyValue*)malloc(sizeof(KeyValue) * MAX_KV_OUT_SIZE);
	size_t size_input_file = (size_t)ifs.tellg();
	ifs.close();

	if (size_input_file <= MAX_RAM_SIZE) {
		
		vector<thread> v;
		unsigned int size_per_thread = size_input_file / MAX_NUM_THREADS;
		unsigned int block_per_thread = size_per_thread / sizeof(KeyValue);

		for (int i = 0; i < MAX_NUM_THREADS; i++) {
			v.push_back(thread{[i, argv, size_per_thread, block_per_thread, g_kv]() {
				ifstream ifs(argv[1], ios::binary | ios::in);
				ifs.seekg(i * size_per_thread);
				ifs.read(&g_kv->key[i * size_per_thread], size_per_thread);
				ifs.close();
				sort(g_kv + i * block_per_thread, g_kv + (i + 1) * block_per_thread, cmp);
			}});
		}
		for (int i = 0; i < MAX_NUM_THREADS; i++) {
			v[i].join();
		}
		
		priority_queue<KeyValueNode> pq;
		for (int i = 0; i < MAX_NUM_THREADS; i++) {
			struct KeyValueNode kvn;
			kvn.idx = i * block_per_thread;
			kvn.end = kvn.idx + block_per_thread;
			kvn.kv = g_kv + kvn.idx;
			pq.push(std::move(kvn));
		}
		int cur_out = 0;
	
		ofstream ofs(argv[2], ios::binary | ios::out);
		while (!pq.empty()) {
			auto kvn = pq.top();
			pq.pop();
			memcpy(&g_out_kv[cur_out++], &(*(kvn.kv)), sizeof(KeyValue));
			if (cur_out == MAX_KV_OUT_SIZE) {
				ofs.write((char*)&g_out_kv->key[0], MAX_KV_OUT_SIZE * sizeof(KeyValue));
				cur_out = 0;
			}
			kvn.idx++;
			if (kvn.idx == kvn.end) {
				continue;
			}
			kvn.kv = g_kv + kvn.idx;
			pq.push(std::move(kvn));
		}
		if (cur_out > 0) {
			ofs.write((char*)&g_out_kv->key[0], cur_out * sizeof(KeyValue));
		}
		
		ofs.close();
	} else {
		unsigned int size_tmp_file = size_input_file / MAX_RAM_SIZE;
		unsigned int size_per_thread = size_input_file / MAX_NUM_THREADS / size_tmp_file;
		unsigned int block_per_thread = size_per_thread / sizeof(KeyValue);

		for (unsigned int i = 0; i < size_tmp_file; i++) {
			string name_tmp_file = "t_" + to_string(i) + ".data";
			unsigned int offset = i * MAX_RAM_SIZE;

			vector<thread> v;

			for (int i = 0; i < MAX_NUM_THREADS; i++) {
				v.push_back(thread{[i, argv, size_per_thread, block_per_thread, g_kv, offset]() {
					ifstream ifs(argv[1], ios::binary | ios::in);
					ifs.seekg(offset + i * size_per_thread);
					ifs.read(&g_kv->key[i * size_per_thread], size_per_thread);
					ifs.close();
					sort(g_kv + i * block_per_thread, g_kv + (i + 1) * block_per_thread, cmp);
				}});
			}
			for (int i = 0; i < MAX_NUM_THREADS; i++) {
				v[i].join();
			}
		
			priority_queue<KeyValueNode> pq;
			for (int i = 0; i < MAX_NUM_THREADS; i++) {
				struct KeyValueNode kvn;
				kvn.idx = i * block_per_thread;
				kvn.end = kvn.idx + block_per_thread;
				kvn.kv = g_kv + kvn.idx;
				pq.push(std::move(kvn));
			}
			int cur_out = 0;
	
			ofstream ofs(name_tmp_file, ios::binary | ios::out);
			while (!pq.empty()) {
				auto kvn = pq.top();
				pq.pop();
				memcpy(&g_out_kv[cur_out++], &(*(kvn.kv)), sizeof(KeyValue));
				if (cur_out == MAX_KV_OUT_SIZE) {
					ofs.write((char*)&g_out_kv->key[0], MAX_KV_OUT_SIZE * sizeof(KeyValue));
					cur_out = 0;
				}
				kvn.idx++;
				if (kvn.idx == kvn.end) {
					continue;
				}
				kvn.kv = g_kv + kvn.idx;
				pq.push(std::move(kvn));
			}
			if (cur_out > 0) {
				ofs.write((char*)&g_out_kv->key[0], cur_out * sizeof(KeyValue));
			}
		
			ofs.close();
		}

		unsigned int size_section = MAX_RAM_SIZE / size_tmp_file;
		unsigned int block_size_section = size_section / sizeof(KeyValue);
		printf("section : %u\n", block_size_section);
		print_duration();
		// merge tmp file
		priority_queue<KeyValueNodeFile> pq;
		vector<thread> v;
		for (unsigned int i = 0; i < size_tmp_file; i++) {
			v.push_back(thread{[i, size_section, g_kv]() {
				string name_tmp_file = "t_" + to_string(i) + ".data";
				ifstream ifs(name_tmp_file, ios::binary | ios::in);
				ifs.read(&g_kv->key[i * size_section], size_section);
				ifs.close();
			}});
		}
		for (unsigned int i = 0; i < size_tmp_file; i++) {
			v[i].join();
			struct KeyValueNodeFile kvnf;
			kvnf.idx = 0;
			kvnf.offset = block_size_section;
			kvnf.end = MAX_RAM_SIZE / sizeof(KeyValue);
			kvnf.idx_file = i;
			kvnf.kv = g_kv + i * block_size_section;
			pq.push(std::move(kvnf));
		}

		print_duration();
		int cur_out = 0;
		int cur_write_idx = 0;
		unsigned int HALF_MAX_KV_OUT_SIZE = MAX_KV_OUT_SIZE / 2;
		thread* t_write = new thread{[ = ](){}};

		ofstream ofs(argv[2], ios::binary | ios::out);
		while (!pq.empty()) {
			auto kvnf = pq.top();
			pq.pop();
			memcpy(&g_out_kv[cur_out++], &(*(kvnf.kv)), sizeof(KeyValue));
			if (cur_out == HALF_MAX_KV_OUT_SIZE) {
				t_write->join();
				t_write = new thread{[&ofs, HALF_MAX_KV_OUT_SIZE, g_out_kv]() {
					ofs.write((char*)&g_out_kv->key[0], HALF_MAX_KV_OUT_SIZE * sizeof(KeyValue));
				}};
				cur_write_idx = HALF_MAX_KV_OUT_SIZE;
			}
			if (cur_out == MAX_KV_OUT_SIZE) {
				t_write->join();
				t_write = new thread{[&ofs, HALF_MAX_KV_OUT_SIZE, g_out_kv]() {
					ofs.write((char*)&g_out_kv->key[HALF_MAX_KV_OUT_SIZE * sizeof(KeyValue)], HALF_MAX_KV_OUT_SIZE * sizeof(KeyValue));
				}};
				cur_write_idx = 0;
				cur_out = 0;
			}
			kvnf.idx++;
			if (kvnf.idx == kvnf.end) {
				string name_tmp_file = "t_" + to_string(kvnf.idx_file) + ".data";
				remove(name_tmp_file.c_str());
				continue;
			}
			if (kvnf.idx == kvnf.offset) {
				unsigned int read_size = min(block_size_section, kvnf.end - kvnf.idx) * sizeof(KeyValue);
				kvnf.offset = kvnf.idx + min(block_size_section, kvnf.end - kvnf.idx);
				string name_tmp_file = "t_" + to_string(kvnf.idx_file) + ".data";
				ifstream ifs(name_tmp_file, ios::binary | ios::in);
				ifs.seekg(kvnf.idx * sizeof(KeyValue));
				ifs.read(&g_kv->key[kvnf.idx_file * block_size_section * sizeof(KeyValue)], read_size);
				ifs.close();
			}
			kvnf.kv = g_kv + (kvnf.idx_file * block_size_section + kvnf.idx % block_size_section);
			pq.push(std::move(kvnf));
		}
		t_write->join();
		if (cur_out > cur_write_idx) {
			ofs.write((char*)&g_out_kv->key[cur_write_idx * sizeof(KeyValue)], (cur_out - cur_write_idx) * sizeof(KeyValue));
		}
		ofs.close();
	}
	print_duration();
	return 0;
}

