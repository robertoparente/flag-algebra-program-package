#ifndef MONITORING_H_INCLUDED
#define MONITORING_H_INCLUDED

#include <iostream>

namespace Monitoring {
	class ProgressKeeper {
	public:
		virtual unsigned long operator()(unsigned long,
										 unsigned long,
										 const char* = "\n",
										 std::ostream& = std::cerr) const;
	};

	class ProgressBar : public ProgressKeeper {
	public:
		explicit ProgressBar(unsigned long = 9,
							 char = '[',
							 char = '=',
							 char = '|',
							 char = ' ',
							 char = ']');
		unsigned long barSize;
		char beginChar, fillChar, tipChar, emptyChar, endChar;
		virtual unsigned long operator()(unsigned long,
										 unsigned long,
										 const char* = "\n",
										 std::ostream& = std::cerr) const;
	};

	class ProgressBarFrac : public ProgressBar {
	public:
		explicit ProgressBarFrac(unsigned long = 9,
								 char = '[',
								 char = '=',
								 char = '|',
								 char = ' ',
								 char = ']');
		virtual unsigned long operator()(unsigned long,
										 unsigned long,
										 const char* = "\n",
										 std::ostream& = std::cerr) const;
	};
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
