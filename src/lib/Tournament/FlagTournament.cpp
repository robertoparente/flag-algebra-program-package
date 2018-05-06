#include "FlagTournament.h"

#include <algorithm>

using namespace FlagAlgebra;
using namespace std;

/* default constructor
 * constructs a tournament of size 0
 */
Tournament::Tournament() : Digraph(){}

/* copy constructor
 */
Tournament::Tournament(const Tournament& other) : Digraph(other) {}

/* Tournament(const std::vector<vbool>& mat):
 * restrictions: mat must be the adjacency matrix of a tournament
 * constructs a tournament with adjacency matrix mat
 */
Tournament::Tournament(const std::vector<vbool>& mat) : Digraph(mat) {}

/* Tournament(unsigned long size, const bool **mat):
 * restrictions: mat[0..size-1][0..size-1] must be the adjacency matrix
 * of a tournament
 * constructs a tournament with adjacency matrix mat[0..size-1][0..size-1]
 */
Tournament::Tournament(unsigned long size, const bool **mat) : Digraph(size, mat) {}

/* (private) copy constructor with base class as RHS
 */
Tournament::Tournament(const Digraph& other) : Digraph(other) {}

/* copy operator
 */
Tournament& Tournament::operator=(const Tournament& other) {
	Digraph::operator=(other);
	return *this;
}

/* size() const:
 * returns: tournament's size
 */
unsigned long Tournament::size() const {
	return Digraph::size();
}

/* subobject(const vbool& sel) const:
 * returns: subgraph induced by the vertices whose index i satisfies sel[i] == true
 * note: such subgraph is guaranteed to be a tournament
 */
Tournament Tournament::subobject(const vbool& sel) const {
	return Tournament(Digraph::subobject(sel));
}


/* completeIsomorphism(const Tournament& other, morphism& f) const:
 * assumes f holds a partial function from the vertices of *this to the vertices
 * of other (indicating undefined values by f[v] = other.size()) and tries to
 * complete f to an isomorphism.
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 */
bool Tournament::completeIsomorphism(const Tournament& other, morphism& f) const {
	return Digraph::completeIsomorphism(other, f);
}


/* findIsomorphism(const Tournament& other, morphism& f) const:
 * tries to find an isomorphism between *this and other.
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 */
bool Tournament::findIsomorphism(const Tournament& other, morphism& f) const {
	f = morphism(s, s);
	return completeIsomorphism(other, f);
}

/* operator==(const Tournament& other) const:
 * returns: true, iff *this and other are isomorphic
 */
bool Tournament::operator==(const Tournament& other) const {
	morphism f;
	return findIsomorphism(other, f);
}

/* operator!=(const Tournament& other) const:
 * returns: false, iff *this and other are isomorphic
 */
bool Tournament::operator!=(const Tournament& other) const {
	return !operator==(other);
}

/* enumerate(unsigned long size):
 * returns: container with every tournament of size size occurring exactly once
 */
container<Tournament> Tournament::enumerate(unsigned long size) {
	vector<vbool> mat(size, vbool(size));
	for (unsigned long i = 1; i < size; ++i)
		for (unsigned long j = 0; j < i; ++j)
			mat[i][j] = true;
	vector<unsigned long> inDegrees(size), outDegrees(size);
	for (unsigned long i = 0; i < size; ++i) {
		inDegrees[i] = size-i-1;
		outDegrees[i] = i;
	}
	
	container<Tournament> ret(1, Tournament(mat));

	while (true) {
		unsigned long inci(0), incj(0);
		for ( ; inci < size; ++inci) {
			for (incj = inci+1; incj < size; ++incj) {
				if (!mat[inci][incj])
					break;
				mat[inci][incj] = false;
				mat[incj][inci] = true;
				--outDegrees[inci];
				--inDegrees[incj];
				++inDegrees[inci];
				++outDegrees[incj];
			}
			if (incj < size)
				break;
		}
		if (inci == size)
			break;
		++outDegrees[inci];
		++inDegrees[incj];
		--inDegrees[inci];
		--outDegrees[incj];

		mat[inci][incj] = true;
		mat[incj][inci] = false;

		unsigned long index;
		for (index = 1; index < size; ++index) {
			if (outDegrees[index-1] < outDegrees[index])
				break;
			else if (outDegrees[index-1] == outDegrees[index]
					 && inDegrees[index-1] < inDegrees[index])
				break;
		}
		if (index < size)
			continue;

		Tournament T(mat);
		for (index = 0; index < ret.size(); ++index)
			if (T == ret[index])
				break;
		if (index == ret.size())
			ret.push_back(T);
	}
	return ret;
}

/*
 * Format for I/O of tournaments:
 * (n
 * a1 b1
 * a2 b2
 * a3 b3
 * ...
 * )
 * Where n is the number of vertices and the digraph's arcs are exactly {ai, bi}
 * for all i.
 * The vertices of the digraph are labeled in 0..n-1
 */

/* readFromFile(FILE *f):
 * reads a tournament from file f and puts it on *this
 * returns: 1 if successful,
 *          -1 otherwise (*this holds corrupted data)
 */
int Tournament::readFromFile(FILE *f) {
	return Digraph::readFromFile(f);
}

/* writeOnFile(FILE *f) const:
 * writes *this on file f
 */
void Tournament::writeOnFile(FILE *f) const {
	return Digraph::writeOnFile(f);
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
