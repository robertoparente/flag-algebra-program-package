#include "FlagGraph.h"
#include "SDPgenerate.h"

int main(int argc, char **argv) {
	return FlagAlgebra::SDPgenerateProgram<FlagAlgebra::Graph>(argc, argv);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
