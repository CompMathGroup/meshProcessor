#ifndef __MESH3D__SIMPLE_MESH__
#define __MESH3D__SIMPLE_MESH__

#include "common.h"

namespace mesh3d {

/** An abstract class used to represent mesh after generation. Holds only basic mesh info */
class simple_mesh {
public:
	/** Virtual destructor */
	virtual ~simple_mesh() { }
	/** Return number of vertices in mesh */
	virtual index num_vertices() const = 0;
	/** Return number of tetrahedrons in mesh */
	virtual index num_tetrahedrons() const = 0;
	/** Return number of boundary faces in mesh */
	virtual index num_bnd_faces() const = 0;
	/** Return i-th vertex coordinates as an array of 3 doubles */
	virtual const double *vertex_coord(index i) const = 0;
	/** Return i-th tetrahedron vertices as an array of 4 indices. Order matters */
	virtual const index *tet_verts(index i) const = 0;
	/** Return i-th boundary face vertices as an array of 3 indices. Order matters */
	virtual const index *bnd_verts(index i) const = 0;
	/** Return i-th tetrahedron material (color) */
	virtual index tet_material(index i) const = 0;
	/** Return i-th boundary face material (color) */
	virtual index bnd_material(index i) const = 0;
};

}

#endif
