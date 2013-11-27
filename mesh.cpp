#include "mesh.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace mesh3d;

typedef std::vector<face_vertex>::const_iterator face_iter_t;

const face *three_way_find_except(face_iter_t x, face_iter_t y, face_iter_t z, const face *p) {
	const face *a = x->f;
	const face *b = y->f;
	const face *c = z->f;

	while (true) {
		if (a == b && b == c) {
			if (a != p)
				return a;
		}
		if (a < b) {
			if (a < c)
				a = (x++)->f;
			else
				c = (z++)->f;
		} else {
			if (b < c)
				b = (y++)->f;
			else
				c = (z++)->f;
		}
	}
}

mesh::mesh(const simple_mesh &sm) {
	index nV = sm.num_vertices();
	for (index i = 0; i < nV; i++) {
		const double *p = sm.vertex_coord(i);
		_vertices.push_back(new vertex(vector(p[0], p[1], p[2])));
		_vertices[i].set_color(BAD_INDEX);
	}

	index nT = sm.num_tetrahedrons();
	for (index i = 0; i < nT; i++) {
		const index *v = sm.tet_verts(i);
		_tets.push_back(new tetrahedron(
			_vertices[v[0]], _vertices[v[1]], 
			_vertices[v[2]], _vertices[v[3]]));
		_tets[i].set_color(sm.tet_material(i));
		for (int j = 0; j < 4; j++) {
			_faces.push_back(&_tets[i].f(j));
			_tets[i].f(j).set_color(BAD_INDEX);
		}
	}

	index nB = sm.num_bnd_faces();
	for (index i = 0; i < nB; i++) {
		const index *v = sm.bnd_verts(i);
		_faces.push_back(new face(
			_vertices[v[0]], _vertices[v[1]], _vertices[v[2]], 0));
		_faces[i].set_color(sm.bnd_material(i));
	}

	for (index i = 0; i < nV; i++) {
		_vertices[i].set_idx(i);
		_vertices[i].sort_lists();
	}

	for (index i = 0; i < nT; i++)
		_tets[i].set_idx(i);

	index nF = nB + 4 * nT;

	for (index i = 0; i < nF; i++)
		_faces[i].set_idx(i);

	for (index i = 0; i < nF; i++) {
		face *f = &_faces[i];
		const std::vector<face_vertex> &f1 = f->p(0).faces();
		const std::vector<face_vertex> &f2 = f->p(1).faces();
		const std::vector<face_vertex> &f3 = f->p(2).faces();

		const face *flip = three_way_find_except(f1.begin(), f2.begin(), f3.begin(), f);
		f->set_flip(const_cast<face &>(*flip));
	}
}

mesh::~mesh() { }

struct string_adder {
	std::stringstream ss;
	template<class T>
	string_adder &operator +(const T &val) { ss << val; return *this; }
	operator std::string() { return ss.str(); }
};

#define _ string_adder()

void mesh::log(std::ostream *o, const std::string &msg) const {
	if (!o)
		return;
	*o << msg << std::endl;
}

bool mesh::check(std::ostream *o) const {
	bool ok = true, lastcheck;
	index wrong;

	index nT = _tets.size();
	index nF = _faces.size();

	ok &= lastcheck = checkVertexIndices(wrong);
	if (!lastcheck)
		log(o, _ + "Vertex #" + wrong + " has wrong index");

	ok &= lastcheck = checkFaceIndices(wrong);
	if (!lastcheck)
		log(o, _ + "Face #" + wrong + " has wrong index");

	ok &= lastcheck = checkTetIndices(wrong);
	if (!lastcheck)
		log(o, _ + "Tet #" + wrong + " has wrong index");

	ok &= lastcheck = checkFlippedFace(wrong);
	if (!lastcheck)
		log(o, _ + "Face #" + wrong + " twice filpped is not the same");

	ok &= lastcheck = checkFlippedOrient(wrong);
	if (!lastcheck)
		log(o, _ + "Face #" + wrong + " flip not opposed to face (probably wrong oriented face)");

	ok &= lastcheck = checkFaceSurface(wrong);
	if (!lastcheck)
		log(o, _ + "Face #" + wrong + " has negative surface = " + _faces[wrong].surface());

	ok &= lastcheck = checkTetVolume(wrong);
	if (!lastcheck)
		log(o, _ + "Tet #" + wrong + " has negative volume = " + _tets[wrong].volume());

	int fno;
	ok &= lastcheck = checkTetFaceNormals(wrong, fno);
	if (!lastcheck)
		log(o, _ + "Tet #" + wrong + " has " + fno + "-th face improperly oriented");

	index faceno, tetno, cnt;
	int vertno;
	ok &= lastcheck = checkVertexTetList(wrong, tetno, vertno, cnt);
	if (!lastcheck)
		log(o, _ + "Tet #" + tetno + " has " + vertno + "-th vertex incorrectly listed in vertex #" + wrong + " _tets list");

	lastcheck = (cnt == 4 * nT);
	ok &= lastcheck;
	if (!lastcheck)
		log(o, _ + "Total elems list has wrong size " + cnt + " != " + (4 * nT));

	ok &= lastcheck = checkVertexFaceList(wrong, faceno, vertno, cnt);
	if (!lastcheck)
		log(o, _ + "Face #" + faceno + " has " + vertno + "-th vertex " +
			"incorrectly listed in vertex #" + wrong + " _faces list");

	lastcheck = (cnt == 3 * nF);
	ok &= lastcheck;
	if (!lastcheck)
		log(o, _ + "Total _faces list has wrong size " + cnt + " != " + (3 * nF));

	return ok;
}

