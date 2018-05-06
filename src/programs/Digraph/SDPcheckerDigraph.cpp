#include "FlagDigraph.h"
#include "SDPchecker.h"

int main(int argc, char **argv) {
	return FlagAlgebra::SDPcheckerProgram<FlagAlgebra::Digraph>(argc, argv);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
