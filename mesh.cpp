#include "mesh.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <stdint.h>

using namespace mesh3d;

const uint64_t MESH3D_SIGNATURE = 0x004853454d544554ull;

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

#ifdef USE_METIS
mesh::mesh(const mesh &m, index dom, const tet_graph &tg) {
	_domain = dom;
	_domains = tg.mapping().size();

	typedef std::map<index, index> mapping_t;
	mapping_t tg2l;
	std::vector<index> tl2g;
	const mapping_t &g2l = tg.mapping()[dom];
	
	for (mapping_t::const_iterator it = g2l.begin();
		it != g2l.end(); ++it)
	{
		const vertex &v = m.vertices(it->first);
		_vertices.push_back(new vertex(v.r()));
		_vertices.back().set_color(v.color());
	}

	index nT = m.tets().size();
	for (index i = 0, k = 0; k < nT; k++) {
		const tetrahedron &tet = m.tets(k);
		if (static_cast<index>(tg.colors(tet.idx())) != dom)
			continue;
		index v[4];
		for (int j = 0; j < 4; j++)
			v[j] = g2l.find(tet.p(j).idx())->second;
		_tets.push_back(new tetrahedron(
			_vertices[v[0]], _vertices[v[1]], 
			_vertices[v[2]], _vertices[v[3]]));
		_tets[i].set_color(tet.color());
		for (int j = 0; j < 4; j++) {
			_faces.push_back(&_tets[i].f(j));
			_tets[i].f(j).set_color(tet.f(j).color());
		}
		tg2l[k] = i;
		tl2g.push_back(k);
		i++;
	}
	nT = tl2g.size();

	index b[3];
	for (index i = 0; i < nT; i++) {
		index gi = tl2g[i];
		const tetrahedron &tet = m.tets(gi);
		for (int j = 0; j < 4; j++) {
			const face &f = tet.f(j).flip();
			for (int k = 0; k < 3; k++) {
				index gi = f.p(k).idx();
				b[k] = g2l.find(gi)->second;
				vertex &vx = _vertices[b[k]];
				for (index d = 0; d < tg.mapping().size(); d++) {
					if (d == dom)
						continue;
					const mapping_t &dmap = tg.mapping()[d];
					mapping_t::const_iterator mit = dmap.find(gi);
					if (mit != dmap.end())
						vx.add(d, mit->second);
				}
			}
			if (f.is_border() || static_cast<index>(tg.colors(f.tet().idx())) != dom) {
				_faces.push_back(new face(
					_vertices[b[0]], _vertices[b[1]], _vertices[b[2]], 0, -1));
				_faces.back().set_color(f.color());
				_faces.back().set_flip(_tets[i].f(j));
				_tets[i].f(j).set_flip(_faces.back());
			} 
		}
	}

	for (index i = 0; i < nT; i++) {
		index gi = tl2g[i];
		const tetrahedron &tet = m.tets(gi);
		for (int j = 0; j < 4; j++) {
			const face &f = tet.f(j).flip();
			if (!f.is_border() && static_cast<index>(tg.colors(f.tet().idx())) == dom) {
				const index other_tet = tg2l[f.tet().idx()];
				int fi = f.face_local_index();
				_tets[i].f(j).set_flip(_tets[other_tet].f(fi));
			} 
		}
	}

	for (index i = 0; i < _vertices.size(); i++)
		_vertices[i].set_idx(i);

	for (index i = 0; i < _tets.size(); i++)
		_tets[i].set_idx(i);

	for (index i = 0; i < _faces.size(); i++)
		_faces[i].set_idx(i);
}
#endif

mesh::mesh(const simple_mesh &sm, index dom, index domains) {
	_domain = dom;
	_domains = domains;
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
			_vertices[v[0]], _vertices[v[1]], _vertices[v[2]], 0, -1));
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

