#ifndef FLAGGRAPH_H_INCLUDED
#define FLAGGRAPH_H_INCLUDED

#include <vector>

#include "Flag.h"

namespace FlagAlgebra {
	// class Graph represents graphs as UTOs
	class Graph {
	public:
		Graph();
		Graph(const Graph&);
		Graph& operator=(const Graph&);

		explicit Graph(const std::vector<vbool>&);
		Graph(unsigned long, const bool**);

		unsigned long size() const;
		Graph subobject(const vbool&) const;
		bool completeIsomorphism(const Graph&, morphism&) const;
		bool findIsomorphism(const Graph&, morphism&) const;
		bool operator==(const Graph&) const;
		bool operator!=(const Graph&) const;

		unsigned long degree(unsigned long) const;

		int readFromFile(FILE*);
		void writeOnFile(FILE*) const;

		static container<Graph> enumerate(unsigned long);
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
