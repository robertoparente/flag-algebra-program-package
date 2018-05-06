#include <iostream>

#include "getoptions.h"

using namespace std;

int main(int argc, char **argv) {
	int ind1 = 1, ind2 = 0;
	char opt;
	const char* arg = "";
	while (Text::getoptions("ab:", ind1, ind2, opt, arg, argc, argv)) {
		switch (opt) {
		case 'a':
			cerr << "Option a.\n";
			break;
		case 'b':
			cerr << "Option b with argument " << arg << "\n";
			break;
		case ':':
			cerr << "Missing argument for option: " << *arg << "\n";
			break;
		case '\0':
			cerr << "Invalid option: " << *arg << "\n";
			return 1;
		}
	}
	cerr << "Option parsing ended with\nind1 = " << ind1 << "\nind2 = " << ind2 << endl;
	return 0;
}
	
