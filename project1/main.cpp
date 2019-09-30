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
const unsigned int MAX_KV_IN_SIZE	= 16000000;
const unsigned int MAX_KV_OUT_SIZE	=  2000000;
const unsigned int MAX_RAM_SIZE		= 1800000000;

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

bool operator<(const KeyValueNode& me, const KeyValueNode &other) {
	return memcmp(me.kv->key, other.kv->key, SIZE_KEY) > 0;
}



// struct KeyValue g_kv[MAX_KV_IN_SIZE];
struct KeyValue g_out_kv[MAX_KV_OUT_SIZE];

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
	size_t size_input_file = (size_t)ifs.tellg();
	ifs.close();
	// int fd = open(argv[1], O_RDWR);
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
		print_duration();
		/*
		FILE* in_file = fopen(argv[1], "rb");
		int r = fread(&g_kv->key[0], sizeof(KeyValue), size_input_file / sizeof(KeyValue), in_file);
		fclose(in_file);
		*/
		/*	
		ifstream ifs(argv[1], ios::binary | ios::in);
		ifs.read(&g_kv->key[0], size_input_file);
		ifs.close();
		*/

		print_duration();
		
		
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
		

		// ofstream ofs(argv[2], ios::binary | ios::out);
		// ofs.write((char*)&g_kv->key[0], size_input_file);
		// ofs.close();
		print_duration();
		/*
		FILE* out_file = fopen(argv[2], "wb+");
		fwrite((char*)&g_kv->key[0], sizeof(KeyValue), size_input_file / sizeof(KeyValue), out_file);
		fclose(out_file);*/
		
	}
	print_duration();
	// string file_read_name = argv[1];
	// string file_write_name = argv[2];
	// remove(file_write_name.c_str());

	return 0;
}

