#include "bignum.h"

#include <cctype>
#include <sstream>
#include <algorithm>
#include <deque>

using namespace BigNum;
using namespace std;

// Auxiliary char type functions
static int isoctal(int c) {
	return '0' <= c && c < '8';
}

// These are useful bignum constants
const bignum BigNum::bn0(0),
				BigNum::bn1(1),
				BigNum::bn2(2),
				BigNum::bn8(8),
				BigNum::bn10(10),
				BigNum::bn16(16);

const size_t bignum::bitsElem(8);
const bignum::elem_t bignum::maxElem(255);

bignum::bignum() : v(), sgn(false) {}

bignum::bignum(long n) : v(sizeof(long)), sgn() {
	if (n < 0) {
		n = -n;
		sgn = true;
	}
	for (unsigned long i = 0; n; ++i) {
		long m(n >> bitsElem);
		v[i] = static_cast<elem_t>(n - (m << bitsElem));
		n = m;
	}
	adjust();
}

bignum::bignum(const bignum& other) : v(other.v), sgn(other.sgn) {}

bignum& bignum::operator=(const bignum& other) {
	v = other.v;
	sgn = other.sgn;
	return *this;
}

bignum bignum::operator+(const bignum& other) const {
	bignum ret(sgn == other.sgn?
			   unsignedAdd(other) :
			   unsignedSubtract(other));
	if (sgn)
		ret.invert();
	return ret;
}

bignum bignum::operator-(const bignum& other) const {
	bignum ret(sgn != other.sgn?
			   unsignedAdd(other) :
			   unsignedSubtract(other));
	if (sgn)
		ret.invert();
	return ret;
}

bignum bignum::operator*(const bignum& other) const {
	bignum ret;

	ret.v.resize(v.size() + other.v.size());
	for (unsigned long shift = 0; shift < other.v.size(); ++shift) {
		const wide_t otherWide(static_cast<wide_t>(other.v[shift]));
		wide_t carry = 0;
		for (unsigned long i = 0; i < v.size(); ++i) {
			wide_t result(static_cast<wide_t>(v[i]) * otherWide
						  + static_cast<wide_t>(ret.v[shift+i]) + carry);
			carry = result >> bitsElem;
			ret.v[shift + i] = static_cast<elem_t>(result - (carry << bitsElem));
		}
		ret.v[shift + v.size()] = static_cast<elem_t>(carry);
	}
	ret.sgn = sgn xor other.sgn;
	ret.adjust();
	ret.shrink();
	return ret;
}

bignum bignum::operator/(const bignum& other) const {
	return divide(*this, other).first;
}

bignum bignum::operator%(const bignum& other) const {
	return divide(*this, other).second;
}

bignum bignum::operator<<(long d) const {
	bignum ret(*this);
	return ret <<= d;
}

bignum bignum::operator>>(long d) const {
	bignum ret(*this);
	return ret >>= d;
}

bignum& bignum::operator+=(const bignum& other) {
	return *this = operator+(other);
}
		
bignum& bignum::operator-=(const bignum& other) {
	return *this = operator-(other);
}
		
bignum& bignum::operator*=(const bignum& other) {
	return *this = operator*(other);
}
		
bignum& bignum::operator/=(const bignum& other) {
	return *this = operator/(other);
}

bignum& bignum::operator%=(const bignum& other) {
	return *this = operator%(other);
}

bignum& bignum::operator<<=(long d) {
	if (d < 0)
		return operator>>=(-d);
	if (d) {
		const unsigned long s(d / bitsElem);
		v.resize(v.size() + s + 1);
		if (s) {
			for (unsigned long i = v.size(); i > s; ) {
				--i;
				v[i] = v[i - s];
			}
			for (unsigned long i = 0; i < s; ++i)
				v[i] = 0;
		}
		if (d %= bitsElem) {
			wide_t carry = 0;
			for (unsigned long i = 0; i < v.size(); ++i) {
				wide_t result((static_cast<wide_t>(v[i]) << d) + carry);
				carry = result >> bitsElem;
				v[i] = static_cast<elem_t>(result - (carry << bitsElem));
			}
			// carry always ends with 0
		}
		adjust();
	}
	return *this;
}

bignum& bignum::operator>>=(long d) {
	if (d < 0)
		return operator<<=(-d);
	if (d) {
		const unsigned long s(d / bitsElem);
		if (s >= v.size())
			return *this = bn0;
		if (s) {
			const unsigned long limit = v.size() - s;
			for (unsigned long i = 0; i < limit; ++i)
				v[i] = v[i + s];
			v.resize(limit);
		}
		if (d %= bitsElem) {
			wide_t revCarry = 0;
			for (unsigned long i = v.size(); i > 0; ) {
				--i;
				wide_t withCarry(static_cast<wide_t>(v[i]) + revCarry);
				v[i] = withCarry >> d;
				revCarry = (withCarry - (v[i] << d)) << bitsElem;
			}
			// revCarry final value holds bits that are discarded
		}
		adjust();
		shrink();
	}
	return *this;
}

const bignum& bignum::operator+() const {
	return *this;
}

bignum bignum::operator-() const {
	bignum ret(*this);
	return ret.invert();
}

bool bignum::operator==(const bignum& other) const {
	adjust();
	other.adjust();
	if (sgn != other.sgn)
		return false;
	if (v.size() != other.v.size())
		return false;
	for (unsigned long i = 0; i < v.size(); ++i)
		if (v[i] != other.v[i])
			return false;
	return true;
}

bool bignum::operator!=(const bignum& other) const {
	return !operator==(other);
}

