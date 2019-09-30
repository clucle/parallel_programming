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

using namespace std;

const unsigned int SIZE_KEY			= 10;
const unsigned int MAX_NUM_THREADS	= 40;
const unsigned int MAX_KV_SIZE		= 20000000;
const unsigned int MAX_RAM_SIZE		= 1800000000;

struct KeyValue {
	char key[10];
	char payload[90];
};

bool cmp(const KeyValue& me, const KeyValue &other) {
	return memcmp(me.key, other.key, SIZE_KEY) < 0;
}

struct KeyValue g_kv[MAX_KV_SIZE];

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
	unsigned int size_input_file = (unsigned int)ifs.tellg();
	ifs.close();
	int fd = open(argv[1], O_RDWR);

	printf("file size : %u\n", size_input_file);
	if (size_input_file <= MAX_RAM_SIZE) {
		// g_kv = (gKeyValue*)mmap(NULL, size_input_file, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);	
		// sort(g_kv->kv, g_kv->kv + size_input_file / sizeof(KeyValue), cmp);
		// munmap(g_kv, size_input_file);
		vector<thread> v;
		unsigned int size_per_thread = size_input_file / MAX_NUM_THREADS;
		for (int i = 0; i < MAX_NUM_THREADS; i++) {
			v.push_back(thread{[i, argv, size_per_thread]() {
				ifstream ifs(argv[1], ios::binary | ios::in);
				ifs.read(&g_kv->key[i * size_per_thread], size_per_thread);
				ifs.close();
			}});
		}
		for (int i = 0; i < MAX_NUM_THREADS; i++) {
			v[i].join();
		}
	
		ifstream ifs(argv[1], ios::binary | ios::in);
		ifs.read(&g_kv->key[0], size_input_file);
		ifs.close();
		
		print_duration();
		sort(g_kv, g_kv + size_input_file / sizeof(KeyValue), cmp);
		
		print_duration();
		ofstream ofs(argv[2], ios::binary | ios::out);
		ofs.write((char*)&g_kv->key[0], size_input_file);
		ofs.close();
	}
	print_duration();
	// string file_read_name = argv[1];
	// string file_write_name = argv[2];
	// remove(file_write_name.c_str());

	return 0;
}

