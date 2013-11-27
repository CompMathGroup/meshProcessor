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
	ptr_vector<vertex> _vertices;
	ptr_vector<face> _faces;
	ptr_vector<tetrahedron> _tets;
	bool checkVertexIndices(index &wrong) const;
	bool checkTetIndices(index &wrong) const;
	bool checkFaceIndices(index &wrong) const;
	bool checkFlippedFace(index &wrong) const;
	bool checkFlippedOrient(index &wrong) const;
	bool checkFaceSurface(index &wrong) const;
	bool checkTetVolume(index &wrong) const;
	bool checkTetFaceNormals(index &wrong, int &faceno) const;
	bool checkVertexFaceList(index &wrong, index &faceno, int &vertno, index &cnt) const;
	bool checkVertexTetList(index &wrong, index &tetno, int &vertno, index &cnt) const;
	void log(std::ostream *o, const std::string &msg) const;
public:
	/** Construct mesh from simple mesh */
	mesh(const simple_mesh &sm);
	/** Destroy mesh */
	~mesh();
	/** Run various checks on mesh */
	bool check(std::ostream *o = 0) const;

	const ptr_vector<vertex> &vertices() const;
	const ptr_vector<face> &faces() const;
	const ptr_vector<tetrahedron> &tets() const;
};

}

#endif