bool bignum::operator<(const bignum& other) const {
	if (sgn != other.sgn)
		return sgn;
	if (v.size() < other.v.size())
		return !sgn;
	if (v.size() > other.v.size())
		return sgn;
	for (unsigned long i = v.size(); i > 0; ) {
		--i;
		if (v[i] < other.v[i])
			return !sgn;
		if (v[i] > other.v[i])
			return sgn;
	}
	return false;
}

bool bignum::operator<=(const bignum& other) const {
	return !(other < *this);
}

bool bignum::operator>(const bignum& other) const {
	return other < *this;
}

bool bignum::operator>=(const bignum& other) const {
	return !operator<(other);
}

bignum& bignum::operator++() {
	if (sgn) {
		unsigned long i = 0;
		while (!v[i]) {
			v[i++] = maxElem;
		}
		--v[i];
		adjust();
		return *this;
	}
	else {
		unsigned long i = 0;
		while (i < v.size()) {
			wide_t result(v[i]);
			++result;
			if (result >> bitsElem) {
				v[i] = 0;
			}
			else {
				v[i] = static_cast<elem_t>(result);
				return *this;
			}
			++i;
		}
		v.push_back(1);
		return *this;
	}
}

bignum& bignum::operator--() {
	invert();
	operator++();
	return invert();
}

bool bignum::zero() const {
	return !v.size();
}

bool bignum::positive() const {
	return !sgn && v.size();
}

bool bignum::negative() const {
	return sgn && v.size();
}

bignum& bignum::invert() {
	sgn = !zero() && !sgn;
	return *this;
}

bignum& bignum::removeSignal() {
	sgn = false;
	return *this;
}

// Auxiliary variable to count number of characters read
static unsigned long charread;
// Auxiliary variable to store signal read
static bool sgnread;
/* readFromStream(std::istream& in, base b):
 * reads a representation of number in base b from in and makes *this be that number
 * returns: in (after the operation)
 * note: puts the number of characters read in charread and signal read in sgnread
 */
std::istream& bignum::readFromStream(std::istream& in, base b) {
	charread = 0;
	sgnread = false;
	const bignum* basebn = 0;

	int (*testfun)(int) = 0;
	switch (b) {
	case octal:
		basebn = &bn8;
		testfun = isoctal;
		break;
	case decimal:
		basebn = &bn10;
		testfun = isdigit;
		break;
	case hexadecimal:
		basebn = &bn16;
		testfun = isxdigit;
		break;
	}

#define basebn (*basebn)

#define readskipws()							\
	do {										\
		c = in.get();							\
		++charread;								\
	} while (in.good() && isspace(c))

#define teststream()							\
	if (!in.good())								\
		return in

	*this = bn0;
	bool s = false;
	int c;
	readskipws();
	teststream();
	
	switch (c) {
	case '-':
		sgnread = s = true;
		// fall through
	case '+':
		readskipws();
		teststream();
		break;
	}
	if (!(*testfun)(c)) {
		in.clear(std::istream::failbit);
		return in;
	}
	do {
		*this *= basebn;
		*this += bignum(c - ('0' <= c && c <= '9'? '0' :
							 ('a' <= c && c <= 'f'? 'a' : 'A') - 10));
		c = in.get();
		++charread;
		if (c == EOF) {
			in.clear(std::istream::goodbit);
			break;
		}
		teststream();
	} while ((*testfun)(c));
	in.unget();
    --charread;
	if (!zero())
		sgn = s;
	return in;
#undef basebn
#undef readskipws
#undef teststream
}

/* readFromFILE(FILE *f, base b):
 * reads a representation of number in base b from f and makes *this be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: puts the number of characters read in charread and the signal read in sgnread
 */
int bignum::readFromFILE(FILE *f, base b) {
	charread = 0;
	sgnread = false;
	const bignum* basebn = 0;

	int (*testfun)(int) = 0;
	switch (b) {
	case octal:
		basebn = &bn8;
		testfun = isoctal;
		break;
	case decimal:
		basebn = &bn10;
		testfun = isdigit;
		break;
	case hexadecimal:
		basebn = &bn16;
		testfun = isxdigit;
		break;
	}

#define basebn (*basebn)

#define readskipws()							\
	do {										\
		c = fgetc(f);							\
		++charread;								\
	} while (c != EOF && isspace(c))

#define teststream()							\
	if (c == EOF)								\
		return EOF;

	*this = bn0;
	bool s = false;
	int c;
	readskipws();
	teststream();
	
	switch (c) {
	case '-':
		sgnread = s = true;
		// fall through
	case '+':
		readskipws();
		teststream();
		break;
	}
	if (!(*testfun)(c)) {
		return 0;
	}
	do {
		*this *= basebn;
		*this += bignum(c - ('0' <= c && c <= '9'? '0' :
							 ('a' <= c && c <= 'f'? 'a' : 'A') - 10));
		c = fgetc(f);
		++charread;
		if (c == EOF) {
			if (feof(f))
				break; // This means that an EOF was read
			return EOF; // This means that an error occurred
		}
	} while ((*testfun)(c));
	ungetc(c, f);
    --charread;
	if (!zero())
		sgn = s;
	return 1;
#undef basebn
#undef readskipws
#undef teststream
}

/* putOnStreamOct(std::ostream& out) const:
 * puts a string representing *this in base 8 on stream out
 * returns: out (after operation)
 */
std::ostream& bignum::putOnStreamOct(std::ostream& out) const {
    if (zero())
		return out << '0';

	if (negative())
		out << '-';

    std::string str;
    divRes dr(*this, bn0);
    dr.first.removeSignal();
    while (!dr.first.zero()) {
        dr = divide(dr.first, bn8);
		if (dr.second.zero())
			str.push_back('0');
		else
			str.push_back(dr.second.v.front() + '0');
    }

    for (unsigned long i = str.size(); i > 0; --i)
        out << str[i-1];
    return out;
}

