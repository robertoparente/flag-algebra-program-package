#ifndef DETERMINANT_H_INCLUDED
#define DETERMINANT_H_INCLUDED

#include <vector>

namespace LinearAlgebra {
	typedef std::vector<bool> vbool;

	template<class Container>
	typename Container::value_type::value_type determinant(const Container&);

	template<class Container>
	typename Container::value_type::value_type principalMinorDeterminant(const Container&,
																		 const vbool&);

	template<class Container>
	bool isSymmetric(const Container&);

	template<class Container, class Signal>
	bool isPositiveSemidefinite(const Container&, Signal);

	template<class Container, class Signal>
	bool isPositiveDefinite(const Container&, Signal);

	template<class Container, class Signal>
	bool isNegativeSemidefinite(const Container&, Signal);

	template<class Container, class Signal>
	bool isNegativeDefinite(const Container&, Signal);

	template<class Container, class Signal>
	bool isIndefinite(const Container&, Signal);

	template<class number>
	int defaultSignal(const number&);

	template<class Container>
	bool isPositiveSemidefinite(const Container&);

	template<class Container>
	bool isPositiveDefinite(const Container&);

	template<class Container>
	bool isNegativeSemidefinite(const Container&);

	template<class Container>
	bool isNegativeDefinite(const Container&);

	template<class Container>
	bool isIndefinite(const Container&);

	// This include is intentionally inside namespace LinearAlgebra
	// and the file has the namespace templates' definitions
#include "DeterminantTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
