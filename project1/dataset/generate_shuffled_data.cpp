#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <bitset>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>

struct key_value {
	char key[10];
	char payload[90];
};

struct key {
	char data[10];
};

inline void swap_endian(unsigned int& x)
{
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}


using namespace std;

int main() {
	vector<key> v;

	// unsigned int cnt = 5;
	unsigned int cnt = 50000;
	// unsigned int cnt = 80000;
	// unsigned int cnt = 200000; // almost 20GB : 200,000 * 100 byte

	int byte_write = sizeof(struct key) - sizeof(int);
	for (unsigned int i = 0; i < cnt; i++) {
		struct key k;
		memset(&k, 0, sizeof(struct key));
		unsigned int tmp = i;
		swap_endian(tmp);
		memcpy(&k.data[byte_write], &tmp, sizeof(int));
		v.push_back(k);
	}

	random_shuffle(v.begin(), v.end());

	ofstream fout;
	string file_name = "test_" + to_string(cnt) +".data";
	fout.open(file_name, ios::binary | ios::out);

	for (auto iter = v.begin(); iter != v.end(); ++iter) {
		struct key_value kv;
		memset(&kv, 0, sizeof(struct key_value));
		memcpy(&kv.key[0], &(*iter), sizeof(key));
		memcpy(&kv.payload[0], &(*iter), sizeof(key));

		fout.write((char*)&kv, sizeof(kv));
	}

	fout.close();
}
