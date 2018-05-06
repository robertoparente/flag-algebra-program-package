#include "FlagGraph.h"
#include "SDPsolver.h"

int main(int argc, char **argv) {
	return FlagAlgebra::SDPsolverProgram<FlagAlgebra::Graph>(argc, argv);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
