#ifndef SDPAUXILIARIES_H_INCLUDED
#define SDPAUXILIARIES_H_INCLUDED

namespace FlagAlgebra {
	// This is an auxiliary class to be used with std::lower_bound
	class compFirstCoeffType {
	public:
		inline bool operator()(const coeff_type& a, const coeff_type& b) {
			return a.first < b.first;
		}
	};

	// This is an auxiliary class to be used with std::accumulate
	class sumSquare {
	public:
		inline unsigned long operator()(unsigned long x, unsigned long y) {
			return x + y*y;
		}
	};

	// This is an auxiliary class to be used in SDProunderProgram(int, char**)
	class MRef {
	public:
		unsigned long block, indi, indj;
		inline MRef() : block(), indi(), indj() {}
		inline MRef(unsigned long b, unsigned long i, unsigned long j)
			: block(b), indi(i), indj(j) {}
	};
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
