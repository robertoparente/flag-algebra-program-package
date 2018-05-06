#ifndef SDPROUNDER_H_INCLUDED
#define SDPROUNDER_H_INCLUDED

#include "Flag.h"
#include "getoptions.h"
#include "SDPauxiliaries.h"
#include "LinearSystem.h"
#include "Determinant.h"
#include "Monitoring.h"

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <map>

#define LAPACKE_AVAILABLE
#ifdef LAPACKE_AVAILABLE
#include <lapacke.h>
#endif

namespace FlagAlgebra {
	template<class UTO>
	int SDProunderProgram(int, char**);

	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "SDProunderTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
