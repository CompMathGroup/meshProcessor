#ifndef __MESH3D__TRI_FACE_H__
#define __MESH3D__TRI_FACE_H__

#include "vector.h"
#include "mesh.h"

struct TriFace : public Face {
	Vertex *p[3];

	TriFace (int _index, Vertex *_p1, Vertex *_p2, Vertex *_p3, Element *_element, int _borderType = -1): Face(_index, _element) {
		type = FC_TRIANGLE;

		p[0] = _p1;
		p[1] = _p2;
		p[2] = _p3;

		flip = 0;

		borderType = _borderType;
		fillData();
	}
	void fillData() {
		Vector p12(p[1]->r), p13(p[2]->r);
		center.add(p[0]->r);
		center.add(p[1]->r);
		center.add(p[2]->r);
		center.scale(1.0/3);
		p12.sub(p[0]->r);
		p13.sub(p[0]->r);
		p12.cross(p13, normal);
		surface = 0.5*normal.norm();
		normal.scale(0.5/surface);
	}
private:
	TriFace &operator=(const TriFace &f);
	TriFace(const TriFace &f);
};

#endif
