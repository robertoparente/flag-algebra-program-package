#ifndef INJECTIVE_H_INCLUDED
#define INJECTIVE_H_INCLUDED

#include <algorithm>

namespace Injective {
	// Exception class
	class inexistent {};

	// Safe constructor signature auxiliary class
	class safe {};

	/* class injective<Cont> implements injective functions from [k] to [m] that
	 * can be iterated on (with next and prev)
	 * note: Cont must be a container of integer values (typically unsigned long)
	 */
	template<class Cont>
	class injective : public Cont {
    public:
        typedef Cont Container;

		injective(unsigned long, unsigned long);
		injective(unsigned long, unsigned long, safe);
		injective(const Cont&, unsigned long);
		injective(const Cont&, unsigned long, safe);

		unsigned long targetsize() const;

		bool next();
		bool prev();

	private:
		unsigned long t;
	};
	// This include is intentionally inside namespace Injective
	// and the file has the namespace templates' definitions
#include "injectiveTemplates.cpp"
}

#endif

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
