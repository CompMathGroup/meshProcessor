#include "mesh.h"

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
		vertices.push_back(new vertex(vector(p[0], p[1], p[2])));
		vertices[i].set_color(BAD_INDEX);
	}

	index nT = sm.num_tetrahedrons();
	for (index i = 0; i < nT; i++) {
		const index *v = sm.tet_verts(i);
		tets.push_back(new tetrahedron(
			vertices[v[0]], vertices[v[1]], 
			vertices[v[2]], vertices[v[3]]));
		tets[i].set_color(sm.tet_material(i));
		for (int j = 0; j < 4; j++) {
			faces.push_back(&tets[i].f(j));
			tets[i].f(j).set_color(BAD_INDEX);
		}
	}

	index nB = sm.num_bnd_faces();
	for (index i = 0; i < nB; i++) {
		const index *v = sm.bnd_verts(i);
		faces.push_back(new face(
			vertices[v[0]], vertices[v[1]], vertices[v[2]], 0));
		faces[i].set_color(sm.bnd_material(i));
	}

	for (index i = 0; i < nV; i++) {
		vertices[i].set_idx(i);
		vertices[i].sort_lists();
	}

	for (index i = 0; i < nT; i++)
		tets[i].set_idx(i);

	index nF = nB + 4 * nT;

	for (index i = 0; i < nF; i++)
		faces[i].set_idx(i);

	for (index i = 0; i < nF; i++) {
		face *f = &faces[i];
		const std::vector<face_vertex> &f1 = f->p(0).faces();
		const std::vector<face_vertex> &f2 = f->p(1).faces();
		const std::vector<face_vertex> &f3 = f->p(2).faces();

		const face *flip = three_way_find_except(f1.begin(), f2.begin(), f3.begin(), f);
		f->set_flip(const_cast<face &>(*flip));
	}
}

mesh::~mesh() {
}