bool mesh::checkVertexIndices(index &wrong) const {
	index nV = _vertices.size();

	for (index i = 0; i < nV; i++)
		if (_vertices[i].idx() != i) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkFaceIndices(index &wrong) const {
	index nF = _faces.size();

	for (index i = 0; i < nF; i++)
		if (_faces[i].idx() != i) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkTetIndices(index &wrong) const {
	index nT = _tets.size();

	for (index i = 0; i < nT; i++)
		if (_tets[i].idx() != i) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkFlippedFace(index &wrong) const {
	index nF = _faces.size();

	for (index i = 0; i < nF; i++)
		if (&_faces[i].flip().flip() != &_faces[i]) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkFlippedOrient(index &wrong) const {
	index nF = _faces.size();

	for (index i = 0; i < nF; i++) {
		double cosv = _faces[i].flip().normal().dot(_faces[i].normal());
		if (std::fabs(cosv + 1) > 1e-10) {
			wrong = i;
			return false;
		}
	}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkFaceSurface(index &wrong) const {
	index nF = _faces.size();

	for (index i = 0; i < nF; i++)
		if (_faces[i].surface() <= 0) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkTetVolume(index &wrong) const {
	index nT = _tets.size();

	for (index i = 0; i < nT; i++)
		if (_tets[i].volume() <= 0) {
			wrong = i;
			return false;
		}
	wrong = BAD_INDEX;
	return true;
}

bool mesh::checkTetFaceNormals(index &wrong, int &faceno) const {
	index nT = _tets.size();

	for (index i = 0; i < nT; i++) {
		for (int j = 0; j < 4; j++) {
			vector r = (1.0 / 3) * _tets[i].f(j).surface() * (_tets[i].p(j).r() - _tets[i].f(j).center());
			double fv = r.dot(_tets[i].f(j).normal());
			if (std::fabs(fv - _tets[i].volume()) > 
				1e-12 * std::fabs(_tets[i].volume())) 
			{
				wrong = i;
				faceno = j;
				return false;
			}
		}
	}

	wrong = BAD_INDEX;
	faceno = -1;

	return true;
}

bool mesh::checkVertexTetList(index &wrong, index &tetno, int &vertno, index &cnt) const {
	bool check, ret = true;
	index nV = _vertices.size();
	cnt = 0;

	for (int i = 0; i < nV; i++) {
		const std::vector<tet_vertex> &vl = _vertices[i].tetrahedrons();
		for (std::vector<tet_vertex>::const_iterator it = vl.begin();
			it != vl.end(); it++)
		{
			cnt++;
			const tetrahedron *tet = it->t;
			check = &tet->p(it->li) == &_vertices[i];
			if (!check) {
				wrong = i;
				tetno = tet->idx();
				vertno = it->li;
				ret = false;
			}
		}
	}
	return ret;
}

bool mesh::checkVertexFaceList(index &wrong, index &faceno, int &vertno, index &cnt) const {
	bool check, ret = true;
	index nV = _vertices.size();
	cnt = 0;

	for (int i = 0; i < nV; i++) {
		const std::vector<face_vertex> &vl = _vertices[i].faces();
		for (std::vector<face_vertex>::const_iterator it = vl.begin();
			it != vl.end(); it++)
		{
			cnt++;
			const face *f = it->f;
			check = &f->p(it->li) == &_vertices[i];
			if (!check) {
				wrong = i;
				faceno = f->idx();
				vertno = it->li;
				ret = false;
			}
		}
	}
	return ret;
}

const ptr_vector<vertex> &mesh::vertices() const {
	return _vertices;
}

const ptr_vector<face> &mesh::faces() const {
	return _faces;
}

const ptr_vector<tetrahedron> &mesh::tets() const {
	return _tets;
}
