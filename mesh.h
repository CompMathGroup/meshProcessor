#ifndef __MESH3D__MESH_H__
#define __MESH3D__MESH_H__

#include "common.h"

#include "vertex.h"
#include "face.h"
#include "tetrahedron.h"

#include "simple_mesh.h"
#include "mesh_graph.h"

#include <vector>
#include <ostream>

namespace mesh3d {

class mesh {
	ptr_vector<vertex> _vertices;
	ptr_vector<face> _faces;
	ptr_vector<tetrahedron> _tets;
	index _domain;
	index _domains;
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
	mesh(const mesh &);
	mesh &operator=(const mesh &);
public:
	/** Construct mesh from simple mesh */
	mesh(const simple_mesh &sm, index dom = 0, index domains = 1);
	/** Construct mesh in domain from global mesh and tet_graph  */
	mesh(const mesh &sm, index dom, const tet_graph &tg);
	/** Construct from binary stream */
	mesh(std::istream &i);
	/** Export to binary stream */
	void serialize(std::ostream &o) const;
	/** Dump to text stream */
	void dump(std::ostream &o) const;

	/** Destroy mesh */
	~mesh();
	/** Return domain id */
	index domain() const;

	/** Run various checks on mesh */
	bool check(std::ostream *o = 0) const;

	/** Return mesh vertices as array */
	const ptr_vector<vertex> &vertices() const;
	/** Return mesh faces as array */
	const ptr_vector<face> &faces() const;
	/** Return mesh tetrahedrons as array */
	const ptr_vector<tetrahedron> &tets() const;
};

}

#endif
