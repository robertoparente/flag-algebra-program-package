#ifndef FLAGDIGRAPH_H_INCLUDED
#define FLAGDIGRAPH_H_INCLUDED

#include <vector>

#include "Flag.h"

namespace FlagAlgebra {
	// class Digraph represents digraphs as UTOs
	class Digraph {
	public:
		Digraph();
		Digraph(const Digraph&);
		Digraph& operator=(const Digraph&);

		explicit Digraph(const std::vector<vbool>&);
		Digraph(unsigned long, const bool**);

		unsigned long size() const;
		Digraph subobject(const vbool&) const;
		bool completeIsomorphism(const Digraph&, morphism&) const;
		bool findIsomorphism(const Digraph&, morphism&) const;
		bool operator==(const Digraph&) const;
		bool operator!=(const Digraph&) const;

		unsigned long outDegree(unsigned long) const;
		unsigned long inDegree(unsigned long) const;

		int readFromFile(FILE*);
		void writeOnFile(FILE*) const;

		static container<Digraph> enumerate(unsigned long);
	protected:
		unsigned long s;
		vbool m;
	};
}


/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
