#ifndef __MESH3D__MESH_GRAPH_H__
#define __MESH3D__MESH_GRAPH_H__

#ifdef USE_METIS

#include "graph.h"
#include <vector>
#include <map>

namespace mesh3d {

class mesh;

/** A class representing a nodal graph of a mesh */
class nodal_graph : public graph {
public:
	/** Construct nodal graph from mesh */
	nodal_graph(const mesh &m);
};

/** A class representing a tet graph of a mesh */
class tet_graph : public graph {
public:
	typedef std::vector<std::map<index, index> > global_to_local;
private:
	const mesh &m;
	global_to_local subvert;
public:
	/** Construct mesh graph from mesh */
	tet_graph(const mesh &m);
	bool partition(index num_parts);
	/** Return mapping between global vertex indices and local indices */
	const global_to_local &mapping() const {
		return subvert;
	}
};

}

#endif

#endif
