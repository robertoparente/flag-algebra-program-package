#ifndef SDPSOLVER_H_INCLUDED
#define SDPSOLVER_H_INCLUDED

#include "Flag.h"
#include "SDPauxiliaries.h"
#include "SolverStrings.h"
#include "getoptions.h"
#include "Monitoring.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#define LAPACKE_AVAILABLE
#ifdef LAPACKE_AVAILABLE
#include <lapacke.h>
#endif

namespace FlagAlgebra {
	template<class UTO>
	int SDPsolverProgram(int, char**);

	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "SDPsolverTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
