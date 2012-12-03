#ifndef __MESH3D__TETRAHEDRON_H__
#define __MESH3D__TETRAHEDRON_H__

#include "vector.h"
#include "mesh.h"
#include "tri_face.h"

struct Tetrahedron : public Element{
	Vertex *p[4];
	TriFace *f[4];

	Tetrahedron(int _index, 
		Vertex *_p1, Vertex *_p2, Vertex *_p3, Vertex *_p4, 
		int _region) 
		: Element(_index) 
	{
		type = EL_TETRAHEDRON;
		region = _region;

		p[0] = _p1;
		p[1] = _p2;
		p[2] = _p3;
		p[3] = _p4;

		f[0] = new TriFace(4*_index+0, p[1], p[2], p[3], this);
		f[1] = new TriFace(4*_index+1, p[0], p[3], p[2], this);
		f[2] = new TriFace(4*_index+2, p[0], p[1], p[3], this);
		f[3] = new TriFace(4*_index+3, p[0], p[2], p[1], this);
		fillData();
	}
/*	Tetrahedron(int _index, 
		Vertex *_p1, Vertex *_p2, Vertex *_p3, Vertex *_p4, 
		int _region, 
		TriFace *_f1, TriFace *_f2, TriFace *_f3, TriFace *_f4) 
		: Element(_index) 
	{
		type = EL_TETRAHEDRON;
		region = _region;

		p[0] = _p1;
		p[1] = _p2;
		p[2] = _p3;
		p[3] = _p4;

		f[0] = _f1;
		f[1] = _f2;
		f[2] = _f3;
		f[3] = _f4;

		for (int i=0; i<4; i++)
			f[i]->element = this;

		fillData();
	}*/
	void fillData() {
		center.add(p[0]->r);
		center.add(p[1]->r);
		center.add(p[2]->r);
		center.add(p[3]->r);
		center.scale(0.25);

		Vector p41(p[0]->r), p42(p[1]->r), p43(p[2]->r), p412;
		p41.sub(p[3]->r);
		p42.sub(p[3]->r);
		p43.sub(p[3]->r);
		p41.cross(p42, p412);
		volume = 1./6 * p412.dot(p43);

		quality = getquality();
	}
	double getquality() {
		/* V/S^(3/2) related to equiliteral */
		double bsurf = 
			f[0]->surface+
			f[1]->surface+
			f[2]->surface+
			f[3]->surface;
		return sqrt((volume*volume/(bsurf * bsurf * bsurf))/(2./144.0/3.0/1.7320508075688772935));
	}
	virtual ~Tetrahedron() {
/*		for (int i = 0; i < 4; i++)
			delete f[i];*/
	}
private:
	Tetrahedron &operator=(const Tetrahedron &f);
	Tetrahedron(const Tetrahedron &f);
};

#endif
