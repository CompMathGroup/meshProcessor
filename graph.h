#ifndef __MESH3D__GRAPH_H__
#define __MESH3D__GRAPH_H__

#include "common.h"
#include <vector>
#include <set>
#include <metis.h>

namespace mesh3d {

/** A class to represent graph suitable for METIS partitioning */
class graph {
	std::vector<idx_t> _nadj;
	std::vector<idx_t> _adj;
	std::vector<idx_t> _colors;

	std::vector<std::set<index> > _m;
protected:
	/** Create a graph with num_vertex vertices */
	graph(index num_vertex) : _colors(num_vertex), _m(num_vertex) { }
	/** Add directed edge (u -> v) to the graph. 
	 *
	 * Loops or duplicated edges are ignored */
	void add_edge(index u, index v, double w = 1) {
		if (u == v)
			return;
		_m[u].insert(v);
	}
	/** Compact graph into CSR format */
	void compact() {
		_nadj.resize(_m.size() + 1);
		_nadj[0] = 0;
		for (index i = 0; i < _m.size(); i++) {
			_nadj[i+1] = _nadj[i] + _m[i].size();
			for (std::set<index>::const_iterator it = _m[i].begin();
				it != _m[i].end(); it++) 
			{
				_adj.push_back(*it);
			}
		}
		MESH3D_ASSERT(_adj.size() == static_cast<size_t>(_nadj.back()));
	}
protected:	
	bool partition(index num_parts);
public:
	const std::vector<idx_t> &colors() const { return _colors; }
};

}

#endif
