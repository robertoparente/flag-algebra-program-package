#ifndef BIGNUM_H_INCLUDED
#define BIGNUM_H_INCLUDED

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <cstdio>

namespace BigNum {
	// Forward declarations
    class bignum;
    class frac;

	// Exception classes
    class divisionBy0 {};
	class invalidString {};

	// Division return type
    typedef std::pair<bignum, bignum> divRes;

	// Frac division return type
	typedef std::pair<bignum, frac> fracDivRes;

	// I/O base representation type
    enum base {octal, decimal, hexadecimal};

	// class bignum implements arbitrarily big integers
    class bignum {
        public:
        bignum();
        explicit bignum(long);
        bignum(const bignum&);
        bignum& operator=(const bignum&);

        bignum operator+(const bignum&) const;
        bignum operator-(const bignum&) const;
        bignum operator*(const bignum&) const;
        bignum operator/(const bignum&) const;
        bignum operator%(const bignum&) const;
        bignum operator<<(long) const;
        bignum operator>>(long) const;

        bignum& operator+=(const bignum&);
        bignum& operator-=(const bignum&);
        bignum& operator*=(const bignum&);
        bignum& operator/=(const bignum&);
        bignum& operator%=(const bignum&);
        bignum& operator<<=(long);
        bignum& operator>>=(long);

        const bignum& operator+() const;
        bignum operator-() const;

        bool operator==(const bignum&) const;
        bool operator!=(const bignum&) const;
        bool operator<(const bignum&) const;
        bool operator<=(const bignum&) const;
        bool operator>(const bignum&) const;
        bool operator>=(const bignum&) const;

        bignum& operator++();
        bignum& operator--();

        bool zero() const;
        bool positive() const;
        bool negative() const;

		bignum& invert();
		bignum& removeSignal();

		std::istream& readFromStream(std::istream&, base = decimal);
		int readFromFILE(FILE*, base = decimal);

		std::ostream& putOnStreamOct(std::ostream&) const;
		std::ostream& putOnStreamDec(std::ostream&) const;
		std::ostream& putOnStreamHex(std::ostream&) const;
		std::ostream& putOnStream(std::ostream&, base) const;

        std::string toStringOct() const;
        std::string toStringDec() const;
        std::string toStringHex() const;
        std::string toString(base) const;

		void swap(bignum&);

	protected:
		typedef unsigned char elem_t;
		typedef unsigned short wide_t;
		static const size_t bitsElem;
		static const elem_t maxElem;

        mutable std::vector<elem_t> v;
        mutable bool sgn;

		void adjust() const;
		void shrink() const;
		bignum unsignedAdd(const bignum&, bool = true) const;
		bignum unsignedSubtract(const bignum&, bool = true) const;

		friend divRes divide(bignum, bignum);
    };

	// These are useful bignum constants
	extern const bignum bn0, bn1, bn2, bn8, bn10, bn16;

	// class frac implements arbitrary precision fractions
    class frac {
        public:
        explicit frac(long = 0, long = 1);
        explicit frac(const bignum&, const bignum& = bn1);
        frac(const frac&);
        frac& operator=(const frac&);

        frac operator+(const frac&) const;
        frac operator-(const frac&) const;
        frac operator*(const frac&) const;
        frac operator/(const frac&) const;

        frac& operator+=(const frac&);
        frac& operator-=(const frac&);
        frac& operator*=(const frac&);
        frac& operator/=(const frac&);

        const frac& operator+() const;
        frac operator-() const;

        bool operator==(const frac&) const;
        bool operator!=(const frac&) const;
        bool operator<(const frac&) const;
        bool operator<=(const frac&) const;
        bool operator>(const frac&) const;
        bool operator>=(const frac&) const;

        bool zero() const;
        bool positive() const;
        bool negative() const;
        bool integer() const;

		frac& invertSum();
		frac& invertProd();
		frac& removeSignal();

		std::istream& readFromStream(std::istream&, base = decimal);
		std::istream& readFromStreamPoint(std::istream&, base = decimal);
		int readFromFILE(FILE*, base = decimal);
		int readFromFILEPoint(FILE*, base = decimal);


		std::ostream& putOnStreamOct(std::ostream&) const;
		std::ostream& putOnStreamDec(std::ostream&) const;
		std::ostream& putOnStreamHex(std::ostream&) const;
		std::ostream& putOnStream(std::ostream&, base) const;
		std::ostream& putOnStreamPoint(std::ostream&,
									   unsigned long,
									   base = decimal) const;

        std::string toStringOct() const;
        std::string toStringDec() const;
        std::string toStringHex() const;
        std::string toString(base) const;
        std::string toStringPoint(unsigned long, base = decimal) const;

		frac rounded(frac) const;
		frac humanRounded(unsigned long) const;
		frac semiHumanRounded(unsigned long) const;

		void swap(frac&);

	protected:
        mutable bignum num, den;

        void reduce() const;

		friend fracDivRes divide(frac, frac);
    };

    bignum pow(bignum, unsigned long);
    frac pow(frac, long);

    divRes divide(bignum, bignum);
	fracDivRes divide(frac, frac);

    bignum stringToBignum(const std::string&, base = decimal);
	frac stringPointToFrac(const std::string&, base = decimal);
	frac stringToFrac(const std::string&, base = decimal);

    const bignum& factorial(unsigned long);
    const bignum& decrescentfactorial(unsigned long, unsigned long);
    const bignum& binomial(unsigned long, unsigned long);

    bignum gcd(bignum, bignum);
    bignum lcm(const bignum&, const bignum&);
	bignum ascendingLcm(unsigned long);

    double bignumToDouble(const bignum&);
	long bignumToLong(const bignum&);

    double fracToDouble(const frac&);
	frac doubleToFrac(double);

	std::ostream& operator<<(std::ostream&, const bignum&);
	std::ostream& operator<<(std::ostream&, const frac&);

	std::istream& operator>>(std::istream&, bignum&);
	std::istream& operator>>(std::istream&, frac&);

	int fscanbignum(FILE*, bignum*, base = decimal);
	int scanbignum(bignum*, base = decimal);
	void fprintbignum(FILE*, const bignum&, base = decimal);
	void printbignum(const bignum&, base = decimal);

	int fscanfrac(FILE*, frac*, base = decimal);
	int scanfrac(frac*, base = decimal);
	void fprintfrac(FILE*, const frac&, base = decimal);
	void printfrac(const frac&, base = decimal);

	void swap(bignum&, bignum&);
	void swap(frac&, frac&);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif

