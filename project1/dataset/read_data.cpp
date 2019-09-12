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


using namespace std;

void test(vector<key_value> &v) {
	for (auto iter = v.begin(); iter != v.end(); ++iter) {
		struct key_value kv;
		memcpy(&kv.key[0], &(*iter), sizeof(key_value));
		for (int j = 0; j < 10; j++) {
			cout << (int)kv.key[j] << ' ';
		}
		cout << '\n';

		for (int j = 0; j < 10; j++) {
			cout << (int)kv.payload[j] << ' ';
		}
		cout << '\n';
		cout << '\n';
	}
}

int main() {
	vector<key_value> v;

	int cnt = 50000;
	string file_name = "test_" + to_string(cnt) +".data";
	ifstream ifs(file_name, ios::binary | ios::in);
	struct key_value kv;


	if (ifs.is_open()) {
		while (ifs.good() && ifs.peek() != EOF) {
			ifs.read(&kv.key[0], sizeof(struct key_value));
			v.push_back(kv);
		}
	}
	ifs.close();

	// test(v);
	for (auto iter = v.begin(); iter != v.end(); ++iter) {
		struct key_value kv;
		memcpy(&kv.key[0], &(*iter), sizeof(key_value));
	}
}

