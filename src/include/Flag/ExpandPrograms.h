#ifndef EXPANDPROGRAMS_H_INCLUDED
#define EXPANDPROGRAMS_H_INCLUDED

#include "Flag.h"
#include <cstdio>
#include <iostream>
#include <cstring>
#include "getoptions.h"

namespace FlagAlgebra {
	template<class UTO>
	int enumerateProgram(int, char**);

	template<class UTO>
	int expandProgram(int, char**);

	template<class UTO>
	int expandProductProgram(int, char**);

	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "ExpandProgramsTemplates.cpp"
}


/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
