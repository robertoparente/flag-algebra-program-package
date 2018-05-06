#ifndef FLAGTEMPLATES_CPP_INCLUDED
#define FLAGTEMPLATES_CPP_INCLUDED

// I/O functions for a UTO
/* fscanobj(FILE *f, UTO *uto):
 * reads *uto from file f
 * returns: the number of objects read or a negative number in case of error
 *          guarantees that the *uto is in a valid state upon return
 */
template<class UTO>
int fscanobj(FILE *f, UTO *uto) {
	return uto->readFromFile(f);
}

/* fprintobj(FILE *f, const UTO& uto):
 * writes uto on file f
 */
template<class UTO>
void fprintobj(FILE *f, const UTO& uto) {
	uto.writeOnFile(f);
}

/* scanobj(UTO *uto):
 * reads *uto from stdin
 * returns: the number of objects read or a negative number in case of error
 *          guarantees that the *uto is in a valid state upon return
 */
template<class UTO>
int scanobj(UTO *uto) {
	return uto->readFromFile(stdin);
}

/* printobj(const UTO& uto):
 * writes uto on file stdout
 */
template<class UTO>
void printobj(const UTO& uto) {
	uto.writeOnFile(stdout);
}


// class Flag functions
/* default constructor:
 * constructs flag of size 0
 */
template<class UTO>
Flag<UTO>::Flag() : obj(), objIndex(), typev(), norm() {
	obj = &enumerateObjects<UTO>(0)[0];
}

/* copy constructor
 */
template<class UTO>
Flag<UTO>::Flag(const Flag<UTO>& other) : obj(other.obj),
										  objIndex(other.objIndex),
										  typev(other.typev),
										  norm(other.norm) {}

/* Flag(const UTO& ob, const morphism& v):
 * constructs a flag whose base object is ob and whose type is specified by v
 * note: internally, the flag does not keep a copy of the base object, instead it keeps
 *       a pointer to the UTO isomorphic to the base object that is enumerated by
 *       enumerateObjects; this means that calling object does not return ob, but an
 *       isomorphic UTO instead.
 *       It also keeps the index objIndex of ob in enumerateObjects(ob.size())
 */
template<class UTO>
Flag<UTO>::Flag(const UTO& ob, const morphism& v) : obj(),
													objIndex(0),
													typev(v.size()),
													norm() {
	const container<UTO>& objEnum(enumerateObjects<UTO>(ob.size()));

	morphism f;
	for ( ;
		  /** The below shouldn't be needed, so it is commented out
		  objIndex < objEnum.size()
		  **/
		  ;
		  ++objIndex)
		if (ob.findIsomorphism(objEnum[objIndex], f))
			break;

	obj = &objEnum[objIndex];
	for (unsigned long j = 0; j < v.size(); ++j)
		typev[j] = f[v[j]];
}

/* Flag(const UTO* ob, unsigned long obInd, const morphism& v):
 * constructs a flag whose base object is ob, its index is obInd
 * and whose type is specified by v
 * note: this constructor is private because it assumes the pointer is to the UTO
 *       enumerated by enumerateObjects and that obInd is its index
 */
template<class UTO>
Flag<UTO>::Flag(const UTO* ob, unsigned long obInd, const morphism& v) : obj(ob),
																		 objIndex(obInd),
																		 typev(v),
																		 norm() {}

/* copy operator
 */
template<class UTO>
Flag<UTO>& Flag<UTO>::operator=(const Flag<UTO>& other) {
	obj = other.obj;
	objIndex = other.objIndex;
	typev = other.typev;
	norm = other.norm;
	return *this;
}

/* size() const:
 * returns: size of the flag
 */
template<class UTO>
unsigned long Flag<UTO>::size() const {
	return obj->size();
}

/* typeSize() const:
 * returns: size of the type of the flag
 */
template<class UTO>
unsigned long Flag<UTO>::typeSize() const {
	return typev.size();
}

/* labeledVertices() const:
 * returns: vbool indicating which vertices are labeled in the flag
 */
template<class UTO>
vbool Flag<UTO>::labeledVertices() const {
	vbool s(size());

	for (unsigned long i = 0; i < typev.size(); ++i)
		s[typev[i]] = true;
	return s;
}

/* unlabeledType() const:
 * returns: type of the flag unlabeled
 */
template<class UTO>
UTO Flag<UTO>::unlabeledType() const {
	return obj->subobject(labeledVertices());
}

/* type() const:
 * returns: type of the flag
 */
template<class UTO>
Flag<UTO> Flag<UTO>::type() const {
	return subflag(labeledVertices());
}

/* object() const:
 * returns: flag's base object
 */
template<class UTO>
const UTO& Flag<UTO>::object() const {
	return *obj;
}

/* objectIndex() const:
 * returns: flag's base object index in enumerateObjects(object().size())
 */
template<class UTO>
unsigned long Flag<UTO>::objectIndex() const {
	return objIndex;
}

/* typeMorphism() const:
 * returns: flag's type morphism
 */
template<class UTO>
const morphism& Flag<UTO>::typeMorphism() const {
	return typev;
}

