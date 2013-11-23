#ifndef __MESH3D__VERTEX_H__
#define __MESH3D__VERTEX_H__

#include "vector.h"
#include "element.h"
#include <vector>
#include <algorithm>

namespace mesh3d {

class tetrahedron;
class face;

/** A pair of tetrahedron with local vertex index in it. */
struct tet_vertex {
	const tetrahedron *t; //!< A pointer to the tetrahedron object
	int li; //!< local index of vertex in the tetrahedron
	/** Construct using tetrahedron pointer and local vertex index */
	tet_vertex(const tetrahedron *tet, int local_idx) : t(tet), li(local_idx) { }
	static bool less(const tet_vertex &a, const tet_vertex &b) { return a.t < b.t; }
};

/** A pair of face with local vertex index in it */
struct face_vertex {
	const face *f; //!< A pointer to the face object
	int li; //!< local index of vertex in the face
	/** Construct using face pointer and local vertex index */
	face_vertex(const face *fac, int local_idx) : f(fac), li(local_idx) { }
	static bool less(const face_vertex &a, const face_vertex &b) { return a.f < b.f; }
};

/** A class that repesents single vertex in the mesh */
class vertex : public element {
	vector _r;

	std::vector<tet_vertex> _elements;
	std::vector<face_vertex> _faces;

	vertex(const vertex &p);
	vertex &operator =(const vertex &p);
public:
	/** Construct vertex with index idx and position r */
	vertex(const vector &r) : _r(r) { }

	/** Get vertex position */
	const vector &r() const { return _r; }

	/** Get a list of tetrahedrons that contain this vertex */
	const std::vector<tet_vertex> &elements() const {
		return _elements;
	}
	
	/** Get a list of faces that contain this vertex */
	const std::vector<face_vertex> &faces() const {
		return _faces;
	}

	/** Add a tetrahedron to vertex tetrahedrons list */
	void add(const tetrahedron *t, int li) {
		_elements.push_back(tet_vertex(t, li));
	}

	/** Add a face to vertex faces list */
	void add(const face *f, int li) {
		_faces.push_back(face_vertex(f, li));
	}

	/** Sort tetrahedrons and faces lists */
	void sort_lists() {
		std::sort(_elements.begin(), _elements.end(), tet_vertex::less);
		std::sort(_faces.begin(), _faces.end(), face_vertex::less);
	}
};

}

#endif
