#ifndef LINEARSYSTEMTEMPLATES_CPP_INCLUDED
#define LINEARSYSTEMTEMPLATES_CPP_INCLUDED

/* solveLinearSystem(const Iterator& eqbegin,
 *                   const Iterator& eqend,
 *                   const Monitoring::ProgressKeeper& progress):
 * solves a linear system Ax = b on the variable x
 * the system is given in the range [eqbegin,eqend) in the block form [A b]
 * the number of variables (lines of x) is eqbegin->size() - 1;
 * restrictions: Iterator must be a mutable bidirectional iterator of a container
 *               of containers of number's,
 *               where number is a type of elements of a field,
 *               i.e., with operations +, -, *, /, etc. properly defined
 *               (e.g., std::list< std::vector<double> >::iterator)
 *               and number() must construct the 0 element (identity of +)
 *               [eqbegin,eqend) must be rectangular range
 *               (i.e., for all i, j iterators in the range, we have
 *               i->size() == j->size()).
 *               Furthermore, it is required that the second
 *               container (i.e., Iterator::value_type) has operator[] defined.
 *               (for instance, std::list< std::list<double> >::iterator does not satisfy
 *               such restriction).
 * returns: a vector with the indexes of the free variables of the solution.
 *          The function returns the solution by rewriting the system as
 *          Cx = d, with C being such that removing the columns corresponding to
 *          free variables we get an identity matrix with some extra 0 lines
 *          and the first non-zero entry of every line of C is not on a column
 *          corresponding to a free variable.
 * note: the function may permute the system lines.
 * throws: noSolution(), if the system has no solution.
 *           In this case, the system is rewritten to a system Ex = f in a way
 *           that there is a line of the form 0 = f[i], with f[i] != 0
 *         badSystem(), if the sys has zero lines or zero columns
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class Iterator>
std::vector<unsigned long>
solveLinearSystem(const Iterator& eqbegin,
				  const Iterator& eqend,
				  const Monitoring::ProgressKeeper& progress) {
	typedef typename Iterator::value_type::value_type number;
	std::vector<unsigned long> freeVars;

	unsigned long total, eqcnt;
	if ((total = 2 * (eqend - eqbegin)) == 0 || !eqbegin->size())
		throw badSystem();

	progress(eqcnt = 0, total, "\nsolveLinearSystem:\n");

	// The last column is the vector b
	const unsigned long variables(eqbegin->size()-1);

	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	unsigned long var = 0;
	Iterator eq(eqbegin);
	while (var < variables && eq < eqend) {
		while (true) {
			Iterator aux(eq);
			for ( ; aux != eqend; ++aux)
				if ((*aux)[var] != zero)
					break;
			if (aux != eqend) {
				if (aux != eq)
					swap(*aux, *eq);
				break;
			}
			freeVars.push_back(var);
			if (++var == variables)
				break;
		}
		if (var == variables)
			break;
		for (unsigned long j = variables+1; j > var; --j)
			(*eq)[j-1] /= (*eq)[var];

		Iterator it(eq);
	    for (++it; it != eqend; ++it) {
			for (unsigned long j = variables+1; j > var; --j)
				(*it)[j-1] -= (*it)[var] * (*eq)[j-1];
		}
		++eq;
		++var;
		progress(++eqcnt, total, "\r");
	}
	for (Iterator e(eq); e != eqend; ++e) {
		if (e->back() != zero)
			throw noSolution();
		progress(eqcnt += 2, total, "\r");
	}
	for ( ; var < variables; ++var)
		freeVars.push_back(var);

	unsigned long freeVarsInd = freeVars.size();
	while (var > 0 && freeVarsInd > 0 && var-1 == freeVars[freeVarsInd-1]) {
		--var;
		--freeVarsInd;
	}
	while (var > 0 && eq != eqbegin) {
		--eq;
		for (Iterator it(eqbegin); it != eq; ++it) {
			for (unsigned long j = variables+1; j > var-1; --j)
				(*it)[j-1] -= (*it)[var-1] * (*eq)[j-1];
		}
		--var;
		while (var > 0 && freeVarsInd > 0 && var-1 == freeVars[freeVarsInd-1]) {
			--var;
			--freeVarsInd;
		}
		progress(++eqcnt, total, "\r");
	}
	return freeVars;
}

/* applySolution(const Iterator& eqbegin,
 *               const Iterator& eqend,
 *               Container& vars,
 *               const Monitoring::ProgressKeeper& progress):
 * Assumes the range [eqbegin,eqend) has a solved linear system (as in the description of 
 * solveLinearSystem(const Iterator&, const Iterator&) and vars is a container
 * of variable values.
 * This function discards all non-free variable values and computes their values from
 * the values given for the free-variables.
 * restrictions: same as solveLinearSystem(const Iterator&, const Iterator&),
 *               but with the aditional restrictions that the system must be solved,
 *               the number of variables of the system (columns-1) must be equal
 *               to vars.size() and Container must be a container of numbers.
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class Iterator, class Container>
void applySolution(const Iterator& eqbegin,
				   const Iterator& eqend,
				   Container& vars,
				   const Monitoring::ProgressKeeper& progress) {
	typedef typename Iterator::value_type::value_type number;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	progress(0, vars.size(), "\napplySolution:\n");

	unsigned long index = 0;
	for (Iterator it(eqbegin); it != eqend; ++it) {
		for ( ; index < it->size()-1; ++index)
			if ((*it)[index] != zero)
				break;
		if (index == it->size()-1) {
			progress(vars.size(), vars.size(), "\r");
			return;
		}
		// In this case, (*it)[index] == 1
		vars[index] = it->back();
		for (unsigned long j = index+1; j < it->size()-1; ++j)
			vars[index] -= (*it)[j] * vars[j];
		progress(++index, vars.size(), "\r");
	}
}

/* solveSparseLinearSystem(const Iterator& eqbegin,
 *                         const Iterator& eqend,
 *                         unsigned long variables,
 *                         const Monitoring::ProgressKeeper& progress):
 * solves a linear system Ax = b on the variable x
 * the system is given in the range [eqbegin, eqend) in the block form [A b]
 * the number of variables (lines of x) is variables
 * the zero valued entries of A and b may and should (for efficiency) be ommited.
 * restrictions: Iterator must be a mutable bidirectional iterator of a container
 *               of a std::map<unsigned long, number>-like container,
 *               where number is a type of elements of a field,
 *               i.e., with operations +, -, *, /, etc. properly defined
 *               (e.g., std::list< std::map<unsigned long, double> >::iterator)
 *               and number() must construct the 0 element (identity of +)
 * note: by std::map-like container we mean a container that has the following
 *       functionalities working exactely as std::map:
 *       1) It is a container with a key_type, a mapped_type and it keeps stored
 *          value_type (which is std::pair<key_type, mapped_type>) in the order of first;
 *       2) The order can be determined by the functionals returned by
 *          key_comp() (to compare key_type) or value_comp() (to compare value_type);
 *       3) It has support for bidirectional iterators and reverse iterators;
 *       4) It has support for finding elements by their keys;
 *       5) It has support for insertion of elements with hint (an iterator to the
 *          will-be-next element) and removal of elements;
 * returns: a vector with the indexes of the free variables of the solution.
 *          The function returns the solution by rewriting the system as
 *          Cx = d, with C being such that removing the columns corresponding to
 *          free variables we get an identity matrix with some extra 0 lines
 *          and the first non-zero entry of every line of C is not on a column
 *          corresponding to a free variable.
 * note: the function may permute the system lines.
 * throws: noSolution(), if the system has no solution.
 *           In this case, the system is rewritten to a system Ex = f in a way
 *           that there is a line of the form 0 = f[i], with f[i] != 0
 *         badSystem(), if the sys has zero lines or zero columns
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class Iterator>
std::vector<unsigned long>
solveSparseLinearSystem(const Iterator& eqbegin,
						const Iterator& eqend,
						unsigned long variables,
						const Monitoring::ProgressKeeper& progress) {
	typedef typename Iterator::value_type map_type;
	typedef typename map_type::mapped_type number;
	typedef typename map_type::iterator map_iter;
	typedef typename map_type::reverse_iterator map_rev_iter;

	unsigned long total, eqcnt;
	if ((total = 2*(eqend - eqbegin)) == 0 || !variables)
		throw badSystem();	

	progress(eqcnt = 0, total, "\nsolveSparseLinearSystem:\n");

	typename map_type::key_compare kCmp(eqbegin->key_comp());
	std::vector<unsigned long> freeVars;

	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();

	unsigned long var = 0;
	Iterator eq(eqbegin);
	while (var < variables && eq < eqend) {
		while (true) {
			Iterator aux(eq);
			for ( ; aux != eqend; ++aux) {
				if (aux->size() && !kCmp(var, aux->begin()->first)) {
					// var >= aux->begin()->first
					if (aux->begin()->second == zero)
						aux->erase(aux->begin());
					else
						break;
				}
			}
			if (aux != eqend) {
				if (aux != eq)
					swap(*aux, *eq);
				break;
			}
			freeVars.push_back(var);
			if (++var == variables)
				break;
		}
		if (var == variables)
			break;

		for (map_rev_iter mrit(eq->rbegin()); mrit != eq->rend(); ++mrit)
			mrit->second /= eq->begin()->second;

		Iterator it(eq);
	    for (++it; it != eqend; ++it) {
			if (it->size() && !kCmp(var, it->begin()->first)) {
				// var >= it->begin()->first
				it->begin()->second = -it->begin()->second;
				map_iter mit_it(it->begin()), mit_eq(eq->begin());
				++mit_it;
				++mit_eq;
				while(true){
					if (kCmp(mit_it->first, mit_eq->first)) {
						// mit_it->first < mit_eq->first
						if (++mit_it == it->end())
							break;
					}
					else if (kCmp(mit_eq->first, mit_it->first)) {
						// mit_eq->first < mit_it->first
						it->insert(mit_it, *mit_eq)->second *= it->begin()->second;
						if (++mit_eq == eq->end())
							break;
					}
					else {
						// mit_it->first == mit_eq->first
						mit_it->second += it->begin()->second * mit_eq->second;
						if (mit_it->second == zero)
							it->erase(mit_it++);
						else
							++mit_it;
						++mit_eq;
						if (mit_it == it->end() || mit_eq == eq->end())
							break;
					}
				}
				for ( ; mit_eq != eq->end(); ++mit_eq) {
					it->insert(it->end(), *mit_eq)->second *= it->begin()->second;
				}
				it->erase(it->begin());
			}
		}
		++eq;
		++var;
		progress(++eqcnt, total, "\r");
	}
	for (Iterator e(eq); e != eqend; ++e) {
		if (e->size() && e->rbegin()->second != zero)
			throw noSolution();
		progress(eqcnt += 2, total, "\r");
	}
	for ( ; var < variables; ++var)
		freeVars.push_back(var);
	while (eq != eqbegin) {
		--eq;
		for (Iterator it(eqbegin); it != eq; ++it) {
			const map_iter mit_var_it(it->find(eq->begin()->first));
			if (mit_var_it != it->end()) {
				mit_var_it->second = -mit_var_it->second;
				map_iter mit_it(mit_var_it), mit_eq(eq->begin());
				++mit_it;
				++mit_eq;
				while(true){
					if (kCmp(mit_it->first, mit_eq->first)) {
						// mit_it->first < mit_eq->first
						if (++mit_it == it->end())
							break;
					}
					else if (kCmp(mit_eq->first, mit_it->first)) {
						// mit_eq->first < mit_it->first
						it->insert(mit_it, *mit_eq)->second *= mit_var_it->second;
						if (++mit_eq == eq->end())
							break;
					}
					else {
						// mit_it->first == mit_eq->first
						mit_it->second += mit_var_it->second * mit_eq->second;
						if (mit_it->second == zero)
							it->erase(mit_it++);
						else
							++mit_it;
						++mit_eq;
						if (mit_it == it->end() || mit_eq == eq->end())
							break;
					}
				}
				for ( ; mit_eq != eq->end(); ++mit_eq) {
					it->insert(it->end(), *mit_eq)->second *= mit_var_it->second;
				}
				it->erase(mit_var_it);
			}
		}
		progress(++eqcnt, total, "\r");
	}
	return freeVars;
}

/* applySparseSolution(const Iterator& eqbegin,
 *                     const Iterator& eqend,
 *                     Container& vars,
 *                     const Monitoring::ProgressKeeper& progress):
 * Assumes the range [eqbegin,eqend) has a solved linear system (as in the description of 
 * solveSparseLinearSystem(const Iterator&, const Iterator&, unsigned long)
 * and vars is a container of variable values.
 * This function discards all non-free variable values and computes their values from
 * the values given for the free-variables.
 * restrictions: same as solveLinearSystem(const Iterator&, const Iterator&),
 *               but with the aditional restrictions that the system must be solved,
 *               the number of variables of the system (the third parameter of
 *               solveSparseLinearSystem(const Iterator&, const Iterator&, unsigned long))
 *               must be equal to vars.size() and Container must be a 
 *               container of numbers.
 *               Furthermore, the zero value entries in A and b are required to
 *               be ommited.
 * note: progress is used to show progress in standard ostream (cerr)
 */