/* subflag(const vbool& v) const:
 * restrictions: v.size() == size()
 *               v is a superset of labeledVertices()
 * returns: subflag induced by the vertices whose index i satisfies v[i] == true
 */
template<class UTO>
Flag<UTO> Flag<UTO>::subflag(const vbool& v) const {
	unsigned long cnt = 0;
	morphism f(v.size(), size());
	for (unsigned long i = 0; i < v.size(); ++i) {
		if (v[i])
			f[i] = i-cnt;
		else
			++cnt;
	}
	morphism g(typev.size());
	for (unsigned long i = 0; i < g.size(); ++i)
		g[i] = f[typev[i]];
	return Flag<UTO>(obj->subobject(v), g);
	// since subobject is applied, we need the public constructor here
}

/* safeSubflag(const vbool& v) const:
 * safe version of subflag (tests restrictions)
 * returns: subflag induced by the vertices whose index i satisfies v[i] == true
 * throws: FlagException, if subflag restrictions are not met
 */
template<class UTO>
Flag<UTO> Flag<UTO>::safeSubflag(const vbool& v) const {
	if (v.size() != size())
		throw FlagException("safeSubflag: subflag selector and flag's sizes differ");

	vbool s(labeledVertices());
	for (unsigned long i = 0; i < s.size(); ++i)
		if (s[i] && !v[i])
			throw FlagException("safeSubflag: "
								"subflag selector is not a superset of type");
	return subflag(v);
}

/* completeIsomorphism(const Flag<UTO>& other, morphism& f) const:
 * assumes f holds a partial function from the vertices of *this to the vertices
 * of other (indicating undefined values by f[v] = other.size()) and tries to
 * complete f to an isomorphism (of flags).
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 * note: this function should not assume that the partially defined f is an
 *       is an isomorphism between the subobjects induced by its defined
 *       domain and its defined image
 */
template<class UTO>
bool Flag<UTO>::completeIsomorphism(const Flag<UTO>& other, morphism& f) const {
	if (size() != other.size() || typeSize() != other.typeSize())
		return false;
	for (unsigned long i = 0; i < typev.size(); ++i)
		if (f[typev[i]] == other.typev[i])
			continue;
		else if (f[typev[i]] == other.size())
			f[typev[i]] = other.typev[i];
		else
			return false;
	return obj->completeIsomorphism(*other.obj, f);
}

/* findIsomorphism(const Flag<UTO>& other, morphism& f) const:
 * tries to find an isomorphism between *this and other.
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 */
template<class UTO>
bool Flag<UTO>::findIsomorphism(const Flag<UTO>& other, morphism& f) const {
	f = morphism(size(), other.size());
    return completeIsomorphism(other, f);
}

/* operator==(const Flag<UTO>& other) const:
 * returns: true, iff *this and other are isomorphic (as flags)
 */
template<class UTO>
bool Flag<UTO>::operator==(const Flag<UTO>& other) const {
	morphism f;
	return findIsomorphism(other, f);
}

/* operator=!(const Flag<UTO>& other) const:
 * returns: false, iff *this and other are isomorphic (as flags)
 */
template<class UTO>
bool Flag<UTO>::operator!=(const Flag<UTO>& other) const {
	morphism f;
	return !findIsomorphism(other, f);
}

/* normFactor() const:
 * returns: the normalizing factor of the flag (to type 0)
 */
template<class UTO>
BigNum::frac Flag<UTO>::normFactor() const {
	if (!norm) {
		const container< Flag<UTO> >* pcont;

		Flag<UTO> nflag(normTypeFlag(&pcont));

		typename container< Flag<UTO> >::const_iterator it =
			find(pcont->begin(), pcont->end(), nflag);
		/** The below shouldn't be needed, so it is commented out
		if (it == cont.end())
			throw FlagException("normFactor: can't find flag on enum");
		**/
		norm = it->norm;
	}
	return BigNum::frac(BigNum::bignum(norm),
						BigNum::decrescentfactorial(size(), typeSize()));
}

/* normTypeFlag(const container< Flag<UTO> >**, morphism* pm):
 * returns: corresponding flag of normalized type and puts the isomorphism from
 *          the normalized type labels to type() labels on *pm and the address of
 *          the enumerating container that has the normalized flag on *ppcont
 */
template<class UTO>
Flag<UTO> Flag<UTO>::normTypeFlag(const container< Flag<UTO> >** ppcont,
										 morphism* pm) const {
	Flag<UTO> tp(type());
	morphism f, g(tp.size());
	if (ppcont) {
		*ppcont = &enumerateFlags<UTO>(size(), *tp.obj, f);
	}
	else {
		enumerateFlags<UTO>(size(), *tp.obj, f);
	}

	morphism tinv(tp.size());

	for (unsigned long i = 0; i < tinv.size(); ++i)
		tinv[tp.typev[i]] = i;

	if (pm) {
		pm->resize(tp.size());
		for (unsigned long i = 0; i < pm->size(); ++i)
			(*pm)[i] = tinv[f[i]];
		for (unsigned long i = 0; i < g.size(); ++i)
			g[i] = typev[(*pm)[i]];
	}
	else {
		for (unsigned long i = 0; i < f.size(); ++i)
			g[i] = typev[tinv[f[i]]];
	}

	return Flag<UTO>(obj, objIndex, g); // private constructor (faster)
}