/* putOnStreamDec(std::ostream& out) const:
 * puts a string representing *this in base 10 on stream out
 * returns: out (after operation)
 */
std::ostream& bignum::putOnStreamDec(std::ostream& out) const {
    if (zero())
		return out << '0';

	if (negative())
		out << '-';

    std::string str;
    divRes dr(*this, bn0);
    dr.first.removeSignal();
    while (!dr.first.zero()) {
        dr = divide(dr.first, bn10);
		if (dr.second.zero())
			str.push_back('0');
		else
			str.push_back(dr.second.v.front() + '0');
    }

    for (unsigned long i = str.size(); i > 0; --i)
        out << str[i-1];
    return out;
}

/* putOnStreamHex(std::ostream& out) const:
 * puts a string representing *this in base 16 on stream out
 * returns: out (after operation)
 */
std::ostream& bignum::putOnStreamHex(std::ostream& out) const {
    if (zero())
		return out << '0';
    if (sgn)
		out << '-';
	std::ostream::fmtflags oldFlags(out.flags());
	std::streamsize oldWidth(out.width(0));
	char oldFill(out.fill('0'));
	
	out.flags(std::ostream::hex | std::ostream::right);
	
	unsigned long i = v.size();
	if (i > 0) {
		out << static_cast<wide_t>(v[--i]);
		out.width(2);
	}
	while (i > 0)
		out << static_cast<wide_t>(v[--i]);
	out.flags(oldFlags);
	out.width(oldWidth);
	out.fill(oldFill);
    return out;
}

/* putOnStream(std::ostream& out, base b) const:
 * puts a string representing *this in base b on stream out
 * returns: out (after operation)
 */
std::ostream& bignum::putOnStream(std::ostream& out, base b) const {
    switch (b) {
	case octal:
		return putOnStreamOct(out);
	case decimal:
		return putOnStreamDec(out);
	case hexadecimal:
		return putOnStreamHex(out);
    }
	return out; //doesn't happen
}

/* toStringOct() const:
 * returns: a string representing *this in base 8
 */
std::string bignum::toStringOct() const {
	std::stringstream s;
	putOnStreamOct(s);
    return s.str();
}

/* toStringDec() const:
 * returns: a string representing *this in base 10
 */
std::string bignum::toStringDec() const {
	std::stringstream s;
	putOnStreamDec(s);
    return s.str();
}

/* toStringHex() const:
 * returns: a string representing *this in base 16
 */
std::string bignum::toStringHex() const {
	std::stringstream s;
	putOnStreamHex(s);
    return s.str();
}

/* toString(base b) const:
 * returns: a string representing *this in base b
 */
std::string bignum::toString(base b) const {
	std::stringstream s;
	putOnStream(s, b);
	return s.str();
}

void bignum::swap(bignum& other) {
	v.swap(other.v);
	std::swap(sgn, other.sgn);
}

void bignum::adjust() const {
	if (v.size()) {
		unsigned long i = v.size();
		while (i > 0 && !v[--i]) {
		}
		if (v[i])
			++i;
		else
			sgn = false;
		v.resize(i);
	}
}
		
/* shrink() const:
 * this function attempts to free any unused memory allocated by *this
 */
void bignum::shrink() const {
	std::vector<elem_t> w(v.size()); // w is as fit as it can be
	for (unsigned long i = 0; i < w.size(); ++i)
		w[i] = v[i];
	w.swap(v);
}

bignum bignum::unsignedAdd(const bignum& other, bool check) const {
	if (check && v.size() < other.v.size())
		return other.unsignedAdd(*this, false);

	bignum ret;
	ret.v.resize(v.size()+1);
	bool carry = 0;
	unsigned long i = 0;
	while (i < other.v.size()) {
		wide_t result(static_cast<wide_t>(v[i])
					  + static_cast<wide_t>(other.v[i])
					  + carry);
		carry = result >> bitsElem;
		ret.v[i++] = result - (static_cast<wide_t>(carry) << bitsElem);
	}
	while (i < v.size()) {
		wide_t result(static_cast<wide_t>(v[i]) + carry);
		carry = result >> bitsElem;
		ret.v[i++] = result - (static_cast<wide_t>(carry) << bitsElem);
	}
	if (carry)
		ret.v.back() = 1;
	ret.adjust();
	return ret;
}

bignum bignum::unsignedSubtract(const bignum& other, bool check) const {
	if (check) {
		bool oldSgn(sgn), oldOtherSgn(other.sgn);
		sgn = false;
		other.sgn = false;
		check = operator<(other);
		sgn = oldSgn;
		other.sgn = oldOtherSgn;
		if (check)
			return other.unsignedSubtract(*this, false).invert();
	}

	bignum ret;
	ret.v.resize(v.size());
	const wide_t pull(static_cast<wide_t>(maxElem) + 1);
	bool revCarry = false;
	unsigned long i = 0;
	while (i < other.v.size()) {
		wide_t result(pull
					  + static_cast<wide_t>(v[i])
					  - static_cast<wide_t>(other.v[i])
					  - revCarry);
		revCarry = !(result >> bitsElem);
		ret.v[i++] = result - (static_cast<wide_t>(revCarry) << bitsElem);
	}
	while (i < v.size()) {
		wide_t result(pull
					  + static_cast<wide_t>(v[i])
					  - revCarry);
		revCarry = !(result >> bitsElem);
		ret.v[i++] = result - (static_cast<wide_t>(revCarry) << bitsElem);
	}
	ret.adjust();
	return ret;
}