template<class Iterator, class Container>
void applySparseSolution(const Iterator& eqbegin,
						 const Iterator& eqend,
						 Container& vars,
						 const Monitoring::ProgressKeeper& progress) {
	typedef typename Iterator::value_type map_type;
	typedef typename map_type::mapped_type number;
	typedef typename map_type::iterator map_iter;
	/* We assume that the default constructor or the empty value constructor
	 * constructs a number of value zero
	 */
	const number zero = number();
	unsigned long index;
	progress(index = 0, vars.size(), "\napplySparseSolution:\n");

	for (Iterator it(eqbegin); it != eqend && it->size(); ++it) {
		map_iter mit_var(it->begin());
		for ( ; mit_var != it->end() && mit_var->second == zero; ++mit_var) {
		}
		if (mit_var == it->end() || mit_var->first == vars.size())
			break;

		map_iter mit_end(it->end());
		if (it->rbegin()->first == vars.size()) {
			vars[mit_var->first] = it->rbegin()->second;
			--mit_end;
		}
		else {
			vars[mit_var->first] = zero;
		}

		map_iter mit(mit_var);
		for (++mit; mit != mit_end; ++mit)
			vars[mit_var->first] -= mit->second * vars[mit->first];
		progress(++index, vars.size(), "\r");
	}
	progress(vars.size(), vars.size(), "\r");
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