// I/O functions for a flag
/* fscanflag(FILE *f, Flag<UTO> *flag):
 * reads a Flag<UTO> from file f and puts it on *flag
 * returns: 1 if successfull,
 *          -1 if there was an error (in this case, leaves *flag unchanged)
 */
template<class UTO>
int fscanflag(FILE *f, Flag<UTO> *flag) {
	UTO uto;
	if (fscanf(f, " (") || uto.readFromFile(f) != 1 || fscanf(f, " ,"))
		return -1;
	morphism g;
	unsigned long t;
	while (fscanf(f, "%lu", &t) == 1)
		g.push_back(t);
	if (fscanf(f, " )"))
		return -1;
	*flag = Flag<UTO>(uto, g);
	return 1;
}

/* fprintflag(FILE *f, const Flag<UTO>& flag):
 * writes flag on file f
 */
template<class UTO>
void fprintflag(FILE *f, const Flag<UTO>& flag) {
	fprintf(f, "(");
    flag.object().writeOnFile(f);
	fprintf(f, ",");
	const morphism& g(flag.typeMorphism());
	for (unsigned long i = 0; i < g.size(); ++i)
		fprintf(f, " %lu", g[i]);
	fprintf(f, ")");
}

/* scanflag(Flag<UTO> *flag):
 * reads a Flag<UTO> from stdin and puts it on *flag
 * returns: 1 if successfull, -1 if there was an error
 */
template<class UTO>
int scanflag(Flag<UTO> *flag) {
	return fscanflag(stdin, flag);
}

/* printflag(const Flag<UTO>& flag):
 * writes flag on stdout
 */
template<class UTO>
void printflag(const Flag<UTO>& flag) {
	fprintflag(stdout, flag);
}


// Enumerating functions
/* enumerateObjects(unsigned long s):
 * this function guarantees that UTO::enumerate gets called only once for each size
 * returns: container with every UTO object of size s (each one appearing exactly once)
 */
template<class UTO>
const container<UTO>& enumerateObjects(unsigned long s) {
	static std::deque< container<UTO> > mem;

	if (mem.size() <= s)
		mem.resize(s+1);
	if (mem[s].size())
		return mem[s];
   	return mem[s] = UTO::enumerate(s);
}

/* enumerateNormalizedTypes(unsigned long s):
 * this (auxiliary) function enumerates all normalized types of size s (those that
 * have identity as typev and base object enumerated by enumerateObjects (or are
 * isomorphic to one such)) and it guarantees that the actual enumeration gets done
 * only once.
 * returns: container with every normalized type of size s (each one appearing exactly
 *          once)
 */

template<class UTO>
container< Flag<UTO> > enumerateNormalizedTypes(unsigned long s) {
	container< Flag<UTO> > ret;
	const container<UTO>& cont(enumerateObjects<UTO>(s));

	morphism f(s);
	for (unsigned long i = 0; i < f.size(); ++i)
		f[i] = i;

	for (unsigned long i = 0; i < cont.size(); ++i)
		ret.push_back(Flag<UTO>(&cont[i], i, f)); // private constructor (faster)
	return ret;
}

/* enumerateFlags(unsigned long s, const UTO& utp, morphism& f):
 * restrictions: s >= utp.size()
 * this function enumerates all flags of size s and type a labeling of utp (determined by
 * this function) and it guarantees that the actual enumeration gets done only once,
 * but to save space, only normalized types (those that have identity as typev and base
 * object enumerated by enumerateObjects (or are isomorphic to one such)) are kept stored.
 * returns: container with every flag of size s and type Flag(utp,f)
 *          f's initial value is ignored
 */
template<class UTO>
const container< Flag<UTO> >& enumerateFlags(unsigned long s,
											 const UTO& utp,
											 morphism& f) {
	static std::deque< std::deque< std::deque< container< Flag<UTO> > > > > mem;

	const unsigned long indt(utp.size());
	const unsigned long inds(s-indt);

	if (mem.size() <= indt)
		mem.resize(indt+1);
	if (mem[indt].size() <= inds)
		mem[indt].resize(inds+1);
	if (!mem[indt][inds].size()) {
		const container< Flag<UTO> >& ntpEnum(enumerateNormalizedTypes<UTO>(utp.size()));
		const container<UTO>& baseObjEnum(enumerateObjects<UTO>(s));
		mem[indt][inds].resize(ntpEnum.size());

		Injective::injective< mcont<unsigned long> > tv(utp.size(), s);
		do {
			for (unsigned long i = 0; i < baseObjEnum.size(); ++i) {
				Flag<UTO> flag(&baseObjEnum[i], i, tv); // private constructor (faster)
				Flag<UTO> tp(flag.type());

				unsigned long index = 0;
				for ( ; index < ntpEnum.size(); ++index)
					if (tp == ntpEnum[index])
						break;
				if (index != ntpEnum.size()) {
					container< Flag<UTO> >& flagcont(mem[indt][inds][index]);
					typename container< Flag<UTO> >::iterator jt =
						find(flagcont.begin(), flagcont.end(), flag);
					if (jt == flagcont.end()) {
						flag.norm = 1;
						flagcont.push_back(flag);
					}
					else {
						++jt->norm;
					}
				}
			}
		} while (tv.next());
	}

	const container<UTO>& utpEnum(enumerateObjects<UTO>(utp.size()));
	unsigned long index = 0;
	for ( ;
		  /** This should never happen so it is commented out
		  index < utpEnum.size()
		  **/
		  ;
		  ++index)
		if (utp.findIsomorphism(utpEnum[index], f))
			break;
	/** The below should never happen, so it is commented out
	if (index == utpEnum.size())
		throw FlagException("enumerateFlags: can't find unlabeled type on enum");
	**/
	return mem[indt][inds][index];
}



