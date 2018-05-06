#ifndef INJECTIVETEMPLATES_CPP_INCLUDED
#define INJECTIVETEMPLATES_CPP_INCLUDED

/* injective(unsigned long s, unsigned long ts):
 * restrictions: s <= ts
 * constructs the first injective function from [s] to [ts]
 */
template<class Cont>
injective<Cont>::injective(unsigned long s, unsigned long ts) : Cont(s), t(ts) {
	for (unsigned long i = 0; i < s; ++i)
		(*this)[i] = i;
}

/* injective(unsigned long s, unsigned long ts, safe sf):
 * constructs the first injective function from [s] to [ts]
 * throws: inexistent(), if s > ts
 */
template<class Cont>
injective<Cont>::injective(unsigned long s, unsigned long ts, safe sf) : Cont(s), t(ts) {
	if (s > ts) throw inexistent();
	for (unsigned long i = 0; i < s; ++i)
		(*this)[i] = i;
}

/* injective(const Cont& v, unsigned long ts):
 * restrictions: v holds an injective function to [ts]
 *               (i.e., there does not exist i != j s.t. v[i] == v[j] and
 *               for all i, v[i] < ts)
 * constructs the injective funcion from v.size() to ts given by v
 */
template<class Cont>
injective<Cont>::injective(const Cont& v, unsigned long ts) : Cont(v), t(ts) {}

/* injective(const Cont& v, unsigned long ts, safe sf):
 * constructs the injective funcion from v.size() to ts given by v
 * throws: inexistent(), if v does not hold an injective function to [ts]
 */
template<class Cont>
injective<Cont>::injective(const Cont& v, unsigned long ts, safe sf) : Cont(v), t(ts) {
	std::vector<bool> vb(v.size());
	for (unsigned long i = 0; i < v.size(); ++i) {
		if (t <= v[i] || vb[v[i]])
			throw inexistent();
		vb[v[i]] = true;
	}
}

/* targetsize() const:
 * returns: the size of the codomain of *this
 */
template<class Cont>
unsigned long injective<Cont>::targetsize() const {
	return t;
}

/* next():
 * transforms *this in the (lexicografically) next injective
 * function from size() to targetsize()
 * returns: false, iff *this was the last injective function from
 *          size() to targetsize() (in this case *this will hold the
 *          first injective function from size() to targetsize()
 */
template<class Cont>
bool injective<Cont>::next() {
	if (std::next_permutation(this->begin(), this->end()))
		return true;

	unsigned long i, k = 0;

	for (i = 0; i < this->size(); ++i) {
		if ((*this)[this->size()-i-1] < t - i - 1) {
			k = ++(*this)[this->size()-i-1] + 1;
			break;
		}
	}
	for (unsigned long j = this->size() - i; j < this->size(); ++j, ++k)
		(*this)[j] = k;

	return i < this->size();
}

/* prev():
 * transforms *this in the (lexicografically) previous injective
 * function from size() to targetsize()
 * returns: false, iff *this was the first injective function from
 *          size() to targetsize() (in this case *this will hold the
 *          last injective function from size() to targetsize()
 */
template<class Cont>
bool injective<Cont>::prev() {
	if (std::prev_permutation(this->begin(), this->end()))
		return true;

	unsigned long i;

	for (i = 0; i < this->size()-1; ++i) {
		if ((*this)[this->size()-i-1] > (*this)[this->size()-i-2] + 1) {
			--(*this)[this->size()-i-1];
			break;
		}
	}

	for (unsigned long j = 0; j < i; ++j)
		(*this)[this->size()-j-1] = t - j - 1;

	if (i == this->size()-1) {
		if ((*this)[0] == 0) {
			(*this)[0] = t - this->size();
			return false;
		}
		--(*this)[0];
	}
	return true;
}

#endif
