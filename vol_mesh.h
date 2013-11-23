#ifndef __MESH3D__VOL_MESH_H__
#define __MESH3D__VOL_MESH_H__

#include "simple_mesh.h"
#include <vector>

namespace mesh3d {

/** simple_mesh implementation for vol mesh format (NETGEN default output) */
class vol_mesh : public simple_mesh {
	int nV;
	int nB;
	int nT;
	std::vector<double> vert;
	std::vector<index> bnd;
	std::vector<index> tet;
	std::vector<index> bndmat;
	std::vector<index> tetmat;
public:
	/** Constuct vol_mesh from file fn */
	vol_mesh(const char *fn);
	/** Destroy vol_mesh object */
	virtual ~vol_mesh();
	/** Return number of vertices in mesh */
	virtual index num_vertices() const;
	/** Return number of tetrahedrons in mesh */
	virtual index num_tetrahedrons() const;
	/** Return number of boundary faces in mesh */
	virtual index num_bnd_faces() const;
	/** Return i-th vertex coordinates as an array of 3 doubles */
	virtual const double *vertex_coord(index i) const;
	/** Return i-th tetrahedron vertices as an array of 4 indices. Order matters */
	virtual const index *tet_verts(index i) const;
	/** Return i-th boundary face vertices as an array of 3 indices. Order matters */
	virtual const index *bnd_verts(index i) const;
	/** Return i-th tetrahedron material (color) */
	virtual index tet_material(index i) const;
	/** Return i-th boundary face material (color) */
	virtual index bnd_material(index i) const;
};

}

#endif