/* enumerateFlags(unsigned long s, const Flag<UTO>& tp):
 * restrictions: s >= tp.size()
 *               tp is a type (i.e. tp.size() == tp.typeSize())
 * this function enumerates all flags of size s and type tp and it guarantees that the
 * actual enumeration gets done only once, but to save space, only normalized types
 * (those that have identity as typev and base object enumerated by enumerateObjects
 * (or are isomorphic to one such)) are kept stored, so the function copies
 * a stored container and may have to alter it as well.
 * returns: container with every flag of size s and type tp.
 */
template<class UTO>
container< Flag<UTO> > enumerateFlags(unsigned long s, const Flag<UTO>& tp) {
	morphism f;
	const container< Flag<UTO> >& cont(enumerateFlags<UTO>(s, *tp.obj, f));

	morphism finv(f.size());
	for (unsigned long i = 0; i < finv.size(); ++i)
		finv[f[i]] = i;

	morphism partial(f.size());
	for (unsigned long i = 0; i < partial.size(); ++i)
		partial[i] = finv[tp.typev[i]];

	container< Flag<UTO> > ret;
	for (unsigned long i = 0; i < cont.size(); ++i) {
		morphism g(f.size());
		for (unsigned long j = 0; j < g.size(); ++j)
			g[j] = cont[i].typev[partial[j]];

		ret.push_back(Flag<UTO>(cont[i].obj, cont[i].objectIndex(),	g));
		// private constructor (faster)
	}
	return ret;
}


// Flag Algebra expansion functions
// Object expansion
/* expandObjectEnumerate(unsigned long baseSize,
 *                       unsigned long expSize):
 * restrictions: expSize >= baseSize()
 * returns: matrix `ret' of flag algebra expansion coefficients of UTOs of
 *          from size baseSize to size expSize
 *          ret[i][j].second holds the coefficient of the
 *          (ret[i][j].first)-th UTO (size expSize) of the expansion of the
 *          i-th UTO (size baseSize), where the indexes are in the order of
 *          enumeration by enumerateObjects
 */
template<class UTO>
const container< container<coeff_type> >& expandObjectEnumerate(unsigned long baseSize,
																unsigned long expSize) {
#ifdef FLAG_ALGEBRA_CONTAINER_REFERENCES
	// references in container are well-behaved
	return expandObjectProductEnumerate<UTO>(0, baseSize, expSize)[0];
#else
	static std::deque< std::deque< container< container<coeff_type> > > > mem;

	const unsigned long indExp(expSize - baseSize);
	if (baseSize >= mem.size())
		mem.resize(baseSize+1);
	if (indExp >= mem[baseSize].size())
		mem[baseSize].resize(indExp+1);
	if (!mem[baseSize][indExp].size())
		mem[baseSize][indExp] =
			expandObjectProductEnumerate<UTO>(0, baseSize, expSize)[0]; // copy container
	return mem[baseSize][indExp];
#endif
}

/* expandObjectProductEnumerate(unsigned long base1Size,
 *                              unsigned long base2Size,
 *                              unsigned long expSize):
 * restrictions: expSize >= base1Size() + base2Size()
 * returns: matrix `ret' of flag algebra product coefficients of UTOs of
 *          from sizes base1Size and base2Size to size expSize
 *          ret[i][j][k].second holds the coefficient of the
 *          (ret[i][j][k].first)-th UTO (size expSize)
 *          of the expansion of the product between the i-th and j-th UTOs
 *          (sizes base1Size and base2Size respectively),
 *          where the indexes are in the order of enumeration by enumerateObjects
 */
template<class UTO>
const container< container< container<coeff_type> > >&
expandObjectProductEnumerate(unsigned long base1Size,
							 unsigned long base2Size,
							 unsigned long expSize) {
	UTO uto;
	morphism f;
	return expandFlagProductEnumerate(uto, base1Size, base2Size, expSize, f);
}

// Flag expansion
/* expandFlagEnumerate(const UTO& utp,
 *                     unsigned long baseSize,
 *                     unsigned long expSize,
 *                     morphism& f):
 * restrictions: expSize >= baseSize() >= utp.size()
 * returns: matrix `ret' of flag algebra expansion coefficients of flags of
 *          unlabeled type utp from size baseSize to size expSize
 *          ret[i][j].second holds the coefficient of the
 *          (ret[i][j].first)-th flag (size expSize)
 *          of the expansion of the i-th flag (size baseSize),
 *          where the indexes are in the order of enumeration by enumerateFlags
 */