frac::frac(long n, long d) : num(d < 0? -n : n), den(d < 0? -d : d) {
	if (!d)
		throw divisionBy0();
	reduce();
}

frac::frac(const bignum& n, const bignum& d)
	: num(d.negative()? -n : n), den(d) {
	if (d.zero())
		throw divisionBy0();
	den.removeSignal();
	reduce();
}

frac::frac(const frac& other) : num(other.num), den(other.den) {}

frac& frac::operator=(const frac& other) {
	num = other.num;
	den = other.den;
	return *this;
}

frac frac::operator+(const frac& other) const {
	return frac(num * other.den + other.num * den, den * other.den);
}

frac frac::operator-(const frac& other) const {
	return frac(num * other.den - other.num * den, den * other.den);
}

frac frac::operator*(const frac& other) const {
	frac f1(num, other.den), f2(other.num, den);
	return frac(f1.num * f2.num, f1.den * f2.den);
}

frac frac::operator/(const frac& other) const {
	frac f1(num, other.num), f2(other.den, den);
	return frac(f1.num * f2.num, f1.den * f2.den);
}

frac& frac::operator+=(const frac& other) {
	return *this = operator+(other);
}

frac& frac::operator-=(const frac& other) {
	return *this = operator-(other);
}

frac& frac::operator*=(const frac& other) {
	return *this = operator*(other);
}

frac& frac::operator/=(const frac& other) {
	return *this = operator/(other);
}

const frac& frac::operator+() const {
	return *this;
}

frac frac::operator-() const {
	frac ret(*this);
	return ret.invertSum();
}

bool frac::operator==(const frac& other) const {
	return num == other.num && den == other.den;
}

bool frac::operator!=(const frac& other) const {
	return !operator==(other);
}

bool frac::operator<(const frac& other) const {
	return num * other.den < other.num * den;
}

bool frac::operator<=(const frac& other) const {
	return !(other < *this);
}

bool frac::operator>(const frac& other) const {
	return other < *this;
}

bool frac::operator>=(const frac& other) const {
	return !operator<(other);
}

bool frac::zero() const {
	return num.zero();
}

bool frac::positive() const {
	return num.positive();
}

bool frac::negative() const {
	return num.negative();
}

bool frac::integer() const {
	return den == bn1;
}

frac& frac::invertSum() {
	num.invert();
	return *this;
}

frac& frac::invertProd() {
	if (den.zero())
		throw divisionBy0();
	num.swap(den);
	if (den.negative()) {
		num.invert();
		den.invert();
	}
	return *this;
}

frac& frac::removeSignal() {
	num.removeSignal();
	return *this;
}

/* readFromStream(std::istream& in, base b):
 * reads a number represented as a quotient of floating points or just one floating point
 * in base b from in and makes *this be that number
 * returns: in (after the operation)
 * note: the only allowed characters between the numerator and '/' are ' ' and '\t'
 * note 2: if the number is in the non-quotient form, all ' ' and '\t' after it will be
 *         consumed
 */
std::istream& frac::readFromStream(std::istream& in, base b) {
#define readskipws()								\
	do {											\
		c = in.get();								\
	} while (in.good() && (c == ' ' || c == '\t'))

#define teststream()							\
	if (!in.good())								\
		return in

	int c;
	readFromStreamPoint(in, b);
	teststream();
	readskipws();
	if (c == EOF) {
		in.clear(std::istream::goodbit);
		return in;
	}
	teststream();
	if (c != '/') {
		in.unget();
		return in;
	}
	frac sf;
	sf.readFromStreamPoint(in, b);
	teststream();
	*this /= sf;
	return in;
#undef readskipws
#undef teststream
}

/* readFromStreamPoint(std::istream& in, base b):
 * reads a floating point representation of number in base b from in and
 * makes *this be that number
 * returns: in (after the operation)
 */
std::istream& frac::readFromStreamPoint(std::istream& in, base b) {
	const bignum* basebn = 0;
	int expchar = 'p';

	int (*testfun)(int) = 0;
	switch (b) {
	case octal:
		basebn = &bn8;
		testfun = isoctal;
		break;
	case decimal:
		basebn = &bn10;
		testfun = isdigit;
		expchar = 'e';
		break;
	case hexadecimal:
		basebn = &bn16;
		testfun = isxdigit;
		break;
	}

#define basebn (*basebn)

#define testEOF(INST)							\
	if (c == EOF) {								\
		in.clear(std::istream::goodbit);		\
		INST;									\
	}


#define teststream()							\
	if (!in.good())								\
		return in

	*this = frac();

	bool realsgn = false;
	int c =	in.peek();
	testEOF(return in);
	teststream();
	if (c == '.') {
		in.ignore();
		c = in.peek();
		testEOF(return in);
		teststream();
		if (!(*testfun)(c)) {
			in.putback('.');
			in.clear(std::istream::failbit);
			return in;
		}
		goto STREAM_POINT_READ; // go directly to the block of '.'
	}
	else {
		num.readFromStream(in);
		realsgn = sgnread;
		teststream();
		c = in.peek();
		testEOF(return in);
		teststream();
	}

	if (c == '.') {
		in.ignore();
		c = in.peek();
		testEOF(return in);
		teststream();
		if ((*testfun)(c)) {
		STREAM_POINT_READ:
			bignum postPoint;
			postPoint.readFromStream(in);
			teststream();
			den = pow(basebn, charread);
			num *= den;
			if (realsgn)
				postPoint.invert();
			num += postPoint;
			c =	in.peek();
			testEOF(reduce(); return in);
			teststream();
		}
	}
	if (c == expchar || c == expchar + 'A' - 'a') {
		in.ignore();
		c = in.peek();
		teststream();
		bignum* multpart = &num;
		switch (c) {
		case '-':
			multpart = &den;
			// fall through
		case '+':
			in.ignore();
			c = in.peek();
			teststream();
			break;
		}
		if (!isdigit(c)) {
			in.clear(std::istream::failbit);
			return in;
		}
		unsigned long exp = 0;
		do {
			in.ignore();
			teststream();
			exp *= 10;
			exp += c - '0';
			c = in.peek();
			testEOF(break);
		} while (isdigit(c));
		// exponent is always in base 10
		*multpart *= pow(bn10, exp);
	}
	reduce();
	return in;
#undef basebn
#undef testEOF
#undef teststream
}

