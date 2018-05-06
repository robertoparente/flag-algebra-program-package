#include "Flag.h"

using namespace std;
using namespace FlagAlgebra;
	
/* This is the precision used to transform from BigNum::frac to string in functions
 * that print SDP problems, programs are free to alter its value.
 * Note that, since CSDP uses double for its problems, using a precision
 * higher than 
 * std::numeric_limits<double>::digits10
 * doesn't enhance the precision of the computation
 */
unsigned long FlagAlgebra::fracPrecision(std::numeric_limits<double>::digits10+1);

// This is the size of the progress bar shown by the SDPprograms
unsigned long FlagAlgebra::progressBarSize(19);

// FlagException class functions
/* default constructor
 */
FlagException::FlagException() throw() : exception(), s() {}

/* copy costructor
 */
FlagException::FlagException(const FlagException& other) throw() : exception(),
																   s(other.s) {}

/* copy operator
 */
FlagException& FlagException::operator=(const FlagException& other) throw() {
	s = other.s;
	return *this;
}

/* FlagException(string str) throw():
 * constructs FlagException such that what() returns str.c_str()
 */
FlagException::FlagException(string str) throw() : exception(), s(str) {}

/* destructor
 */
FlagException::~FlagException() throw() {}

/* what() const throw():
 * returns char* describing the exception
 */
const char* FlagException::what() const throw() {
	return s.c_str();
}