template<class UTO>
const container< container<coeff_type> >& expandFlagEnumerate(const UTO& utp,
															  unsigned long baseSize,
															  unsigned long expSize,
															  morphism& f) {
#ifdef FLAG_ALGEBRA_CONTAINER_REFERENCES
	return expandFlagProductEnumerate(utp, utp.size(), baseSize, expSize, f)[0];
#else
	static std::deque< std::deque< std::deque< std::deque< container< container<coeff_type> > > > > > mem;

	const container<UTO>& utpEnum(enumerateObjects<UTO>(utp.size()));
	const unsigned long indTypeSize(utp.size());
	unsigned long indType = 0;
	for ( ;
		  /** This should never happen so it is commented out
		  indType < utpEnum.size()
		  **/
		  ;
		  ++indType)
		if (utp.findIsomorphism(utpEnum[indType], f))
			break;
	const unsigned long indBase(baseSize - utp.size())
	const unsigned long indExp(expSize - baseSize);

	if (indTypeSize >= mem.size())
		mem.resize(indTypeSize+1);
	if (!mem[indTypeSize].size())
		mem[indTypeSize].resize(utpEnum.size());
	if (indBase >= mem[indTypeSize][indType].size())
		mem[indTypeSize][indType].resize(indBase+1);
	if (indExp >= mem[indTypeSize][indType][baseSize].size())
		mem[indTypeSize][indType][baseSize].resize(indExp+1);
	if (!mem[indTypeSize][indType][baseSize][indExp].size())
		mem[indTypeSize][indType][baseSize][indExp] =
			expandFlagProductEnumerate<UTO>(utp, utp.size(), baseSize, expSize)[0];
	// copy container
	return mem[indTypeSize][indType][baseSize][indExp];
#endif
}

/* const container< container< container<coeff_type> > >&
 * expandFlagProductEnumerate(const UTO& utp,
 *                            unsigned long base1Size,
 *                            unsigned long base2Size,
 *                            unsigned long expSize,
 *                            morphism& f):
 * restrictions: expSize >= base1Size() + base2Size() - utp.size()
 *               base1Size() >= utp.size()
 *               base2Size() >= utp.size()
 * returns: matrix `ret' of flag algebra product coefficients of flags of
 *          unlabeled type utp from sizes base1Size and base2Size to size expSize
 *          ret[i][j][k].second holds the coefficient of the
 *          (ret[i][j][k].first)-th flag (size expSize)
 *          of the expansion of the product between the i-th and j-th flags
 *          (sizes base1Size and base2Size respectively),
 *          where the indexes are in the order of enumeration by enumerateFlags
 */
