#include "Monitoring.h"

#include <cstring>
#include <sstream>

using namespace std;
using namespace Monitoring;

unsigned long ProgressKeeper::operator()(unsigned long n,
										 unsigned long t,
										 const char* s,
										 ostream& out) const {
	return 0;
}

ProgressBar::ProgressBar(unsigned long size,
						 char begin,
						 char fill,
						 char tip,
						 char empty,
						 char end)
	: ProgressKeeper(),
	  barSize(size),
	  beginChar(begin),
	  fillChar(fill),
	  tipChar(tip),
	  emptyChar(empty),
	  endChar(end) {}

unsigned long ProgressBar::operator()(unsigned long n,
									  unsigned long t,
									  const char* s,
									  ostream& out) const {
	out << s << beginChar;
	if (barSize) {
		const unsigned long fill(n*(barSize+1)/t);
		unsigned long i = 0;
		if (fill) {
			for (++i; i < fill; ++i)
				out << fillChar;
			if (i < barSize+1)
				out << tipChar;
		}
		for ( ; i < barSize; ++i)
			out << emptyChar;
	}
	out << endChar;
	return strlen(s) + 2 + barSize;
}

ProgressBarFrac::ProgressBarFrac(unsigned long size,
								 char begin,
								 char fill,
								 char tip,
								 char empty,
								 char end)
	: ProgressBar(size, begin, fill, tip, empty, end) {}

unsigned long ProgressBarFrac::operator()(unsigned long n,
										  unsigned long t,
										  const char* s,
										  ostream& out) const {
	stringstream ss;
	ss << " " << n << " / " << t;
	unsigned long ret(ProgressBar::operator()(n, t, s, out) + ss.str().size());
	out << ss.str();
	return ret;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
