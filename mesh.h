#ifndef __MESH3D__MESH_H__
#define __MESH3D__MESH_H__

#include "common.h"

#include "vertex.h"
#include "face.h"
#include "tetrahedron.h"

#include "simple_mesh.h"

#include <vector>
#include <ostream>

namespace mesh3d {

class mesh {
	ptr_vector<vertex> vertices;
	ptr_vector<face> faces;
	ptr_vector<tetrahedron> tets;
public:
	/** Construct mesh from simple mesh */
	mesh(const simple_mesh &sm);
	/** Destroy mesh */
	~mesh();
	/** Run various checks on mesh */
	bool check() const;
};

}

#endif