template<class UTO>
const container< container< container<coeff_type> > >&
expandFlagProductEnumerate(const UTO& utp,
						   unsigned long base1Size,
						   unsigned long base2Size,
						   unsigned long expSize,
						   morphism& f) {
	const container<UTO>& utpEnum(enumerateObjects<UTO>(utp.size()));
	const container< Flag<UTO> >& contBase1(enumerateFlags<UTO>(base1Size, utp, f));
	const container< Flag<UTO> >& contBase2(enumerateFlags<UTO>(base2Size, utp, f));
	const container< Flag<UTO> >& contExp(enumerateFlags<UTO>(expSize, utp, f));

	static std::deque< std::deque< std::deque< std::deque< std::deque< container< container< container<coeff_type> > > > > > > > mem;

	const unsigned long indTypeSize(utp.size());
	unsigned long indType = 0;
	for ( ;
		  /** This should never happen so it is commented out
		  indType < utpEnum.size()
		  **/
		  ;
		  ++indType)
		if (utp.findIsomorphism(utpEnum[indType], f))
			break;
	const unsigned long indBase1(base1Size - utp.size());
	const unsigned long indBase2(base2Size - utp.size());
	const unsigned long indExp(expSize - base1Size  + utp.size() - base2Size);
	const unsigned long diffSize1(indBase1);
	const unsigned long diffSize2(indBase2);
	const unsigned long diffSizeExp(expSize - utp.size());

	if (indTypeSize >= mem.size())
		mem.resize(indTypeSize+1);
	if (!mem[indTypeSize].size())
		mem[indTypeSize].resize(utpEnum.size());
	if (indBase1 >= mem[indTypeSize][indType].size())
		mem[indTypeSize][indType].resize(indBase1+1);
	if (indBase2 >= mem[indTypeSize][indType][indBase1].size())
		mem[indTypeSize][indType][indBase1].resize(indBase2+1);
	if (indExp >= mem[indTypeSize][indType][indBase1][indBase2].size())
		mem[indTypeSize][indType][indBase1][indBase2].resize(indExp+1);

	container< container< container<coeff_type> > >& ret
		(mem[indTypeSize][indType][indBase1][indBase2][indExp]);
	if (!ret.size()) {
		ret.resize(contBase1.size(),
				   container< container<coeff_type> >(contBase2.size()));


		BigNum::bignum normfactor(BigNum::binomial(diffSizeExp, diffSize1)
								  * BigNum::binomial(diffSizeExp - diffSize1, diffSize2));

		for (unsigned long k = 0; k < contExp.size(); ++k) {
			const Flag<UTO>& expFlag(contExp[k]);
			const vbool& labeledVertices(expFlag.labeledVertices());
			std::vector< std::vector<unsigned long> > m
				(contBase1.size(),
				 std::vector<unsigned long>(contBase2.size()));

			vbool sub1(labeledVertices);
			Selector::selector<vbool::iterator> sel1(sub1.begin(),
													 sub1.end(),
													 sub1.begin(),
													 sub1.end(),
													 true);

			for (unsigned long a = 0; a < diffSize1; ++a)
				sel1[sel1.size()-a-1] = true;
			do {
				Flag<UTO> subflag1(expFlag.subflag(sub1));
				for (unsigned long i = 0;
					 /** This should never happen so it is commented out
						 i < contBase1.size()
					 **/
					 ;
					 ++i) {
					if (subflag1 == contBase1[i]) {
						vbool sub2(labeledVertices);
						Selector::selector<vbool::iterator> sel2
							(sub2.begin(), sub2.end(), sub1.begin(), sub1.end(), true);

						for (unsigned long a = 0; a < diffSize2; ++a)
							sel2[sel2.size()-a-1] = true;
						do {
							Flag<UTO> subflag2(expFlag.subflag(sub2));
							for (unsigned long j = 0;
								 /** This should never happen so it is commented out
									 j < contBase2.size()
								 **/
								 ;
								 ++j) {
								if (subflag2 == contBase2[j]) {
									++m[i][j];
									break;
								}
							}
						} while (std::next_permutation(sel2.begin(), sel2.end()));
						break;
					}
				}
			} while (std::next_permutation(sel1.begin(), sel1.end()));
			for (unsigned long i = 0; i < ret.size(); ++i)
				for (unsigned long j = 0; j < ret[i].size(); ++j)
					if (m[i][j])
						ret[i][j]
							.push_back(coeff_type(k,
												  BigNum::frac(BigNum::bignum(m[i][j]),
															   normfactor)));
		}
	}
	return ret;
}