/* readFromFILE(FILE *f, base b):
 * reads a number represented as a quotient of floating points or just one floating point
 * in base b from f and makes *this be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: the only allowed characters between the numerator and '/' are ' ' and '\t'
 * note 2: if the number is in the non-quotient form, all ' ' and '\t' after it will be
 *         consumed
 */
int frac::readFromFILE(FILE *f, base b) {
#define readskipws()								\
	do {											\
		c = fgetc(f);								\
	} while (c != EOF && (c == ' ' || c == '\t'))

	int c;
	int r = readFromFILEPoint(f, b);
	if (r != 1)
		return r;
	readskipws();
	if (c == EOF) {
		if (feof(f))
			return 1;
		return EOF;
	}
	if (c != '/') {
		ungetc(c, f);
		return 1;
	}
	frac sf;
	r = sf.readFromFILEPoint(f, b);
	if (r != 1)
		return r;
	*this /= sf;
	return 1;
#undef readskipws
}

/* readFromFILEPoint(FILE *f, base b):
 * reads a floating point representation of number in base b from f and
 * makes *this be that number
 * returns: EOF, if an error occurred; 0, if read bad formatted string; 1, otherwise
 */
int frac::readFromFILEPoint(FILE *f, base b) {
	const bignum* basebn = 0;
	int expchar = 'p';
	int r;

	int (*testfun)(int) = 0;
	switch (b) {
	case octal:
		basebn = &bn8;
		testfun = isoctal;
		break;
	case decimal:
		basebn = &bn10;
		testfun = isdigit;
		expchar = 'e';
		break;
	case hexadecimal:
		basebn = &bn16;
		testfun = isxdigit;
		break;
	}

#define basebn (*basebn)

#define testEOFanderror(INST)					\
	if (c == EOF) {								\
		if (!feof(f))							\
			return EOF;							\
		INST;									\
	}

	*this = frac();

	bool realsgn = false;
	int c = fgetc(f);
	if (c == '.') {
		c = fgetc(f);
		testEOFanderror(ungetc(c, f); ungetc('.', f); return 0);
		if (!(*testfun)(c)) {
			ungetc(c, f);
			return 0;
		}
		goto FILE_POINT_READ; // go directly to the block of '.'

	}
	else {
		ungetc(c, f);

		r = num.readFromFILE(f);
		if (r != 1)
			return r;
		realsgn = sgnread;
		c = fgetc(f);
		testEOFanderror(return 1);
	}

	if (c == '.') {
		c = fgetc(f);
		testEOFanderror(return 1);
		if ((*testfun)(c)) {
		FILE_POINT_READ:
			ungetc(c, f);
			bignum postPoint;
			r = postPoint.readFromFILE(f);
			if (r != 1) // r can only be 1 or EOF
				return r;
			den = pow(basebn, charread);
			num *= den;
			if (realsgn)
				postPoint.invert();
			num += postPoint;
			c = fgetc(f);
			testEOFanderror(reduce(); return 1);
		}
	}
	if (c == expchar || c == expchar + 'A' - 'a') {
		c = fgetc(f);
		testEOFanderror(return EOF);
		bignum* multpart = &num;
		switch (c) {
		case '-':
			multpart = &den;
			// fall through
		case '+':
			c = fgetc(f);
			testEOFanderror(return EOF);
			break;
		}
		if (!isdigit(c)) {
			return 0;
		}
		unsigned long exp = 0;
		do {
			exp *= 10;
			exp += c - '0';
			c = fgetc(f);
			testEOFanderror(break);
		} while (isdigit(c));
		// exponent is always in base 10
		*multpart *= pow(bn10, exp);
	}
	ungetc(c, f);
	reduce();
	return 1;
#undef basebn
#undef testEOFanderror
}

/* putOnStreamOct(std::ostream& out) const:
 * puts a string representing *this in base 8 on stream out
 * returns: out (after operation)
 */
std::ostream& frac::putOnStreamOct(std::ostream& out) const {
    return den.putOnStreamOct(num.putOnStreamOct(out) << " / ");
}

/* putOnStreamDec(std::ostream& out) const:
 * puts a string representing *this in base 10 on stream out
 * returns: out (after operation)
 */
std::ostream& frac::putOnStreamDec(std::ostream& out) const {
    return den.putOnStreamDec(num.putOnStreamDec(out) << " / ");
}

/* putOnStreamHex(std::ostream& out) const:
 * puts a string representing *this in base 16 on stream out
 * returns: out (after operation)
 */
std::ostream& frac::putOnStreamHex(std::ostream& out) const {
    return den.putOnStreamHex(num.putOnStreamHex(out) << " / ");
}

/* putOnStream(std::ostream& out, base b) const:
 * puts a string representing *this in base b on stream out
 * returns: out (after operation)
 */
