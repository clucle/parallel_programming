#include <iostream>
#include <cstring>
#include <fstream>
#include <string>

using namespace std;

struct KeyValue {
	char key[10];
	char payload[90];
};

int main() {
	int cnt = 200000;
	string file_name = "./dataset/test_" + to_string(cnt) +"_result.data";

	ifstream ifs(file_name, ios::binary | ios::in);
	if (ifs.is_open()) {
		struct KeyValue kv;
		while (ifs.good() && ifs.peek() != EOF) {
			ifs.read(&kv.key[0], sizeof(struct KeyValue));
			for (int i = 0; i < 10; i++) {
				cout << (int)kv.key[i] << ' ';
			}
			cout << '\n';
			for (int i = 0; i < 10; i++) {
				cout << (int)kv.payload[i] << ' ';
			}
			cout << '\n';
		}
	}
	ifs.close();
	return 0;
}

