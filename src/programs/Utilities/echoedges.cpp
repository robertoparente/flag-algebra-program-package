#include <iostream>
#include <string>
#include <utility>
#include <algorithm>
#include <sstream>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
	unsigned long st = 1, end = 1;

	if (argc > 1) {
		stringstream ss(argv[1]);
		ss >> st;
	}
	if (argc > 2) {
		stringstream ss(argv[2]);
		ss >> end;
	}

	vector<string> strs(++end);
	unsigned long ind = 0;

	for ( ; st; --st) {
		getline(cin, strs[0]);
		if (!cin.good())
			return 0;
		cout << strs[0] << endl;
	}
	cout << "[...]\n";
	do {
		(++ind) %= end;
		getline(cin, strs[ind]);
		strs[ind] += "\n";
	} while (cin.good());
	for (unsigned long i = (ind+1) % end; i != ind; (++i) %= end)
		cout << strs[i];
	return 0;
}
	