std::ostream& frac::putOnStream(std::ostream& out, base b) const {
    return den.putOnStream(num.putOnStream(out, b) << " / ", b);
}

/* putOnStreamPoint(std::ostream& out, unsigned long precision, base b) const:
 * puts a string representing an floating point approximation of *this in base b
 * with precision precision on stream out
 * returns: out (after operation)
 * note: the written string is a number in scientific notation XCY (in base b)
 *       such that XCY and *this differ by at most the (|Y - precision|)-th power
 *       of 10 for base decimal or of 2 for other bases.
 *       The character C is e for base decimal and p for other bases.
 */
std::ostream& frac::putOnStreamPoint(std::ostream& out,
									 unsigned long precision,
									 base b) const {
	if (num.zero())
		return out << '0';

    const bignum *basebn = 0;
	char expchar = 'p';
    switch (b) {
	case octal:
        basebn = &bn8;
        break;
	case decimal:
        basebn = &bn10;
		expchar = 'e';
        break;
	case hexadecimal:
        basebn = &bn16;
        break;
    }

#define basebn (*basebn)

	bignum altnum(num);
	if (num.negative()) {
		out << '-';
		altnum.invert();
	}

	bignum altden(den);
	std::stringstream expss;
	if (altnum >= altden) {
		unsigned long t = 0;
		bignum aux;
		bignum *a = &altden, *b = &aux;
		do {
			*b = *a * basebn;
			bignum *c = b;
			b = a;
			a = c;
			++t;
		} while (altnum >= *a);
		altden = *b;
		if (--t)
			expss << expchar << t; // t is in base 10
	}
	else {
		unsigned long t = 0;
		do {
			altnum *= basebn;
			++t;
		} while (altnum < altden);
		expss << expchar << '-' << t; // t is in base 10
	}

    divRes dr(divide(altnum, altden));

    dr.first.putOnStream(out, b);
    dr.second *= pow(basebn, precision);
    dr.second /= altden;
    if (!dr.second.zero()) {
        std::string str(dr.second.toString(b));
		out << '.';
		for (unsigned long i = precision - str.size(); i > 0; --i)
			out << '0';
		unsigned long end = str.size();
		for ( ; end > 0; --end)
			if (str[end-1] != '0')
				break;
		for (unsigned long i = 0; i < end; ++i)
			out << str[i];
    }
    return out << expss.str();
#undef basebn
}

/* toStringOct() const:
 * returns: a string representing *this in base 8
 */
std::string frac::toStringOct() const {
    return num.toStringOct() + " / " + den.toStringOct();
}

/* toStringDec() const:
 * returns: a string representing *this in base 10
 */
std::string frac::toStringDec() const {
    return num.toStringDec() + " / " + den.toStringDec();
}

/* toStringHex() const:
 * returns: a string representing *this in base 16
 */
std::string frac::toStringHex() const {
    return num.toStringHex() + " / " + den.toStringHex();
}

/* toString(base b) const:
 * returns: a string representing *this in base b
 */
std::string frac::toString(base b) const {
    return num.toString(b) + " / " + den.toString(b);
}

/* toStringPoint(unsigned long precision, base b) const:
 * returns: a string representing an floating point approximation of *this in base b
 *          with precision precision
 * note: the returned string is a number in scientific notation XCY (in base b)
 *       such that XCY and *this differ by at most the (|Y - precision|)-th power
 *       of 10 for base decimal or of 2 for other bases.
 *       The character C is e for base decimal and p for other bases.
 */
std::string frac::toStringPoint(unsigned long precision, base b) const {
	std::stringstream s;
	putOnStreamPoint(s, precision, b);
	return s.str();
}

/* rounded(frac prec) const:
 * returns: the (integer) multiple of prec that is closest to *this
 */
frac frac::rounded(frac prec) const {	
	if (prec.zero())
		return prec;

	fracDivRes fdr(divide(*this, prec));
	bool inverted = false;
	if (fdr.second.negative()) {
		fdr.second.invertSum();
		inverted = true;
	}
	frac comp(prec - fdr.second);
	if (comp < fdr.second)
		inverted? --fdr.first : ++fdr.first;
	return frac(fdr.first * prec.num, prec.den);
}

/* humanRounded(unsigned long n) const:
 * restrictions: n > 0
 * returns: the (integer) multiple of a fraction a / b that is closest to *this
 *          with the restriction that b <= n
 * note: this function is much slower than rounded(const frac&) but produces
 *       results somewhat closer to how a human would round a fraction
 */
frac frac::humanRounded(unsigned long n) const {
	bignum bnn(n), bni(bn1);
	frac closest(rounded(frac(bn1, bni)));
	frac dist(*this - closest);
	if (dist.negative())
		dist.invertSum();
	for (++bni; bni <= bnn; ++bni) {
		frac test(rounded(frac(bn1, bni)));
		frac testDist(*this - test);
		if (testDist.negative())
			testDist.invertSum();
		if (testDist < dist) {
			closest = test;
			dist = testDist;
		}
	}
	return closest;
}

/* semiHumanRounded(unsigned long n) const:
 * restrictions: n > 0
 * returns: the (integer) multiple of a fraction a / b that is closest to *this
 *          with the restriction that a / b = c / ascendingLcm(n) for some integer c
 * note: this function is a compromise between rounded(const frac&)'s efficiency and
 *       humanRounded(unsigned long)'s attractiveness
 * note2: if this->den is a number with very few factors (somewhat close to a power of
 *        a prime), then the returned fraction doesn't tend to be that attractive looking
 */
