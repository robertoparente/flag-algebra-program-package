#ifndef SELECTORTEMPLATES_CPP_INCLUDED
#define SELECTORTEMPLATES_CPP_INCLUDED

//class selector functions
/* select(base_iterator begin,
 *        const base_iterator& end,
 *        bool_it boolb,
 *        const bool_it& boole,
 *        bool reverse):
 * turns *this into a selector that selects from [begin,end) the elements such that
 * the corresponding iterator it in [boolb,boole) satisfies *it == !reverse
 */
template <class Iterator>
void selector<Iterator>::select(base_iterator begin,
								const base_iterator& end,
								bool_it boolb,
								const bool_it& boole,
								bool reverse) {
    while (begin != end && boolb != boole) {
        if (*boolb xor reverse)
            v.push_back(begin);
        ++begin;
        ++boolb;
    }
}

/* selector(const base_iterator& begin,
 *          const base_iterator& end,
 *          const vbool& sel,
 *          bool reverse):
 * constructs a selector that selects from [begin,end) the elements such that
 * the corresponding iterator it in [sel.begin(), sel.end()) satisfies *it == !reverse
 */
template <class Iterator>
selector<Iterator>::selector(const base_iterator& begin,
							 const base_iterator& end,
							 const vbool& sel,
							 bool reverse) : v() {
    select(begin, end, sel.begin(), sel.end(), reverse);
}

/* selector(const base_iterator& begin,
 *          const base_iterator& end,
 *          const bool_it& boolb,
 *          const bool_it& boole,
 *          bool reverse):
 * constructs a selector that selects from [begin,end) the elements such that
 * the corresponding iterator it in [boolb(), boole()) satisfies *it == !reverse
 */
template <class Iterator>
selector<Iterator>::selector(const base_iterator& begin,
							 const base_iterator& end,
							 const bool_it& boolb,
							 const bool_it& boole,
							 bool reverse) : v() {
    select(begin, end, boolb, boole, reverse);
}

/* selector(const base_iterator& begin,
 *          const base_iterator& end,
 *          bool (*f)(value_type)):
 * constructs a selector that selects from [begin,end) the elements e such that
 * (*f)(e) == true
 */
template <class Iterator>
selector<Iterator>::selector(const base_iterator& begin,
							 const base_iterator& end,
							 bool (*f)(value_type)) {
    for (base_iterator b(begin); b != end; ++b)
        if ((*f)(*b))
            v.push_back(b);
}

/* begin() const:
 * returns: a seliterator to the beginning of the selected range
 */
template <class Iterator>
typename selector<Iterator>::iterator selector<Iterator>::begin() const {
    return iterator(v.begin());
}

/* end() const:
 * returns: a seliterator to the end of the selected range
 */
template <class Iterator>
typename selector<Iterator>::iterator selector<Iterator>::end() const {
    return iterator(v.end());
}

/* rbegin() const:
 * returns: a reverse seliterator to the reverse beginning of the selected range
 */
template <class Iterator>
typename selector<Iterator>::reverse_iterator selector<Iterator>::rbegin() const {
    return reverse_iterator(end());
}

/* rend() const:
 * returns: a reverse seliterator to the reverse end of the selected range
 */
template <class Iterator>
typename selector<Iterator>::reverse_iterator selector<Iterator>::rend() const{
    return reverse_iterator(begin());
}

/* size() const:
 * returns: the selector's size() (the number of base_iterator's selected)
 */
template <class Iterator>
typename selector<Iterator>::size_type selector<Iterator>::size() const {
    return v.size();
}

/* empty() const:
 * returns: true, iff the selector is empty (i.e., size() == 0)
 */
template <class Iterator>
bool selector<Iterator>::empty() const {
    return v.empty();
}

/* operator[](difference_type n) const:
 * restrictions: n < size()
 * returns: a reference to the n-th element selected
 */
template <class Iterator>
typename selector<Iterator>::reference
selector<Iterator>::operator[](difference_type n) const {
    return *v[n];
}

/* at(difference_type n) const:
 * returns: a reference to the n-th element selected
 * throws: std::out_of_range, if n >= size()
 */
template <class Iterator>
typename selector<Iterator>::reference selector<Iterator>::at(difference_type n) const {
    return *v.at(n);
}

/* front() const:
 * restrictions: !empty()
 * returns: a reference to the first element selected
 */
template <class Iterator>
typename selector<Iterator>::reference selector<Iterator>::front() const {
    return *v.front();
}

/* back() const:
 * restrictions: !empty()
 * returns: a reference to the last element selected
 */
template <class Iterator>
typename selector<Iterator>::reference selector<Iterator>::back() const {
    return *v.back();
}

//class seliterator functions
/* copy constructor
 */
template <class Iterator>
seliterator<Iterator>::seliterator(const vit& it) : vit(it) {}

/* default constructor
 */
template <class Iterator>
seliterator<Iterator>::seliterator() : vit() {}

/* operator++():
 * increments *this
 * returns: *this (after operation)
 */
