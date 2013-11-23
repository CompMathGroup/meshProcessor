#ifndef __MESH3D__MESH_H__
#define __MESH3D__MESH_H__

#include "vertex.h"
#include "face.h"
#include "tetrahedron.h"

#include "simple_mesh.h"

#include <vector>

class mesh {
	std::vector<vertex> vertices;
	std::vector<face> faces;
	std::vector<tetrahedron> tets;

	std::ofstream *logger;

	void log(const std::string &msg) { if (logger) *logger << msg; }
public:
	/** Construct mesh from simple mesh and optional log stream */
	mesh(const simple_mesh &sm, std::ostream *ostr = 0);
	/** Destroy mesh */
	~mesh();
	/** Run various checks on mesh */
	bool check() const;
};

#endif