frac frac::semiHumanRounded(unsigned long n) const {
	return rounded(frac(bn1, ascendingLcm(n)));
}

void frac::swap(frac& other) {
	num.swap(other.num);
	den.swap(other.den);
}

/* reduce() const:
 * reduces *this (num and den become co-prime)
 */
void frac::reduce() const {
    if (num.zero()) {
        den = bn1;
        return;
    }
    bignum g = gcd(num, den);
    num /= g;
    den /= g;
}

/* pow(bignum b, unsigned long e):
 * returns: b to the power e
 */
bignum BigNum::pow(bignum b, unsigned long e) {
    bignum r(bn1);
    for ( ; e; e /= 2) {
        if (e % 2)
            r *= b;
        b *= b;
    }
    return r;
}

/* pow(frac b, long e):
 * returns: b to the power e
 */
frac BigNum::pow(frac b, long e) {
	if (e < 0) {
		e = -e;
		b.invertProd();
	}
	frac r(bn1);
    for ( ; e; e /= 2) {
        if (e % 2)
            r *= b;
        b *= b;
    }
    return r;
}

divRes BigNum::divide(bignum num, bignum den) {
	if (den.zero())
		throw divisionBy0();
	bool numSgn(num.negative()), denSgn(den.negative());
	num.removeSignal();
	den.removeSignal();
	if (num < den) {
		if (numSgn)
			num.invert();
		return divRes(bn0, num);
	}


	const unsigned long shift((num.v.size() - den.v.size() + 1) * bignum::bitsElem);
	den <<= shift;
	bignum quot, pow2(bn1 << shift);
    while (!pow2.zero()) {
		if (den <= num) {
			quot += pow2;
			num -= den;
		}
		den >>= 1;
		pow2 >>= 1;
	}
	if (numSgn xor denSgn)
		quot.invert();
	if (numSgn)
		num.invert();
	return divRes(quot, num);
}

fracDivRes BigNum::divide(frac f1, frac f2) {
	if (f2.zero())
		throw divisionBy0();
	bool f1Sgn(f1.negative()), f2Sgn(f2.negative());
	f1.removeSignal();
	f2.removeSignal();

	bignum quot, pow2(bn1);
	while (f1 >= f2) {
		f2.num <<= 1;
		pow2 <<= 1;
	}

    while (!pow2.zero()) {
		if (f2 <= f1) {
			quot += pow2;
			f1 -= f2;
		}
		f2.num >>= 1;
		pow2 >>= 1;
	}
	if (f1Sgn xor f2Sgn)
		quot.invert();
	if (f1Sgn)
		f1.invertSum();
	return fracDivRes(quot, f1);
}

/* stringToBignum(const std::string& str, base b):
 * assumes that str holds the representation of a bignum in base b
 * returns: the bignum represented by str
 */
bignum BigNum::stringToBignum(const std::string& str, base b) {
	std::stringstream s(str);
	bignum bn;
	bn.readFromStream(s, b);
	return bn;
}

/* stringPointToFrac(const std::string& str, base b):
 * assumes that str holds the floating point representation of a number in base b
 * returns: the number represented by str as a frac
 */
frac BigNum::stringPointToFrac(const std::string& str, base b) {
	std::stringstream s(str);
	frac f;
	f.readFromStreamPoint(s, b);
	return f;
}

/* stringToFrac(const std::string& str, base b):
 * assumes that str holds the representation of a quotient of floating point numbers
 * in base b
 * returns: the number represented by str as a frac
 */
frac BigNum::stringToFrac(const std::string& str, base b) {
	std::stringstream s(str);
	frac f;
	f.readFromStream(s, b);
	return f;
}

/* factorial(unsigned long n):
 * returns: n!
 */
const bignum& BigNum::factorial(unsigned long n) {
	return decrescentfactorial(n, n);
}



/* decrescentfactorial(unsigned long n, unsigned long k):
 * restrictions: k <= n
 * returns: n*(n-1)*(n-2)*...*(n-k+1)
 */
const bignum& BigNum::decrescentfactorial(unsigned long n, unsigned long k) {
	static std::deque< std::deque<bignum> > mem;
	if (k == 0)
		return bn1;
	if (n == 0)
		return bn0;
	if (n > mem.size())
		mem.resize(n);
	if (!mem[n-1].size())
		mem[n-1].resize(n);
	if (mem[n-1][k-1].zero())
		mem[n-1][k-1] = bignum(n) * decrescentfactorial(n-1,k-1);
    return mem[n-1][k-1];
}

/* binomial(unsigned long n, unsigned long k):
 * restrictions: k <= n
 */
const bignum& BigNum::binomial(unsigned long n, unsigned long k) {
	static std::deque< std::deque<bignum> > mem;
	if (k == 0 || k == n)
		return bn1;
	if (n-1 > mem.size())
		mem.resize(n-1);
	if (!mem[n-2].size())
		mem[n-2].resize(n-1);
	if (mem[n-2][k-1].zero())
		mem[n-2][k-1] = binomial(n-1,k-1) + binomial(n-1,k);
	return mem[n-2][k-1];
}

/* gcd(bignum a, bignum b):
 * restrictions: at least one of a or b is non-zero
 * returns: the greatest common divisor of a and b
 */
bignum BigNum::gcd(bignum a, bignum b) {
    bignum c;
    bignum *d1 = &a, *d2 = &b, *r = &c;
	a.removeSignal();
	b.removeSignal();
    if (b.zero()) {
		return a;
    }

    for (*r = *d1 % *d2; !r->zero(); *r = *d1 % *d2) {
        bignum *aux = d1;
        d1 = d2;
        d2 = r;
        r = aux;
    }
    return *d2;
}

