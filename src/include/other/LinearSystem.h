#ifndef LINEARSYSTEM_H_INCLUDED
#define LINEARSYSTEM_H_INCLUDED

#include <vector>
#include <algorithm>
#include <utility>
#include <iterator>
#include "Monitoring.h"

namespace LinearAlgebra {
	class noSolution {};
	class badSystem {};

	template<class Iterator>
	std::vector<unsigned long>
	solveLinearSystem(const Iterator&,
					  const Iterator&,
					  const Monitoring::ProgressKeeper& = Monitoring::ProgressKeeper());

	template<class Iterator, class Container>
	void applySolution(const Iterator&,
					   const Iterator&,
					   Container&,
					   const Monitoring::ProgressKeeper& = Monitoring::ProgressKeeper());

	template<class Iterator>
	std::vector<unsigned long> solveSparseLinearSystem(const Iterator&,
													   const Iterator&,
													   unsigned long,
													   const Monitoring::ProgressKeeper&
													   = Monitoring::ProgressKeeper());

	template<class Iterator, class Container>
	void applySparseSolution(const Iterator&,
							 const Iterator&,
							 Container&,
							 const Monitoring::ProgressKeeper&
							 = Monitoring::ProgressKeeper());

	// This include is intentionally inside namespace LinearAlgebra
	// and has the namespace templates' definitions
#include "LinearSystemTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