template <class Iterator>
seliterator<Iterator>& seliterator<Iterator>::operator++() {
    vit::operator++();
    return *this;
}

/* operator--():
 * decrements *this
 * returns: *this (after operation)
 */
template <class Iterator>
seliterator<Iterator>& seliterator<Iterator>::operator--() {
    vit::operator--();
    return *this;
}

/* operator++(int):
 * increments *this
 * returns: the value of *this before the operation
 */
template <class Iterator>
seliterator<Iterator> seliterator<Iterator>::operator++(int) {
    seliterator<Iterator> aux(*this);
    operator++();
    return aux;
}

/* operator--(int):
 * decrements *this
 * returns: the value of *this before the operation
 */
template <class Iterator>
seliterator<Iterator> seliterator<Iterator>::operator--(int) {
    seliterator<Iterator> aux(*this);
    operator--();
    return aux;
}

/* operator+=(seliterator<Iterator>::difference_type n):
 * increments *this by n units
 * returns: *this (after the operation)
 */
template <class Iterator>
seliterator<Iterator>& seliterator<Iterator>::operator+=(seliterator<Iterator>::difference_type n) {
    vit::operator+=(n);
    return *this;
}

/* operator-=(seliterator<Iterator>::difference_type n):
 * decrements *this by n units
 * returns: *this (after the operation)
 */
template <class Iterator>
seliterator<Iterator>& seliterator<Iterator>::operator-=(seliterator<Iterator>::difference_type n) {
    vit::operator-=(n);
    return *this;
}

/* operator+(seliterator<Iterator>::difference_type n) const:
 * returns: an iterator to n elements after *this
 */
template <class Iterator>
seliterator<Iterator> seliterator<Iterator>::operator+(seliterator<Iterator>::difference_type n) const {
    seliterator<Iterator> aux(*this);
    return aux += n;
}

/* operator-(seliterator<Iterator>::difference_type n) const:
 * returns: an iterator to n elements before *this
 */
template <class Iterator>
seliterator<Iterator> seliterator<Iterator>::operator-(seliterator<Iterator>::difference_type n) const {
    seliterator<Iterator> aux(*this);
    return aux -= n;
}

/* operator-(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: distance between *this and a
 */
template <class Iterator>
typename seliterator<Iterator>::difference_type seliterator<Iterator>::operator-(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) - static_cast<vit>(a);
}

/* operator[](seliterator<Iterator>::difference_type n):
 * restrictions: *this + n is a valid seliterator
 * returns: reference to *(*this + n)
 */
template <class Iterator>
typename seliterator<Iterator>::reference seliterator<Iterator>::operator[](seliterator<Iterator>::difference_type n) {
    return *vit::operator[](n);
}

/* operator*() const:
 * restrictions: *this is a valid seliterator
 * returns: reference to the element `pointed' by *this
 */
template <class Iterator>
typename seliterator<Iterator>::reference seliterator<Iterator>::operator*() const {
    return *vit::operator*();
}

/* operator->() const:
 * restrictions: *this is a valid seliterator
 * returns: base_iterator `pointed' by *this
 */
template <class Iterator>
typename seliterator<Iterator>::base_iterator seliterator<Iterator>::operator->() const {
    return vit::operator*();
}

/* operator==(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: true, iff *this and a `point' to the same element
 */
template <class Iterator>
bool seliterator<Iterator>::operator==(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) == (static_cast<vit>(a));
}

/* operator!=(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: false, iff *this and a `point' to the same element
 */
template <class Iterator>
bool seliterator<Iterator>::operator!=(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) != (static_cast<vit>(a));
}

/* operator<(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: true, iff *this `points' to an element that comes strictly before
 *          the element `pointed' by a in the underlying range
 */
template <class Iterator>
bool seliterator<Iterator>::operator<(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) < (static_cast<vit>(a));
}

/* operator<=(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: true, iff *this `points' to an element that comes before
 *          the element `pointed' by a in the underlying range
 */
template <class Iterator>
bool seliterator<Iterator>::operator<=(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) <= (static_cast<vit>(a));
}

/* operator>(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: true, iff *this `points' to an element that comes strictly after
 *          the element `pointed' by a in the underlying range
 */
template <class Iterator>
bool seliterator<Iterator>::operator>(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) > (static_cast<vit>(a));
}

/* operator>(const seliterator<Iterator>& a) const:
 * restrictions: a and *this are seliterators of the same selector
 * returns: true, iff *this `points' to an element that comes after
 *          the element `pointed' by a in the underlying range
 */
template <class Iterator>
bool seliterator<Iterator>::operator>=(const seliterator<Iterator>& a) const {
    return static_cast<vit>(*this) >= (static_cast<vit>(a));
}

/* operator+(typename seliterator<Iterator>::difference_type n,
 *           const seliterator<Iterator>& it):
 * returns: an iterator to n elements after *this
 */
template <class Iterator>
seliterator<Iterator> operator+(typename seliterator<Iterator>::difference_type n,
								const seliterator<Iterator>& it) {
	return it + n;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