/*
	Mesh format

	u64 sig // signature - TETMESH\0
	u64 dom // domain #
	u64 nV // vertex #
	u64 nT // tets #
	u64 nB // boundary faces #
	u64 nI // interface vertex #
	f64 vertex[3xnV] // vertex x, y, z
	u64 tets[4xnT] // tet vertex ids
	u64 bnds[3xnB] // bnd faces vertex ids
	s64 vcol[nV] // vertex color
	s64 tcol[nT] // tet color
	s64 fcol[4xnT+nB] // face color
	u64 flips[4xnT+nB] // flipped faces
	nI x {
		u64 idx // vertex id
		u64 nA // number of aliases for this vertex
		nA x {
			u64 other_domain
			u64 remote_idx
		}
	}
*/
mesh::mesh(std::istream &is) {
	uint64_t sig;
	uint64_t nV, nT, nB, nI, dom, doms;
	double p[3];
	uint64_t v[4];
	uint64_t b[3];

	is.read(reinterpret_cast<char *>(&sig), sizeof(sig));
	if (sig != MESH3D_SIGNATURE)
		throw std::invalid_argument("Invalid mesh file signature");
	is.read(reinterpret_cast<char *>(&dom), sizeof(dom));
	_domain = dom;
	is.read(reinterpret_cast<char *>(&doms), sizeof(doms));
	_domains = doms;
	is.read(reinterpret_cast<char *>(&nV), sizeof(nV));
	is.read(reinterpret_cast<char *>(&nT), sizeof(nT));
	is.read(reinterpret_cast<char *>(&nB), sizeof(nB));
	is.read(reinterpret_cast<char *>(&nI), sizeof(nI));
	for (uint64_t i = 0; i < nV; i++) {
		is.read(reinterpret_cast<char *>(&p[0]), sizeof(p));
		_vertices.push_back(new vertex(vector(p[0], p[1], p[2])));
	}
	for (uint64_t i = 0; i < nT; i++) {
		is.read(reinterpret_cast<char *>(&v[0]), sizeof(v));
		_tets.push_back(new tetrahedron(
			_vertices[v[0]], _vertices[v[1]], 
			_vertices[v[2]], _vertices[v[3]]));
		_tets[i].set_idx(i);
		for (int j = 0; j < 4; j++)
			_faces.push_back(&_tets[i].f(j));
	}
	for (uint64_t i = 0; i < nB; i++) {
		is.read(reinterpret_cast<char *>(&b[0]), sizeof(b));
		_faces.push_back(new face(
			_vertices[b[0]], _vertices[b[1]], _vertices[b[2]], 0, -1));
	}
	for (uint64_t i = 0; i < nV; i++) {
		int64_t col;
		is.read(reinterpret_cast<char *>(&col), sizeof(col));
		_vertices[i].set_color(col);
		_vertices[i].set_idx(i);
	}
	for (uint64_t i = 0; i < nT; i++) {
		int64_t col;
		is.read(reinterpret_cast<char *>(&col), sizeof(col));
		_tets[i].set_color(col);
		_tets[i].set_idx(i);
	}
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		int64_t col;
		is.read(reinterpret_cast<char *>(&col), sizeof(col));
		_faces[i].set_color(col);
		_faces[i].set_idx(i);
	}
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		uint64_t flip;
		is.read(reinterpret_cast<char *>(&flip), sizeof(flip));
		_faces[i].set_flip(_faces[flip]);
	}
	for (uint64_t i = 0; i < nI; i++) {
		uint64_t vi, nA;
		is.read(reinterpret_cast<char *>(&vi), sizeof(vi));
		is.read(reinterpret_cast<char *>(&nA), sizeof(nA));
		for (uint64_t j = 0; j < nA; j++) {
			uint64_t did, rid;
			is.read(reinterpret_cast<char *>(&did), sizeof(did));
			is.read(reinterpret_cast<char *>(&rid), sizeof(rid));
			_vertices[vi].add(did, rid);
		}
	}
}

void mesh::serialize(std::ostream &os) const {
	uint64_t sig = MESH3D_SIGNATURE;
	uint64_t nV, nT, nB, nI;
	uint64_t dom = _domain, doms = _domains;
	double p[3];
	uint64_t v[4];
	uint64_t b[3];

	nV = _vertices.size();
	nT = _tets.size();
	nB = _faces.size() - 4 * nT;
	nI = 0;
	for (uint64_t i = 0; i < nV; i++)
		if (_vertices[i].aliases().size() > 0)
			nI++;

	os.write(reinterpret_cast<char *>(&sig), sizeof(sig));
	os.write(reinterpret_cast<char *>(&dom), sizeof(dom));
	os.write(reinterpret_cast<char *>(&doms), sizeof(doms));
	os.write(reinterpret_cast<char *>(&nV), sizeof(nV));
	os.write(reinterpret_cast<char *>(&nT), sizeof(nT));
	os.write(reinterpret_cast<char *>(&nB), sizeof(nB));
	os.write(reinterpret_cast<char *>(&nI), sizeof(nI));
	for (uint64_t i = 0; i < nV; i++) {
		const vertex &vr = _vertices[i];
		p[0] = vr.r().x;
		p[1] = vr.r().y;
		p[2] = vr.r().z;
		os.write(reinterpret_cast<char *>(&p[0]), sizeof(p));
	}
	for (uint64_t i = 0; i < nT; i++) {
		const tetrahedron &tet = _tets[i];
		for (int j = 0; j < 4; j++)
			v[j] = tet.p(j).idx();
		os.write(reinterpret_cast<char *>(&v[0]), sizeof(v));
	}
	for (uint64_t i = 0; i < nB; i++) {
		const face &f = _faces[4 * nT + i];
		for (int j = 0; j < 3; j++)
			b[j] = f.p(j).idx();
		os.write(reinterpret_cast<char *>(&b[0]), sizeof(b));
	}
	for (uint64_t i = 0; i < nV; i++) {
		int64_t col = _vertices[i].color();
		os.write(reinterpret_cast<char *>(&col), sizeof(col));
	}
	for (uint64_t i = 0; i < nT; i++) {
		int64_t col = _tets[i].color();
		os.write(reinterpret_cast<char *>(&col), sizeof(col));
	}
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		int64_t col = _faces[i].color();
		os.write(reinterpret_cast<char *>(&col), sizeof(col));
	}
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		uint64_t flip = _faces[i].flip().idx();
		os.write(reinterpret_cast<char *>(&flip), sizeof(flip));
	}
	for (uint64_t i = 0; i < nV; i++) {
		uint64_t nA = _vertices[i].aliases().size();
		if (nA == 0)
			continue;
		os.write(reinterpret_cast<char *>(&i), sizeof(i));
		os.write(reinterpret_cast<char *>(&nA), sizeof(nA));
		for (std::map<index, index>::const_iterator it = _vertices[i].aliases().begin();
			it != _vertices[i].aliases().end(); ++it)
		{
			uint64_t did, rid;
			did = it->first;
			rid = it->second;
			os.write(reinterpret_cast<char *>(&did), sizeof(did));
			os.write(reinterpret_cast<char *>(&rid), sizeof(rid));
		}
	}
}

