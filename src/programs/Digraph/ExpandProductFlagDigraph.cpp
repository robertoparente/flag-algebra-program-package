#include "FlagDigraph.h"
#include "ExpandPrograms.h"

int main(int argc, char **argv) {
	FlagAlgebra::expandProductProgram<FlagAlgebra::Digraph>(argc, argv);
	return 0;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