// SDP problem functions
/* fprintSDPproblem(FILE* file,
 *					bool min,
 *                  unsigned long countObjsSize,
 *					const container<coeff_type>& countObjsCoeffs,
 *                  unsigned long forbObjsSize,
 *					const vbool& forbObjsSelect,
 *					unsigned long expSize,
 *					const container<UTO>& utypes,
 *					const container<unsigned long>& flagSizes,
 *					const container<vbool>& flagSelect,
 *                  BigNum::bignum scale,
 *                  Monitoring::ProgressKeeper& progress):
 * restrictions: expSize >= countObjsSize;
 *               expSize >= forbObjsSize;
 *               utypes.size() == flagSizes.size() == flagSelect.size();
 *               for every 0 <= i < utypes.size(), we must have
 *                 flagSelect[i].size() ==
 *                   enumerateFlags(flagSizes[i], utypes[i], morphism()).size()
 *                 2*flagSizes[i] - utypes[i].size() <= expSize
 * prints on file the Semi-Definite-Programming Problem of minimizing/maximizing
 * (minimizing, iff min=true) the function of induced subUTOs of size countObjsSize
 * given by the sum over i of
 *
 * countObjsCoeffs[i].second * (countObjsCoeffs[i].first)-th UTO 
 *                                         (in enumerateObjects order)
 *
 * in the class of UTOs that don't have any of
 *
 * enumerateObjects(forbObjsSize)
 *
 * such that forbObjsSelect[j] == true as induced subobjects
 *
 * The problem is described using flag expansion size expSize and flags specified by
 * utypes, flagSizes and flagSelect as follows:
 * the flags used are precisely the results of
 * enumerateFlags(flagSizes[i], utypes[i], morphism())[j]
 * for the indexes such that flagSelect[i][j] == true
 *
 * If scale is not zero the problem is rescaled such that every entry of the problem
 * is an integer multiple of scale (typical values for scale are zero or one)
 * returns: container of size (typesUsed+1) (where typesUsed is the number of types
 *          effectively used, i.e., that have at least one entry of flagSelect true)
 *          with the rescaling factors used on each block corresponding to the types
 *          followed by the rescaling factor globally used.
 * note: this rescaling is useful for reducing rounding errors when converting to
 *       floating points.
 * note: the objective value is not rescaled so that the dual problem also gets rescaled.
 *       This means that the solution value to the problem printed is the original value
 *       multiplied by the global rescaling factor of this function
 * note: to obtain the original matrix blocks entries, one would have to multiply
 *       each block by the respective scaling factor (of course it is much easier to
 *       multiply the densities of the product of flags of that block by the scaling
 *       factor instead)
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class UTO>
container<BigNum::bignum> fprintSDPproblem(FILE* file,
										   bool min,
										   unsigned long countObjsSize,
										   const container<coeff_type>& countObjsCoeffs,
										   unsigned long forbObjsSize,
										   const vbool& forbObjsSelect,
										   unsigned long expSize,
										   const container<UTO>& utypes,
										   const container<unsigned long>& flagSizes,
										   const container<vbool>& flagSelect,
										   BigNum::bignum scale,
										   const Monitoring::ProgressKeeper& progress) {
	const char* sgnStr(min? "" : "-");
	const container<UTO>& finalUTOs(enumerateObjects<UTO>(expSize));
	if (scale.negative())
		scale.invert();

	container<BigNum::bignum> rescalingFactors;

	std::vector<unsigned long> flagsUsed(utypes.size());
	unsigned long typesUsed = 0;
	/* Problem header */ {
		for (unsigned long i = 0; i < flagSelect.size(); ++i) {
			for (unsigned long j = 0; j < flagSelect[i].size(); ++j)
				if (flagSelect[i][j])
					++flagsUsed[i];
			if (flagsUsed[i])
				++typesUsed;
		}
		progress(0, typesUsed+1, "\nfprintSDPproblem:\n");
		
		fprintf(file,
				"%lu\n%lu\n",
				// Number of restriction matrices
				static_cast<unsigned long>(finalUTOs.size()),
				// Number of blocks (in every matrix)
				static_cast<unsigned long>(typesUsed + 2));
		// Block sizes (negative means diagonal block)
		for (unsigned long i = 0; i < flagSelect.size(); ++i) {
			if (flagsUsed[i]) {
				fprintf(file, "%lu ", flagsUsed[i]);
				unsigned long pickSize(flagSizes[i] - utypes[i].size());
				rescalingFactors.push_back
					(scale.zero()?
					 BigNum::bn1 :
					 BigNum::binomial(expSize - utypes[i].size(), pickSize)
					 * BigNum::binomial(expSize - flagSizes[i], pickSize)
					 * BigNum::decrescentfactorial(expSize, utypes[i].size()));
			}
		}
		fprintf(file, "-1 -%lu\n", static_cast<unsigned long>(finalUTOs.size()));
	}

	if (scale.zero())
		scale = BigNum::bn1;
	else
		scale = BigNum::lcm(scale, BigNum::binomial(expSize, countObjsSize));
	// Now scale has the global rescaling factor
	rescalingFactors.push_back(scale);

	vbool allowed(finalUTOs.size(), true);
	/* Allowed final UTOs */ {
		const container< container<coeff_type> >& expandCoeffs
			(expandObjectEnumerate<UTO>(forbObjsSize, expSize));

		for (unsigned long i = 0; i < expandCoeffs.size(); ++i) {
			for (unsigned long j = 0; j < expandCoeffs[i].size(); ++j) {
				if (forbObjsSelect[i]
					/** The below is redundant, so it is commented out
					&& !expandCoeffs[i][j].second.zero()
					**/
					) {
					allowed[expandCoeffs[i][j].first] = false;
					break;
				}
			}
		}
	}

	/* Densities on final UTOs (coefficients of restrictions) */ {
		const container< container<coeff_type> >& expandCoeffs
			(expandObjectEnumerate<UTO>(countObjsSize, expSize));
		std::vector<unsigned long> indexes(countObjsCoeffs.size());
		for (unsigned long j = 0; j < finalUTOs.size(); ++j) {
			if (!allowed[j]) {
				fprintf(file, "0 ");
				continue;
			}
			BigNum::frac coeff(0);
			for (unsigned long i = 0; i < countObjsCoeffs.size(); ++i) {
				unsigned long& ind(indexes[i]);
				const unsigned long countObjInd(countObjsCoeffs[i].first);
				while (ind < expandCoeffs[countObjInd].size()
					   && expandCoeffs[countObjInd][ind].first < j) {
					++ind;
				}
				if (ind < expandCoeffs[countObjInd].size()
					&& expandCoeffs[countObjInd][ind].first == j) {
					coeff += countObjsCoeffs[i].second
						* expandCoeffs[countObjInd][ind].second;
				}
			}
			fprintf(file,
					"%s%s ",
					sgnStr,
					(coeff * BigNum::frac(scale)).toStringPoint(fracPrecision).c_str());
		}
		fprintf(file, "\n");
	}

	/* Objective function */ {
		fprintf(file, "0 %lu 1 1 %s1.0\n", typesUsed+1, sgnStr);
	}

	// Restrictions on initial blocks
	unsigned long blockInd = 0;
	for (unsigned long b = 0; b < flagSelect.size(); ++b) {
		if (flagsUsed[b]) {
			++blockInd;

			const vbool& fSel(flagSelect[b]);
			morphism f;
			const container< container< container<coeff_type> > >& expProdEnumMat
				(expandFlagProductEnumerate(utypes[b],
											flagSizes[b],
											flagSizes[b],
											expSize,
											f));

			const container< Flag<UTO> >& expFlagEnum
				(enumerateFlags(expSize, utypes[b], f));

			unsigned long indi = 0;
			for (unsigned long i = 0; i < fSel.size(); ++i) {
				if (fSel[i]) {
					unsigned long indj = indi;
					++indi;
					for (unsigned long j = i; j < fSel.size(); ++j) {
						if (fSel[j]) {
							++indj;
							const container<coeff_type>& expProdEnum
								(expProdEnumMat[i][j]);
							for (unsigned long k = 0; k < expProdEnum.size(); ++k) {
								const unsigned long ind(expProdEnum[k].first);
								const BigNum::frac& value(expProdEnum[k].second
														  * expFlagEnum[ind]
														  .normFactor());
								// Entry value
								if (!value.zero()
									&& allowed[expFlagEnum[ind].objectIndex()]) {
									fprintf
										(file,
										 "%lu %lu %lu %lu %s\n",
										 expFlagEnum[ind].objectIndex()+1,
										 // Matrix number
										 blockInd, // Block number
										 indi, // Line number
										 indj, // Column number
										 (value
										  * BigNum::frac(scale
														 * rescalingFactors[blockInd-1]))
										 .toStringPoint(fracPrecision)
											.c_str());
								}
							}
						}
					}
				}
			}
			progress(blockInd, typesUsed+1, "\r");
		}
	}

	// Restrictions on last two blocks
	for (unsigned long i = 0; i < finalUTOs.size(); ++i) {
		if (allowed[i]) {
			// First block
			fprintf(file, "%lu %lu 1 1 %s1.0\n", i+1, typesUsed+1, sgnStr);
			// Second block
			fprintf(file, "%lu %lu %lu %lu 1.0\n", i+1, typesUsed+2, i+1, i+1);
		}
	}
	progress(typesUsed+1, typesUsed+1, "\r");
	return rescalingFactors;
}

