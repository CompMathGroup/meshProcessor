#ifndef __MESH3D__TETRAHEDRON_H__
#define __MESH3D__TETRAHEDRON_H__

#include "vertex.h"
#include "face.h"
#include "element.h"

namespace mesh3d {

/** A class that represents single tetrahedron from the mesh */
class tetrahedron : public element {
	vertex *_p[4];
	face *_f[4];

	vector _center;
	double _volume;
public:
	/** Get vertex with specified local index */
	vertex &p(int vertex_li) const {
		MESH3D_ASSERT(vertex_li >= 0 && vertex_li < 4);
		return *_p[vertex_li];
	}
	/** Get face with specified local index */
	face &f(int face_li) const {
		MESH3D_ASSERT(face_li >= 0 && face_li < 4);
		return *_f[face_li];
	}

	/** Get tetrahedron volume */
	double volume() const {
		return _volume;
	}

	/** Get tetrahedron center */
	const vector &center() const {
		return _center;
	}

	/** Construct tetrahedron from four vertices */
	tetrahedron(vertex &p1, vertex &p2, vertex &p3, vertex &p4) {
		_p[0] = &p1;
		_p[1] = &p2;
		_p[2] = &p3;
		_p[3] = &p4;

		for (int j = 0; j < 4; j++)
			_p[j]->add(this, j);

		_f[0] = new face(p2, p3, p4, this, 0);
		_f[1] = new face(p1, p4, p3, this, 1);
		_f[2] = new face(p1, p2, p4, this, 2);
		_f[3] = new face(p1, p3, p2, this, 3);

		_center += p1.r();
		_center += p2.r();
		_center += p3.r();
		_center += p4.r();
		_center *= 0.25;

		_volume = 1. / 6 * (p3.r() - p4.r()).dot((p1.r() - p4.r()) % (p2.r() - p4.r()));
	}

	/** Destroy tetrahedron object */
	~tetrahedron() {
	}
};

}

#endif
