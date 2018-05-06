#ifndef FLAG_H_INCLUDED
#define FLAG_H_INCLUDED

#include <vector>
#include <deque>
#include <exception>
#include <string>
#include <cstdio>
#include <limits>
#include <utility>
#include "Monitoring.h"

/* This is the container of the enumeration
 * Note: changing this container must be done with caution since some functions may
 *       assume that references to elements of this container remain valid after it
 *       resizes upward. If this is not true for the container, the macro
 *       FLAG_ALGEBRA_CONTAINER_REFERENCES should not be defined
 */
#define container std::deque
#define FLAG_ALGEBRA_CONTAINER_REFERENCES

// This is the container used for morphisms
#define mcont std::vector

#include "injective.h"
#include "bignum.h"
#include "selector.h"


namespace FlagAlgebra {
	extern unsigned long fracPrecision;
	extern unsigned long progressBarSize;

    typedef std::vector<bool> vbool;
	typedef mcont<unsigned long> morphism;

	/* This is a model class for a Universal Theory Object (UTO)
	 * Documentation of functions that should be implemented are in this file
	 * All other functions' documentations are in the files in which they are implemented
	 * Note: you should use this structure as a model, not as a base class for inheritance
	 */
    class UniversalTheoryObject {
	public:
		/* default constructor:
		 * constructs the object of size 0
		 */
		UniversalTheoryObject();

		/* copy constructor
		 */
		UniversalTheoryObject(const UniversalTheoryObject&);

		/* copy operator
		 */
		UniversalTheoryObject& operator=(const UniversalTheoryObject&);

		/* size() const:
		 * returns: size of the object
		 */
		unsigned long size() const;

		/* subobject(const vbool& v) const:
		 * restrictions: v.size() == size()
		 * returns: subobject induced by the vertices whose index i satisfies v[i] == true
		 * note: the vertices must remain in the same order and be the integers from 0 to
		 *       the new size minus one.
		 */
		UniversalTheoryObject subobject(const vbool&) const;

		/* completeIsomorphism(const UniversalTheoryObject& other, morphism& f) const:
		 * assumes f holds a partial function from the vertices of *this to the vertices
		 * of other (indicating undefined values by f[v] = other.size()) and tries to
		 * complete f to an isomorphism.
		 * returns: true, if an isomorphism exists (f holds the isomorphism found)
		 *          false, if no such isomorphism exists (f's data may be corrupted)
		 * note: this function should not assume that the partially defined f is an
		 *       is an isomorphism between the subobjects induced by its defined
		 *       domain and its defined image
		 */
		bool completeIsomorphism(const UniversalTheoryObject&, morphism&) const;

		/* findIsomorphism(const UniversalTheoryObject& other, morphism& f) const:
		 * tries to find an isomorphism between *this and other.
		 * returns: true, if an isomorphism exists (f holds the isomorphism found)
		 *          false, if no such isomorphism exists (f's data may be corrupted)
		 */
		bool findIsomorphism(const UniversalTheoryObject&, morphism&) const;

		/* operator==(const UniversalTheoryObject& other) const:
		 * returns: true, iff *this and other are isomorphic
		 */
		bool operator==(const UniversalTheoryObject&) const;

		/* readFromFile(FILE *f):
		 * reads an UTO from file f and puts it on *this
		 * returns: the number of objects read or a negative number in case of error
		 *          guarantees that the object is in a valid state upon return
		 */
		int readFromFile(FILE*);

		/* writeOnFile(FILE *f) const:
		 * writes *this on file f
		 */
		void writeOnFile(FILE*) const;


		/* static container<UniversalTheoryObject> enumerate(unsigned long s):
		 * returns: a container with every object of size s
		 * (each one appearing exactly once)
		 */
		static container<UniversalTheoryObject> enumerate(unsigned long);
    };

	// I/O functions for a UTO
	template<class UTO>
	int fscanobj(FILE*, UTO*);

	template<class UTO>
	void fprintobj(FILE*, const UTO&);

	template<class UTO>
	int scanobj(UTO*);

	template<class UTO>
	void printobj(const UTO&);

	// Forward declaration
	template<class UTO>
	class Flag;

	// Enumerating functions
	template<class UTO>
	const container<UTO>& enumerateObjects(unsigned long);

	template<class UTO>
	container< Flag<UTO> > enumerateNormalizedTypes(unsigned long);

	template<class UTO>
	const container< Flag<UTO> >& enumerateFlags(unsigned long,
												 const UTO&,
												 morphism&);