/* printSDPproblem(bool min,
 *                 unsigned long countObjsSize,
 *				   const container<coeff_type>& countObjsCoeffs,
 *                 unsigned long forbObjsSize,
 *				   const vbool& forbObjsSelect,
 *				   unsigned long expSize,
 *				   const container<UTO>& utypes,
 *				   const container<unsigned long>& flagSizes,
 *				   const container<vbool>& flagSelect,
 *                 BigNum::bignum scale,
 *                 Monitoring::ProgressKeeper& progress):
 * restrictions: expSize >= countObjsSize;
 *               expSize >= forbObjsSize;
 *               utypes.size() == flagSizes.size() == flagSelect.size();
 *               for every 0 <= i < utypes.size(), we must have
 *                 flagSelect[i].size() ==
 *                   enumerateFlags(flagSizes[i], utypes[i], morphism()).size()
 *                 2*flagSizes[i] - utypes[i].size() <= expSize
 * prints on stdout the Semi-Definite-Programming Problem of minimizing/maximizing
 * (minimizing, iff min=true) the function of induced subUTOs of size countObjsSize
 * given by the sum over i of
 *
 * countObjsCoeffs[i].second * (countObjsCoeffs[i].first)-th UTO 
 *                                         (in enumerateObjects order)
 *
 * in the class of UTOs that don't have any of
 *
 * enumerateObjects(forbObjsSize)
 *
 * such that forbObjsSelect[j] == true as induced subobjects
 *
 * The problem is described using flag expansion size expSize and flags specified by
 * utypes, flagSizes and flagSelect as follows:
 * the flags used are precisely the results of
 * enumerateFlags(flagSizes[i], utypes[i], morphism())[j]
 * for the indexes such that flagSelect[i][j] == true
 *
 * If scale is not zero the problem is rescaled such that every entry of the problem
 * is an integer multiple of scale (typical values for scale are zero or one)
 * returns: container of size (typesUsed+1) (where typesUsed is the number of types
 *          effectively used, i.e., that have at least one entry of flagSelect true)
 *          with the rescaling factors used on each block corresponding to the types
 *          followed by the rescaling factor globally used.
 * note: this rescaling is useful for reducing rounding errors when converting to
 *       floating points.
 * note: the objective value is not rescaled so that the dual problem also gets rescaled.
 *       This means that the solution value to the problem printed is the original value
 *       multiplied by the global rescaling factor of this function
 * note: to obtain the original matrix blocks entries, one would have to multiply
 *       each block by the respective scaling factor
 * note: to obtain the original matrix blocks entries, one would have to multiply
 *       each block by the respective scaling factor (of course it is much easier to
 *       multiply the densities of the product of flags of that block by the scaling
 *       factor instead)
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class UTO>
container<BigNum::bignum> printSDPproblem(bool min,
										  unsigned long countObjsSize,
										  const container<coeff_type>& countObjsCoeffs,
										  unsigned long forbObjsSize,
										  const vbool& forbObjsSelect,
										  unsigned long expSize,
										  const container<UTO>& utypes,
										  const container<unsigned long>& flagSizes,
										  const container<vbool>& flagSelect,
										  BigNum::bignum scale,
										  const Monitoring::ProgressKeeper& progress) {
	return fprintSDPproblem(stdout,
							min,
							countObjsSize,
							countObjsCoeffs,
							forbObjsSize,
							expSize,
							utypes,
							flagSizes,
							flagSelect,
							scale,
							progress);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