void mesh::dump(std::ostream &os) const {
	uint64_t nV, nT, nB, nI;
	uint64_t dom = _domain, doms = _domains;
	double p[3];
	uint64_t v[4];
	uint64_t b[3];

	nV = _vertices.size();
	nT = _tets.size();
	nB = _faces.size() - 4 * nT;
	nI = 0;
	for (uint64_t i = 0; i < nV; i++)
		if (_vertices[i].aliases().size() > 0)
			nI++;

	os << "Mesh dump" << std::endl;
	os << "Domain #" << dom << " of " << doms << std::endl;
	os << "Vertices: " << nV << std::endl;
	os << "Tetras: " << nT << std::endl;
	os << "Boundary faces: " << nB << std::endl;
	os << "Interface vertex: " << nI << std::endl;

	os << "Vertices: " << std::endl;
	for (uint64_t i = 0; i < nV; i++) {
		const vertex &vr = _vertices[i];
		p[0] = vr.r().x;
		p[1] = vr.r().y;
		p[2] = vr.r().z;
		os << i << ". r = (" << p[0] << ", " << p[1] << ", " << p[2] << ")" << std::endl;
	}

	os << "Tetras: " << std::endl;
	for (uint64_t i = 0; i < nT; i++) {
		const tetrahedron &tet = _tets[i];
		for (int j = 0; j < 4; j++)
			v[j] = tet.p(j).idx();
		os << i << ". v = (" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ")" << std::endl;
	}

	os << "Boundary faces: " << std::endl;
	for (uint64_t i = 0; i < nB; i++) {
		const face &f = _faces[4 * nT + i];
		for (int j = 0; j < 3; j++)
			b[j] = f.p(j).idx();
		os << i << ". v = (" << b[0] << ", " << b[1] << ", " << b[2] << ")" << std::endl;
	}

	os << "Vert colors: " << std::endl;
	for (uint64_t i = 0; i < nV; i++) {
		int64_t col = _vertices[i].color();
		os << i << ". " << col << std::endl;
	}

	os << "Tet colors: " << std::endl;
	for (uint64_t i = 0; i < nT; i++) {
		int64_t col = _tets[i].color();
		os << i << ". " << col << std::endl;
	}
	
	os << "Face colors: " << std::endl;
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		int64_t col = _faces[i].color();
		os << i << ". " << col << std::endl;
	}

	os << "Face flips: " << std::endl;
	for (uint64_t i = 0; i < 4 * nT + nB; i++) {
		uint64_t flip = _faces[i].flip().idx();
		os << i << ". " << flip << std::endl;
	}

	os << "Aliases: " << std::endl;
	for (uint64_t i = 0; i < nV; i++) {
		uint64_t nA = _vertices[i].aliases().size();
		if (nA == 0)
			continue;
		os << i << ". ";
		for (std::map<index, index>::const_iterator it = _vertices[i].aliases().begin();
			it != _vertices[i].aliases().end(); ++it)
		{
			uint64_t did, rid;
			did = it->first;
			rid = it->second;
			os << "(" << did << ":" << rid << ") ";
		}
		os << std::endl;
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

	ok &= lastcheck = (_domain < _domains);
	if (!lastcheck)
		log(o, _ + "Domain number " + _domain + " is not less than domain count " + _domains);

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
	vertno = -1;
	tetno = BAD_INDEX;
	wrong = BAD_INDEX;

	for (index i = 0; i < nV; i++) {
		const std::vector<tet_vertex> &vl = _vertices[i].tetrahedrons();
		for (std::vector<tet_vertex>::const_iterator it = vl.begin();
			it != vl.end(); ++it)
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
	vertno = -1;
	faceno = BAD_INDEX;
	wrong = BAD_INDEX;

	for (index i = 0; i < nV; i++) {
		const std::vector<face_vertex> &vl = _vertices[i].faces();
		for (std::vector<face_vertex>::const_iterator it = vl.begin();
			it != vl.end(); ++it)
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
