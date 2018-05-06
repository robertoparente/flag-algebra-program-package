#ifndef TESTFLAG_H_INCLUDED
#define TESTFLAG_H_INCLUDED

#include <iostream>
#include <cstdio>

#include "Flag.h"

namespace FlagAlgebra {
	template<class UTO>
	void testFlag();

	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "TestFlagTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
