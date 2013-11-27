#ifndef __MESH3D__VERTEX_H__
#define __MESH3D__VERTEX_H__

#include "vector.h"
#include "element.h"
#include <vector>
#include <algorithm>
#include <stdint.h>

namespace mesh3d {

class tetrahedron;
class face;

/** A pair of tetrahedron with local vertex index in it. */
struct tet_vertex {
	const tetrahedron *t; //!< A pointer to the tetrahedron object
	int li; //!< local index of vertex in the tetrahedron
	/** Construct using tetrahedron pointer and local vertex index */
	tet_vertex(const tetrahedron *tet, int local_idx) : t(tet), li(local_idx) { }
	/** Comparator for sorting */
	static bool less(const tet_vertex &a, const tet_vertex &b) { return a.t < b.t; }
};

/** A pair of face with local vertex index in it */
struct face_vertex {
	const face *f; //!< A pointer to the face object
	int li; //!< local index of vertex in the face
	/** Construct using face pointer and local vertex index */
	face_vertex(const face *fac, int local_idx) : f(fac), li(local_idx) { }
	/** Comparator for sorting */
	static bool less(const face_vertex &a, const face_vertex &b) { return a.f < b.f; }
};

/** A pair to identify vertex in other domains */
struct dom_vertex {
	index remote_idx;
	uint64_t domain_id;
	/** Construct using domain id and remote index  */
	dom_vertex(uint64_t domain_id, index remote_idx) : remote_idx(remote_idx), domain_id(domain_id) { }
	/** Comparator for sorting */
	static bool less(const dom_vertex &a, const dom_vertex &b) { return a.domain_id < b.domain_id; }
};

/** A class that repesents single vertex in the mesh */
class vertex : public element {
	vector _r;

	std::vector<tet_vertex> _tetrahedrons;
	std::vector<face_vertex> _faces;
	std::vector<dom_vertex> _aliases;

	vertex(const vertex &p);
	vertex &operator =(const vertex &p);
public:
	/** Construct vertex with index idx and position r */
	vertex(const vector &r) : _r(r) { }

	/** Get vertex position */
	const vector &r() const { return _r; }

	/** Get a list of tetrahedrons that contain this vertex */
	const std::vector<tet_vertex> &tetrahedrons() const {
		return _tetrahedrons;
	}
	
	/** Get a list of faces that contain this vertex */
	const std::vector<face_vertex> &faces() const {
		return _faces;
	}

	/** Get a list of foreign aliases for this vertex */
	const std::vector<dom_vertex> &aliases() const {
		return _aliases;
	}

	/** Add a tetrahedron to vertex tetrahedrons list */
	void add(const tetrahedron *t, int li) {
		_tetrahedrons.push_back(tet_vertex(t, li));
	}

	/** Add a face to vertex faces list */
	void add(const face *f, int li) {
		_faces.push_back(face_vertex(f, li));
	}

	/** Add an alias from another domain */
	void add(int domain_id, index remote_idx) {
		_aliases.push_back(dom_vertex(domain_id, remote_idx));
	}

	/** Sort tetrahedrons and faces lists */
	void sort_lists() {
		std::sort(_tetrahedrons.begin(), _tetrahedrons.end(), tet_vertex::less);
		std::sort(_faces.begin(), _faces.end(), face_vertex::less);
		std::sort(_aliases.begin(), _aliases.end(), dom_vertex::less);
	}
};

}

#endif
