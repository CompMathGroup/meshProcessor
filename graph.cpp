#include "graph.h"

using namespace mesh3d;

bool graph::partition(index num_parts) {
	idx_t nparts = num_parts;
	idx_t ntvxs = _nadj.size() - 1;
	idx_t ncon = 1;
	idx_t objval;

	return METIS_OK == METIS_PartGraphKway(&ntvxs, &ncon, &_nadj[0], &_adj[0], 
		0, 0, 0, &nparts, 0, 0, 0, &objval, &_colors[0]);
}
