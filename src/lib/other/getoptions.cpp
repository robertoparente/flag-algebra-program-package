#include "getoptions.h"

#include <cstring>

/* getoptions(const char *optstring,
 *			  int& index1,
 *			  int& index2,
 *			  char& option,
 *			  const char*& argument,
 *			  int argc,
 *			  char **argv):
 * Parses options given in (argc, argv) according to optstring starting from
 * argv[index][index2] (where the options are passed in bash fashion).
 * restrictions: optstring is a sequence of letters each possibly followed by ':'
 *               index >= argc or index2 < strlen(argv[index])
 * returns: true, iff an option was read (even if it was an invalid one)
 * note: the function sets option to be the option read;
 *       if the option read required an argument,
 *         it sets argument to be the argument read
 *       if the option read required an argument but none was given,
 *         it sets option = ':' and argument to point to the option read in optstring
 *       if the option read was invalid,
 *         it sets option = '\0' and argument to point to the option read in argv
 *       in any of these cases, the function sets index and index2 to be such that
 *         argv[index][index2] would be the next character to be parsed
 * note2: if index2 != 0, the function assumes it is in the middle of an option string
 * note3: this function is meant to have a similar behaviour of bash's getopts, but
 *        changing '?' option to '\0'
 */
bool Text::getoptions(const char *optstring,
					  int& index1,
					  int& index2,
					  char& option,
					  const char*& argument,
					  int argc,
					  char **argv) {
	if (index1 >= argc)
		return false;
	if (!index2) {
		for ( ; index1 < argc; ++index1)
			if (argv[index1][0])
				break;
		if (index1 >= argc) // Ran out of arguments to parse
			return false;
		if (argv[index1][0] != '-') // Argument is not an option
			return false;
		if (argv[index1][1] == '-') // Argument starts with an option terminator
			return false;
		// Option is valid
		index2 = 1;
	}
	for (unsigned long i = 0; optstring[i]; ++i) {
		if (optstring[i] == ':')
			continue;
		if (argv[index1][index2] == optstring[i]) {
			if (!argv[index1][++index2]) {
				index2 = 0;
				for (++index1; index1 < argc; ++index1)
					if (argv[index1][0])
						break;
			}
			if (optstring[i+1] == ':') { // Option requires argument
				if (index1 >= argc) { // No argument was given
					option = ':'; // Indicate missing argument
					argument = &optstring[i]; // Option found
					return true;
				}
				// An argument was given
				option = optstring[i];
				argument = argv[index1++] + index2;
				index2 = 0;
				return true;
			}
			// Option does not require argument
			option = optstring[i];
			return true;
		}
	}
	// Option given does not match any of accepted options
	option = '\0';
	argument = argv[index1] + index2;
	if (!argv[index1][++index2]) {
		index2 = 0;
		for (++index1; index1 < argc; ++index1)
			if (argv[index1][0])
				break;
	}
	return true;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