	template<class UTO>
	container< Flag<UTO> > enumerateFlags(unsigned long, const Flag<UTO>&);

	/* Template class for flags of a given Universal Theory
	 * (the Universal Theory class must have implemented the funcionality described
	 * by the UTO model class
	 */
	template<class UTO>
	class Flag {
	public:
		typedef UTO object_type;

		Flag();
		Flag(const Flag<UTO>&);
		Flag(const UTO&, const morphism&);
		Flag<UTO>& operator=(const Flag<UTO>&);

		unsigned long size() const;
		unsigned long typeSize() const;
		vbool labeledVertices() const;
		UTO unlabeledType() const;
		Flag<UTO> type() const;
		const UTO& object() const;
		unsigned long objectIndex() const;
		const morphism& typeMorphism() const;

		Flag<UTO> subflag(const vbool&) const;
		Flag<UTO> safeSubflag(const vbool&) const;
		bool completeIsomorphism(const Flag<UTO>&, morphism&) const;
		bool findIsomorphism(const Flag<UTO>&, morphism&) const;
		bool operator==(const Flag<UTO>&) const;
		bool operator!=(const Flag<UTO>&) const;

		BigNum::frac normFactor() const;
	private:
		const UTO* obj;
		unsigned long objIndex;
		morphism typev;
		mutable unsigned long norm;

		Flag(const UTO*, unsigned long, const morphism&);
		Flag<UTO> normTypeFlag(const container< Flag<UTO> >** = 0, morphism* = 0) const;

		friend container< Flag<UTO> > enumerateNormalizedTypes<UTO>(unsigned long);
		friend const container< Flag<UTO> >& enumerateFlags<UTO>(unsigned long,
																 const UTO&,
																 morphism&);
		friend container< Flag<UTO> > enumerateFlags<UTO>(unsigned long,
														  const Flag<UTO>&);
	};

	// I/O functions for a flag

	template<class UTO>
	int fscanflag(FILE*, Flag<UTO>*);

	template<class UTO>
	void fprintflag(FILE*, const Flag<UTO>&);

	template<class UTO>
	int scanflag(Flag<UTO>*);

	template<class UTO>
	void printflag(const Flag<UTO>&);


	// Flag Algebra expansion functions
	// Object expansion
	typedef std::pair<unsigned long, BigNum::frac> coeff_type;

	template<class UTO>
	const container< container<coeff_type> >& expandObjectEnumerate(unsigned long,
																	unsigned long);

	template<class UTO>
	const container< container< container<coeff_type> > >&
	expandObjectProductEnumerate(unsigned long, unsigned long, unsigned long);


	// Flag expansion
	template<class UTO>
	const container< container<coeff_type> >& expandFlagEnumerate(const UTO&,
																  unsigned long,
																  unsigned long,
																  morphism&);

	template<class UTO>
	const container< container< container<coeff_type> > >&
	expandFlagProductEnumerate(const UTO&,
							   unsigned long,
							   unsigned long,
							   unsigned long,
							   morphism&);


	// SDP problem functions
	template<class UTO>
	container<BigNum::bignum>
	fprintSDPproblem(FILE*,
					 bool,
					 unsigned long,
					 const container<coeff_type>&,
					 unsigned long,
					 const vbool&,
					 unsigned long,
					 const container<UTO>&,
					 const container<unsigned long>&,
					 const container<vbool>&,
					 BigNum::bignum = BigNum::bn0,
					 const Monitoring::ProgressKeeper& = Monitoring::ProgressKeeper());

	template<class UTO>
	container<BigNum::bignum>
	printSDPproblem(bool,
					unsigned long,
					const container<coeff_type>&,
					unsigned long,
					const vbool&,
					unsigned long,
					const container<UTO>&,
					const container<unsigned long>&,
					const container<vbool>&,
					BigNum::bignum = BigNum::bn0,
					const Monitoring::ProgressKeeper& = Monitoring::ProgressKeeper());

	/* Exception class: this class is used whenever a function of namespace FlagAlgebra
	 * needs to throw an exception
	 */
	class FlagException : public std::exception {
	public:
		FlagException() throw();
		FlagException(const FlagException&) throw();
		FlagException& operator=(const FlagException&) throw();
		explicit FlagException(std::string) throw();

		virtual ~FlagException() throw();

		virtual const char* what() const throw();
	private:
		std::string s;
	};


	// This include is intentionally inside namespace FlagAlgebra
	// and the file has the namespace templates' definitions
#include "FlagTemplates.cpp"
}


/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
