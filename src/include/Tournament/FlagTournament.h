#ifndef FLAGTOURNAMENT_H_INCLUDED
#define FLAGTOURNAMENT_H_INCLUDED

#include <vector>

#include "Flag.h"
#include "FlagDigraph.h"

namespace FlagAlgebra {
	// class Tournament represents tournament as UTOs
	class Tournament : protected Digraph {
	public:
		Tournament();
		Tournament(const Tournament&);
		Tournament& operator=(const Tournament&);

		explicit Tournament(const std::vector<vbool>&);
		Tournament(unsigned long, const bool**);

		unsigned long size() const;
		Tournament subobject(const vbool&) const;
		bool completeIsomorphism(const Tournament&, morphism&) const;
		bool findIsomorphism(const Tournament&, morphism&) const;
		bool operator==(const Tournament&) const;
		bool operator!=(const Tournament&) const;

		int readFromFile(FILE*);
		void writeOnFile(FILE*) const;

		static container<Tournament> enumerate(unsigned long);
	protected:
		explicit Tournament(const Digraph&);
	};
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
