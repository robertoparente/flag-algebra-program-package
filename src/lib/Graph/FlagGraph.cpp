#include "FlagGraph.h"

#include <algorithm>

using namespace FlagAlgebra;
using namespace std;

/* default constructor
 * constructs a graph of size 0
 */
Graph::Graph() : s(0), m() {}

/* copy constructor
 */
Graph::Graph(const Graph& other) : s(other.s), m(other.m) {}

/* Graph(const vector<vbool>& mat):
 * restriction: mat must be the adjacency matrix of a graph
 * (i.e., must be symmetric and with diagonal 0)
 * constructs the graph with adjacency matrix mat
 */
Graph::Graph(const vector<vbool>& mat) : s(mat.size()), m(mat.size()*mat.size()) {
	for (unsigned long i = 0; i < s; ++i)
		for (unsigned long j = 0; j < s; ++j)
			m[s*i+j] = mat[i][j];
}

/* Graph(unsigned long size, const bool **mat):
 * restrictions: mat[0..size-1][0..size-1] must be the adjacency matrix
 * of a graph (i.e., must be symmetric and with diagonal 0)
 * constructs the graph with adjacency matrix mat[0..size-1][0..size-1]
 */
Graph::Graph(unsigned long size, const bool **mat) : s(size), m(size*size) {
	for (unsigned long i = 0; i < size; ++i)
		for (unsigned long j = 0; j < size; ++j)
			m[s*i+j] = mat[i][j];
}

/* copy operator
 */
Graph& Graph::operator=(const Graph& other) {
	s = other.s;
	m = other.m;
	return *this;
}

/* size() const:
 * returns: graph's size
 */
unsigned long Graph::size() const {
	return s;
}

/* subobject(const vbool& sel) const:
 * returns: subgraph induced by the vertices whose index i satisfies sel[i] == true
 */
Graph Graph::subobject(const vbool& sel) const {

	vector<unsigned long> v;

	for (unsigned long i = 0; i < s; ++i)
		if (sel[i])
			v.push_back(i);

	Graph ret;
	ret.s = v.size();
	ret.m.resize(ret.s*ret.s);

	for (unsigned long i = 0; i < v.size(); ++i)
		for (unsigned long j = 0; j < v.size(); ++j)
			ret.m[ret.s*i+j] = m[s*v[i]+v[j]];
	return ret;
}

// This is an auxiliary type to be used with completeIsomorphism
typedef pair<unsigned long, unsigned long> degreeIndex;
// This is an auxiliary class to be used with sort
class compFirstDegreeIndex {
public:
	compFirstDegreeIndex() {}
	bool operator()(const degreeIndex& a, const degreeIndex& b) {
		return a.first < b.first;
	}
};


/* completeIsomorphism(const Graph& other, morphism& f) const:
 * assumes f holds a partial function from the vertices of *this to the vertices
 * of other (indicating undefined values by f[v] = other.size()) and tries to
 * complete f to an isomorphism.
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 */
bool Graph::completeIsomorphism(const Graph& other, morphism& f) const {
	if (size() != other.size())
		return false;
	vector<unsigned long> deg(size()), otherDeg(size());

	vector<degreeIndex> ordDeg(size()), ordOtherDeg(size());
	for (unsigned long i = 0; i < size(); ++i) {
		ordDeg[i] = degreeIndex(deg[i] = degree(i), i);
		ordOtherDeg[i] = degreeIndex(otherDeg[i] = other.degree(i), i);
	}
	sort(ordDeg.begin(), ordDeg.end(), compFirstDegreeIndex());
	sort(ordOtherDeg.begin(), ordOtherDeg.end(), compFirstDegreeIndex());

	for (unsigned long i = 0; i < size(); ++i)
		if (ordDeg[i].first != ordOtherDeg[i].first)
			return 0;

	typedef pair<unsigned long, bool> vertexBool;

	vector< vector<vertexBool> > notUsed(size());
	vector<unsigned long> order(size());
	unsigned long start = 0;
	/* Backtrack order setup block */{
		unsigned long index = size();
		vbool used(size());
		for (unsigned long i = size(); i > 0; --i) {
			if (f[ordDeg[i-1].second] != other.size()) {
				order[start++] = ordDeg[i-1].second;
				if (used[f[ordDeg[i-1].second]])
					return false;
				used[f[ordDeg[i-1].second]] = true;
			}
			else {
				order[--index] = ordDeg[i-1].second;
			}
		}
		for (unsigned long i = 0; i < size(); ++i)
			if (!used[i])
				notUsed[otherDeg[i]].push_back(vertexBool(i, true));
	}	

    /* Given partial checking block */ {
		for (unsigned long i = 0; i < start; ++i)
			if (deg[order[i]] != otherDeg[f[order[i]]])
				return false;

		for (unsigned long i = 0; i < start; ++i)
			for (unsigned long j = i+1; j < start; ++j)
				if (m[size()*order[i] + order[j]]
					!= other.m[size()*f[order[i]] + f[order[j]]])
					return false;
	}
 
    /* Isomorphism completion block */ {
		unsigned long index;
		vector<unsigned long> btind(size());
		for (index = start; start <= index && index < size(); ) {
			const unsigned long v(order[index]);
			vector<vertexBool>& candidates(notUsed[deg[v]]);
			unsigned long& thisind(btind[v]);

			if (f[v] == size())
				thisind = 0;
			else {
				candidates[thisind].second = true;
				f[v] = size();
				++thisind;
			}
			while (true) {
				for ( ; thisind < candidates.size(); ++thisind)
					if (candidates[thisind].second)
						break;
				if (thisind == candidates.size()) {
					--index;
					break;
				}
				const unsigned long image(candidates[thisind].first);
				const unsigned long vLine(v * size());
				const unsigned long imageLine(image * size());
				unsigned long morphind;
				for (morphind = 0; morphind < index; ++morphind)
					if (m[vLine + order[morphind]]
						!= other.m[imageLine + f[order[morphind]]])
						break;
				if (morphind == index) {
					candidates[thisind].second = false;
					f[v] = image;
					++index;
					break;
				}
				++thisind;
			}
		}
		return index == size();
	}
}

