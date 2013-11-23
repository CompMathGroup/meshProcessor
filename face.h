#ifndef __MESH3D__FACE_H__
#define __MESH3D__FACE_H__

#include "vertex.h"
#include "element.h"

class tetrahedron;

namespace mesh3d {

/** A class that represents a single face from mesh */
class face : public element {
	const index _color;

	vertex *_p[3];
	tetrahedron *_tet;
	face *_flip;

	vector _normal;
	vector _center;
	double _surface;

	face(const face &other);
	face &operator =(const face &other);
public:
	/** Construct a face with three vertices p1, p2 and p3 and tetrahedron t */
	face(vertex &p1, vertex &p2, vertex &p3, tetrahedron &t) {
		_p[0] = &p1;
		_p[1] = &p2;
		_p[2] = &p3;

		_tet = &t;

		_center += p1.r();
		_center += p2.r();
		_center += p3.r();
		_center *= 1.0 / 3;

		_normal = (p2.r() - p1.r()) % (p3.r() - p1.r());

		_surface = 0.5 * norm(_normal);
		_normal *= 0.5 / _surface;

		_flip = 0;
	}
	
	/** Checks if face is a border face */
	bool is_border() const {
		return _tet == 0;
	}

	/** Get associated tetrahedron */
	tetrahedron &tet() const {
		MESH3D_ASSERT(!is_border());
		return *_tet;
	}

	/** Get vertex using local index */
	vertex &p(int li) const {
		MESH3D_ASSERT(li >= 0 && li < 3);
		return *_p[li];
	}

	/** Return face surface */
	double surface() const {
		return _surface;
	}

	/** Return face normal 
	* 
	* Face normals are always directed toward tetrahedron's center, to allow flipped ones,
	* which could be boundary, to be directed outside of the domain */
	const vector &normal() const {
		return _normal;
	}

	/** Get flipped face */
	face &flip() const {
		MESH3D_ASSERT(_flip);
		return *_flip;
	}

	/** Set flipped face */
	void set_flip(face &flip) {
		_flip = &flip;
	}
};

}

#endif
