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
	string file_s = "./dataset/single.data";
	string file_m = "./dataset/multi.data";

	ifstream ifs_s(file_s, ios::binary | ios::in);
	ifstream ifs_m(file_m, ios::binary | ios::in);

	struct KeyValue kv_s;
	struct KeyValue kv_m;
	int idx = 0;
	while (ifs_s.good() && ifs_s.peek() != EOF) {
		ifs_s.read(&kv_s.key[0], sizeof(struct KeyValue));
		ifs_m.read(&kv_m.key[0], sizeof(struct KeyValue));
		bool isWrong_key = false;
		bool isWrong_payload = false;
		for (int i = 0; i < 10; i++) {
			if ((char)kv_s.key[i] != (char)kv_m.key[i]) isWrong_key = true;
		}
		if (idx < 10 && (isWrong_key || isWrong_payload)) {
			cout << idx << " line wrong\n";
			for (int i = 0; i < 10; i++) {
				cout << (int)kv_s.key[i] << ' ';
			}
			cout << '\n';

			for (int i = 0; i < 10; i++) {
				cout << (int)kv_m.key[i] << ' ';
			}
			cout << '\n';
		}
		idx++;
		if (idx % 50000 == 0) idx = 0;
	}
	ifs_s.close();
	ifs_m.close();
	return 0;
}

