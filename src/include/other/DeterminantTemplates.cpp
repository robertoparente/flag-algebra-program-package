#ifndef DETERMINANTTEMPLATES_CPP_INCLUDED
#define DETERMINANTTEMPLATES_CPP_INCLUDED

template<class Container>
typename Container::value_type::value_type determinant(const Container& M) {
	typedef typename Container::value_type::value_type number;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	Container mat(M);
	bool sgn = false;
	for (unsigned long i = 0; i < mat.size(); ++i) {
		unsigned long ind = i;
		for ( ; ind < mat.size(); ++ind)
			if (mat[ind][i] != zero)
				break;
		if (ind == mat.size())
			return zero;
		if (ind != i) {
			swap(mat[i], mat[ind]);
			sgn = !sgn;
		}
		for (unsigned long j = i+1; j < mat.size(); ++j) {
			mat[j][i] /= mat[i][i];
			for (unsigned long k = i+1; k < mat[j].size(); ++k)
				mat[j][k] -= mat[i][k] * mat[j][i];
		}
	}
	for (unsigned long i = 1; i < mat.size(); ++i)
		mat[0][0] *= mat[i][i];
	return sgn? -mat[0][0] : mat[0][0];
}

template<class Container>
typename Container::value_type::value_type principalMinorDeterminant(const Container& M,
																	 const vbool& s) {
	typedef typename Container::value_type::value_type number;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	Container mat;
	for (unsigned long i = 0; i < M.size(); ++i) {
		if (s[i]) {
			mat.resize(mat.size()+1);
			for (unsigned long j = 0; j < M.size(); ++j)
				if (s[j])
					mat.back().push_back(M[i][j]);
		}
	}

	bool sgn = false;
	for (unsigned long i = 0; i < mat.size(); ++i) {
		unsigned long ind = i;
		for ( ; ind < mat.size(); ++ind)
			if (mat[ind][i] != zero)
				break;
		if (ind == mat.size())
			return zero;
		if (ind != i) {
			swap(mat[i], mat[ind]);
			sgn = !sgn;
		}
		for (unsigned long j = i+1; j < mat.size(); ++j) {
			mat[j][i] /= mat[i][i];
			for (unsigned long k = i+1; k < mat[j].size(); ++k)
				mat[j][k] -= mat[i][k] * mat[j][i];
		}
	}
	for (unsigned long i = 1; i < mat.size(); ++i)
		mat[0][0] *= mat[i][i];
	return sgn? -mat[0][0] : mat[0][0];
}

template<class Container>
bool isSymmetric(const Container& M) {
	for (unsigned long i = 0; i < M.size(); ++i)
		for (unsigned long j = 0; j < M[i].size(); ++j)
			if (M[i][j] != M[j][i])
				return false;
	return true;
}

template<class Container, class Signal>
bool isPositiveSemidefinite(const Container& M, Signal signal) {
	typedef typename Container::value_type::value_type number;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	Container mat(M);
	for (unsigned long i = 0; i < mat.size(); ++i) {
		int sgn = signal(mat[i][i]);
		if (sgn < 0) {
			return false;
		}
		else if (sgn == 0) {
			for (unsigned long j = i+1; j < mat.size(); ++j)
				if (mat[j][i] != zero)
					return false;
		}
		else {
			for (unsigned long j = i+1; j < mat.size(); ++j) {
				mat[j][i] /= mat[i][i];
				for (unsigned long k = i+1; k < mat[j].size(); ++k)
					mat[j][k] -= mat[i][k] * mat[j][i];
			}
		}
	}
	return true;
}

template<class Container, class Signal>
bool isPositiveDefinite(const Container& M, Signal signal) {
	Container mat(M);
	for (unsigned long i = 0; i < mat.size(); ++i) {
		int sgn = signal(mat[i][i]);
		if (sgn > 0) {
			for (unsigned long j = i+1; j < mat.size(); ++j) {
				mat[j][i] /= mat[i][i];
				for (unsigned long k = i+1; k < mat[j].size(); ++k)
					mat[j][k] -= mat[i][k] * mat[j][i];
			}
		}
		else {
			return false;
		}
	}
	return true;
}

template<class Container, class Signal>
bool isNegativeSemidefinite(const Container& M, Signal signal) {
	typedef typename Container::value_type::value_type number;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	Container mat(M);
	for (unsigned long i = 0; i < mat.size(); ++i) {
		int sgn = signal(mat[i][i]);
		if (sgn > 0) {
			return false;
		}
		else if (sgn == 0) {
			for (unsigned long j = i+1; j < mat.size(); ++j)
				if (mat[j][i] != zero)
					return false;
		}
		else {
			for (unsigned long j = i+1; j < mat.size(); ++j) {
				mat[j][i] /= mat[i][i];
				for (unsigned long k = i+1; k < mat[j].size(); ++k)
					mat[j][k] -= mat[i][k] * mat[j][i];
			}
		}
	}
	return true;
}

template<class Container, class Signal>
bool isNegativeDefinite(const Container& M, Signal signal) {
	Container mat(M);
	for (unsigned long i = 0; i < mat.size(); ++i) {
		int sgn = signal(mat[i][i]);
		if (sgn < 0) {
			for (unsigned long j = i+1; j < mat.size(); ++j) {
				mat[j][i] /= mat[i][i];
				for (unsigned long k = i+1; k < mat[j].size(); ++k)
					mat[j][k] -= mat[i][k] * mat[j][i];
			}
		}
		else {
			return false;
		}
	}
	return true;
}

template<class Container, class Signal>
bool isIndefinite(const Container& M, Signal signal) {
	Container mat(M);
	bool pos = false, neg = false;
	for (unsigned long i = 0; i < mat.size(); ++i) {
		int sgn = signal(mat[i][i]);
		if (sgn > 0)
			pos = true;
		else if (sgn == 0)
			return true;
		else
			neg = true;

		if (pos && neg)
			return true;
		for (unsigned long j = i+1; j < mat.size(); ++j) {
			mat[j][i] /= mat[i][i];
			for (unsigned long k = i+1; k < mat[j].size(); ++k)
				mat[j][k] -= mat[i][k] * mat[j][i];
		}
	}
	return false;
}


template<class number>
int defaultSignal(const number& n) {
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();
	return (zero < n) - (n < zero);
}

template<class Container>
bool isPositiveSemidefinite(const Container& M) {
	typedef typename Container::value_type::value_type number;
	return isPositiveSemidefinite(M, defaultSignal<number>);
}

template<class Container>
bool isPositiveDefinite(const Container& M) {
	typedef typename Container::value_type::value_type number;
	return isPositiveDefinite(M, defaultSignal<number>);
}

template<class Container>
bool isNegativeSemidefinite(const Container& M) {
	typedef typename Container::value_type::value_type number;
	return isNegativeSemidefinite(M, defaultSignal<number>);
}

template<class Container>
bool isNegativeDefinite(const Container& M) {
	typedef typename Container::value_type::value_type number;
	return isNegativeDefinite(M, defaultSignal<number>);
}

template<class Container>
bool isIndefinite(const Container& M) {
	typedef typename Container::value_type::value_type number;
	return isIndefinite(M, defaultSignal<number>);
}


/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
