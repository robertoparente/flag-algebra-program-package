#include "FlagGraph.h"
#include "SDPchecker.h"

int main(int argc, char **argv) {
	return FlagAlgebra::SDPcheckerProgram<FlagAlgebra::Graph>(argc, argv);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
