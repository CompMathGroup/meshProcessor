#include "mesh_graph.h"
#include "mesh.h"

using namespace mesh3d;

nodal_graph::nodal_graph(const mesh &m) : graph(m.vertices().size()) {
	for (index i = 0; i < m.vertices().size(); i++) {
		const std::vector<tet_vertex> &tl = m.vertices()[i].tetrahedrons();
		for (std::vector<tet_vertex>::const_iterator it = tl.begin();
			it != tl.end(); it++)
		{
			for (int j = 0; j < 4; j++)
				if (j != it->li)
					add_edge(i, it->t->p(j).idx());
		}
	}
	compact();
}

tet_graph::tet_graph(const mesh &m) : graph(m.tets().size()), m(m) {
	for (index i = 0; i < m.tets().size(); i++) {
		const tetrahedron &tet = m.tets()[i];
		for (int j = 0; j < 4; j++) {
			const face &other = tet.f(j).flip();
			if (other.is_border())
				continue;
			add_edge(i, other.tet().idx());
		}
	}
	compact();
}

bool tet_graph::partition(index num_parts) {
	subvert.clear();
	if (!graph::partition(num_parts))
		return false;
	subvert.resize(num_parts);
	for (index i = 0; i < m.tets().size(); i++) {
		index dom = colors()[i];
		const tetrahedron &tet = m.tets()[i];
		for (int j = 0; j < 4; j++)
			subvert[dom].insert(std::pair<index, index>(tet.p(j).idx(), BAD_INDEX));
	}

	for (index d = 0; d < num_parts; d++) {
		typedef std::map<index, index> mapping_t;
		index local = 0;
		for (mapping_t::iterator it = subvert[d].begin();
			it != subvert[d].end(); it++)
		{
			it->second = local++;
		}
	}

	return true;
}
