#ifndef SELECTOR_H_INCLUDED
#define SELECTOR_H_INCLUDED

#include <iterator>
#include <vector>

namespace Selector {
	// Forward declaration
    template <class Iterator>
    class seliterator;

	typedef std::vector<bool> vbool;
	typedef vbool::const_iterator bool_it;

	/* class selector<Iterator> implements an iterator filter that produces a
	 * class with the functionality of container of references to the elements
	 * pointed by the base iterators (the produced pseudo-container has
	 * random access iterators
	 * note: Iterator must be a forward iterator
	 * note 2: Using a const selector of a mutable iterator doesn't give
	 *         behaviour of a const container. To get that effect, one needs to
	 *         use selector<const_iterator>.
	 */
    template <class Iterator>
    class selector {
        public:
        typedef Iterator base_iterator;
        typedef typename Iterator::value_type value_type;
        typedef typename Iterator::reference reference;
        typedef seliterator<Iterator> iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef typename iterator::difference_type difference_type;
        typedef typename std::vector<base_iterator>::size_type size_type;

        private:
        std::vector<base_iterator> v;

        void select(base_iterator, const base_iterator&, bool_it, const bool_it&, bool = false);

        public:
        selector(const base_iterator&, const base_iterator&, const vbool&, bool = false);
        selector(const base_iterator&, const base_iterator&, const bool_it&, const bool_it&, bool = false);

        selector(const base_iterator&, const base_iterator&, bool (*)(value_type));


        iterator begin() const;
        iterator end() const;
        reverse_iterator rbegin() const;
        reverse_iterator rend() const;

        size_type size() const;
        bool empty() const;
        reference operator[](difference_type) const;
        reference at(difference_type) const;
        reference front() const;
        reference back() const;
    };

	// class seliterator<Iterator> is the underlying iterator of selector<Iterator>
    template <class Iterator>
    class seliterator : private std::vector<Iterator>::const_iterator {
        private:
        typedef typename std::vector<Iterator>::const_iterator vit;

        public:
        typedef Iterator base_iterator;
        typedef typename Iterator::value_type value_type;
        typedef typename Iterator::pointer pointer;
        typedef typename Iterator::reference reference;
        typedef typename std::vector<Iterator>::iterator::difference_type difference_type;
        typedef std::random_access_iterator_tag iterator_category;

        private:
        friend class selector<Iterator>;
        seliterator(const vit&);

        public:
        seliterator();
        seliterator<Iterator>& operator++();
        seliterator<Iterator>& operator--();
        seliterator<Iterator> operator++(int);
        seliterator<Iterator> operator--(int);
        seliterator<Iterator>& operator+=(difference_type);
        seliterator<Iterator>& operator-=(difference_type);
        seliterator<Iterator> operator+(difference_type) const;
        seliterator<Iterator> operator-(difference_type) const;
        difference_type operator-(const seliterator<Iterator>&) const;
        reference operator*() const;
		base_iterator operator->() const;
        reference operator[](difference_type);
        bool operator==(const seliterator<Iterator>&) const;
        bool operator!=(const seliterator<Iterator>&) const;
        bool operator<(const seliterator<Iterator>&) const;
        bool operator<=(const seliterator<Iterator>&) const;
        bool operator>(const seliterator<Iterator>&) const;
        bool operator>=(const seliterator<Iterator>&) const;
    };

    template <class Iterator>
    seliterator<Iterator> operator+(typename seliterator<Iterator>::difference_type, const seliterator<Iterator>&);

	// This include is intentionally inside namespace Selector
	// and the file has the namespace templates' definitions
#include "selectorTemplates.cpp"
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