/* findIsomorphism(const Graph& other, morphism& f) const:
 * tries to find an isomorphism between *this and other.
 * returns: true, if an isomorphism exists (f holds the isomorphism found)
 *          false, if no such isomorphism exists (f's data may be corrupted)
 */
bool Graph::findIsomorphism(const Graph& other, morphism& f) const {
	f = morphism(s, s);
	return completeIsomorphism(other, f);
}

/* operator==(const Graph& other) const:
 * returns: true, iff *this and other are isomorphic
 */
bool Graph::operator==(const Graph& other) const {
	morphism f;
	return findIsomorphism(other, f);
}

/* operator!=(const Graph& other) const:
 * returns: false, iff *this and other are isomorphic
 */
bool Graph::operator!=(const Graph& other) const {
	return !operator==(other);
}

/* degree(unsigned long v) const:
 * restrictions: v < size()
 * returns: degree of vertex v
 */
unsigned long Graph::degree(unsigned long v) const {
	unsigned long ret(0), line(v*size()), upper(line+size());
	for (unsigned long i = line; i < upper; ++i)
		if (m[i])
			++ret;
	return ret;
}

/* enumerate(unsigned long size):
 * returns: container with every graph of size size occurring exactly once
 */
container<Graph> Graph::enumerate(unsigned long size) {
	vector<vbool> mat(size, vbool(size));
	vector<unsigned long> degrees(size);
	container<Graph> ret(1, Graph(mat));

	while (true) {
		unsigned long inci(0), incj(0);
		for ( ; inci < size; ++inci) {
			for (incj = inci+1; incj < size; ++incj) {
				if (!mat[inci][incj])
					break;
				mat[inci][incj] = mat[incj][inci] = false;
				--degrees[inci];
				--degrees[incj];
			}
			if (incj < size)
				break;
		}
		if (inci == size)
			break;
		++degrees[inci];
		++degrees[incj];
	
		mat[inci][incj] = mat[incj][inci] = true;

		unsigned long index;
		for (index = 1; index < size; ++index)
			if (degrees[index-1] < degrees[index])
				break;
		if (index < size)
			continue;
		Graph G(mat);
		for (index = 0; index < ret.size(); ++index)
			if (G == ret[index])
				break;
		if (index == ret.size())
			ret.push_back(G);
	}
	return ret;
}


/**
Format for I/O of graphs:
(n
a1 b1
a2 b2
a3 b3
...
)
Where n is the number of vertices and the graph's edges are exactly {ai, bi} for all i.
The vertices of the graph are labeled in 0..n-1
**/

/* readFromFile(FILE *f):
 * reads a graph from file f and puts it on *this
 * returns: 1 if successful,
 *          -1 otherwise (*this holds corrupted data)
 */
int Graph::readFromFile(FILE *f) {
	if (fscanf(f, " (%lu", &s) != 1)
		return -1;
	m.assign(s*s, false);
	unsigned long a, b;
	int ret;
	while ((ret = fscanf(f, "%lu %lu", &a, &b)) == 2) {
		m[a*s + b] = m[b*s + a] = true;
	}
	if (ret != 0)
		return -1;
	if (fscanf(f, " )"))
		return -1;
	return 1;
}

/* writeOnFile(FILE *f) const:
 * writes *this on file f
 */
void Graph::writeOnFile(FILE *f) const {
	fprintf(f, "(%lu\n", s);
	for (unsigned long i = 0; i < s; ++i)
		for (unsigned long j = i+1; j < s; ++j)
			if (m[i*s + j])
				fprintf(f, "%lu %lu\n", i, j);
	fprintf(f, ")");
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