/* lcm(const bignum& a, const bignum& b):
 * retuns: the lowest common multiple of a and b
 */
bignum BigNum::lcm(const bignum& a, const bignum& b) {
	if (a.zero() || b.zero())
		return bignum(0);
    return a * b / gcd(a,b);
}

/* ascendingLcm(unsigned long n):
 * restrictions: n > 0
 * returns: the lcm of [n]
 */
bignum BigNum::ascendingLcm(unsigned long n) {
	static std::deque<bignum> mem;
	if (n <= 1)
		return bn1;
	if (mem.size() < n-1)
		mem.resize(n-1);
	if (mem[n-2].zero())
		mem[n-2] = lcm(ascendingLcm(n-1), bignum(n));
	return mem[n-2];
}

/* bignumToDouble(const bignum& n):
 * returns: double representing n (there might be an overflow error)
 */
double BigNum::bignumToDouble(const bignum& n) {
    std::stringstream s;
	n.putOnStreamDec(s);
    double d;
    s >> d;
    return d;
}

/* bignumToLong(const bignum& n):
 * returns: long representing n (there might be an overflow error)
 */
long BigNum::bignumToLong(const bignum& n) {
    std::stringstream s;
	n.putOnStreamDec(s);
    long l;
    s >> l;
    return l;
}

/* fracToDouble(const frac& f):
 * returns: double approximating f (there might be an overflow error)
 */
double BigNum::fracToDouble(const frac& f) {
    std::stringstream s;
	f.putOnStreamPoint(s, 30);
    double d;
    s >> d;
    return d;
}

/* doubleToFrac(double d):
 * returns: frac that represents d
 */
frac BigNum::doubleToFrac(double d) {
	frac f;
	std::stringstream s;
	s.precision(30);
	s << d;
	s >> f;
	return f;
}

/* operator<<(std::ostream& out, const bignum& bn):
 * puts a string representing bn in base 10 on stream out
 * returns: out (after operation)
 */
std::ostream& BigNum::operator<<(std::ostream& out, const bignum& bn) {
	return bn.putOnStreamDec(out);
}

/* operator<<(std::ostream& out, const frac& f):
 * puts a string representing f in base 10 on stream out
 * returns: out (after operation)
 */
std::ostream& BigNum::operator<<(std::ostream& out, const frac& f) {
	return f.putOnStreamDec(out);
}

/* operator>>(std::istream& in, bignum& bn):
 * reads a bignum from in and puts in bn (in base 10)
 * returns: in (after operation)
 */
std::istream& BigNum::operator>>(std::istream& in, bignum& bn) {
	return bn.readFromStream(in);
}

/* operator>>(std::istream& in, frac& f):
 * reads a frac from in and puts in f (in base 10)
 * returns: in (after operation)
 */
std::istream& BigNum::operator>>(std::istream& in, frac& f) {
	return f.readFromStream(in);
}

/* fscanbignum(FILE *f, bignum *n, base b):
 * reads a representation of number in base b from f and makes *n be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: puts the number of characters read in charread
 */
int BigNum::fscanbignum(FILE *f, bignum *n, base b) {
	return n->readFromFILE(f, b);
}

/* scanbignum(bignum *n, base b):
 * reads a representation of number in base b from stdin and makes *n be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: puts the number of characters read in charread
 */
int BigNum::scanbignum(bignum *n, base b) {
	return n->readFromFILE(stdin, b);
}

/* fprintbignum(FILE *f, const bignum& n, base b) const:
 * prints a string representing n in base b on f
 */
void BigNum::fprintbignum(FILE *f, const bignum& n, base b) {
	fprintf(f, "%s", n.toString(b).c_str());
}

/* printbignum(const bignum& n, base b) const:
 * prints a string representing n in base b on f
 */
void BigNum::printbignum(const bignum& n, base b) {
	printf("%s", n.toString(b).c_str());
}

/* fscanfrac(FILE *f, frac *n, base b):
 * reads a number represented as a quotient of floating points or just one floating point
 * in base b from f and makes *n be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: the only allowed characters between the numerator and '/' are ' ' and '\t'
 * note 2: if the number is in the non-quotient form, all ' ' and '\t' after it will be
 *         consumed
 */
int BigNum::fscanfrac(FILE *f, frac *n, base b) {
	return n->readFromFILE(f, b);
}

/* scanfrac(frac *n, base b):
 * reads a number represented as a quotient of floating points or just one floating point
 * in base b from stdin and makes *n be that number
 * returns: EOF, if there was a read error; 0, if found bad formatted string; 1, otherwise
 * note: the only allowed characters between the numerator and '/' are ' ' and '\t'
 * note 2: if the number is in the non-quotient form, all ' ' and '\t' after it will be
 *         consumed
 */
int BigNum::scanfrac(frac *n, base b) {
	return n->readFromFILE(stdin, b);
}

/* fprintfrac(FILE *f, const frac& n, base b) const:
 * prints a string representing n in base b on f
 */
void BigNum::fprintfrac(FILE *f, const frac& n, base b) {
	fprintf(f, "%s", n.toString(b).c_str());
}

/* printfrac(const frac& n, base b) const:
 * prints a string representing n in base b on f
 */
void BigNum::printfrac(const frac& n, base b) {
	printf("%s", n.toString(b).c_str());
}

/* swap(bignum& a, bignum& b):
 * swaps a and b
 */
void BigNum::swap(bignum& a, bignum& b) {
	a.swap(b);
}

/* swap(frac& a, frac& b):
 * swaps a and b
 */
void BigNum::swap(frac& a, frac& b) {
	a.swap(b);
}


/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
