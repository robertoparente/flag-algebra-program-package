#ifndef SDPCHECKER_H_INCLUDED
#define SDPCHECKER_H_INCLUDED

#include "Flag.h"
#include "SDPauxiliaries.h"
#include "Determinant.h"
#include "getoptions.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>

namespace FlagAlgebra {
	template<class UTO>
	int SDPcheckerProgram(int, char**);

	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "SDPcheckerTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
