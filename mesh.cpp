#include "mesh.h"

#include "tri_face.h"
#include "tetrahedron.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <cstring>
#include <ctype.h>

#include <ostream>
#include <iterator>
#include <algorithm>

void Mesh::fillData(
	std::vector<std::vector<int> > &vertex2face,
	int nV, int nB, int nT, 
	double *vert, int *bnd, int *tet, 
	int *bndmat, int *tetmat) 
{
	for (int i = 0; i < nV; i++)
		vertices[i] = new Vertex(i, Vector(vert[3*i], vert[3*i+1], vert[3*i+2]));

	for (int i = 0; i < nT; i++) {
		elements[i] = new Tetrahedron(i, 
				vertices[tet[4*i + 0]], vertices[tet[4*i + 1]], 
				vertices[tet[4*i + 2]], vertices[tet[4*i + 3]], 
				tetmat[i]);
		for (int j = 0; j < 4; j++) {
			vertices[tet[4*i+j]]->elems.push_back(
				std::pair<Element *, int>(elements[i], j));

			faces[4*i+j] = static_cast<Tetrahedron *>(elements[i])->f[j];
			for (int k = 0; k < 4; k++)
				if (j != k)
					vertex2face[tet[4*i+k]].push_back(4*i+j);
		}
	}
	for (int i = 0; i < nB; i++) {
		faces[4*nT + i] = new TriFace(4*nT + i, 
			vertices[bnd[3*i+0]], vertices[bnd[3*i+1]], vertices[bnd[3*i+2]], 
			0, bndmat[i]);
		for (int j = 0; j < 3; j++) {
			vertices[bnd[3*i+j]]->bnds.push_back(
				std::pair<Face *, int>(faces[4*nT+i], j)
			);
			vertex2face[bnd[3*i+j]].push_back(4*nT+i);
		}
	}
	for (int i = 0; i < nVert; i++)
		std::sort(vertex2face[i].begin(), vertex2face[i].end());
}

typedef std::vector<int>::iterator iter_t;

int three_way_find_except(iter_t x, iter_t y, iter_t z, int exc) 
{
	int a = *x, b = *y, c = *z;
	while (true) {
		if (a == b && b == c) {
			if (a != exc)
				return a;
		}
		if (a < b) {
			if (a < c)
				a = *x++;
			else 
				c = *z++;
		} else {
			if (b < c)
				b = *y++;
			else
				c = *z++;
		}
	}
}

void Mesh::fromVol(
	std::ostream &log,
	int nV, int nB, int nT, 
	double *vert, int *bnd, int *tet, 
	int *bndmat, int *tetmat
	)
{
	nVert = nV;
	nElems = nT;
	nFaces = 4*nT + nB;

	vertices.resize(nVert);
	elements.resize(nElems);
	faces.resize(nFaces);

	std::vector<std::vector<int> > vertex2face(nV);
	fillData(vertex2face, nV, nB, nT, vert, bnd, tet, bndmat, tetmat);
	
	int bnd_idx = 0;
	for (int i = 0; i < nFaces; i++) {
		TriFace *f = static_cast<TriFace *>(faces[i]);
		Vertex *p1 = f->p[0], *p2 = f->p[1], *p3 = f->p[2];
		int i1 = p1->index;
		int i2 = p2->index;
		int i3 = p3->index;

		int found = -1;
		typedef std::vector<int>::const_iterator iter_t;
		std::vector<int> 
			&v1 = vertex2face[i1],
			&v2 = vertex2face[i2],
			&v3 = vertex2face[i3];

		found = three_way_find_except(
			v1.begin(), v2.begin(), v3.begin(), i);
		
		if (found == -1) {
			throw "Flipped face not found";
		}
		f->setFlip(faces[found]);

		if (!f->element) 
			f->bnd_index = bnd_idx++;
		else
			f->bnd_index = -1;
	}
	nBndFaces = bnd_idx;
}

enum State {
	ST_NORM,
	ST_SURF_INIT,
	ST_SURF_DATA,
	ST_VOL_INIT,
	ST_VOL_DATA,
	ST_PTS_INIT,
	ST_PTS_DATA,
	ST_CURVE_INIT,
	ST_CURVE_DATA,
	ST_STOP,
}; 

const char *getStateString(State st) {
#define state_case(x) case x: return #x
	switch (st) {
		state_case(ST_NORM);
		state_case(ST_SURF_INIT);
		state_case(ST_SURF_DATA);
		state_case(ST_VOL_INIT);
		state_case(ST_VOL_DATA);
		state_case(ST_PTS_INIT);
		state_case(ST_PTS_DATA);
		state_case(ST_CURVE_INIT);
		state_case(ST_CURVE_DATA);
		state_case(ST_STOP);
		default: 
			return "(error state)";
	}
#undef state_case
}

Mesh::Mesh(std::ostream &log, const char *fn) {
	int i = 0, cnt = 0;
	State state = ST_NORM;

	log << "Reading mesh from `" << fn << "'\n";

	std::ifstream f(fn, std::ios::in);
	if (!f) {
		log << "Could not open file\n";
		throw 0;
	}
	
	char buf[1024];
	int nV = 0, nB = 0, nBx = 0, nT = 0;
	double *vert = 0;
	int *bnd = 0, *tet = 0, *bndmat = 0, *tetmat = 0;

	while (!!f.getline(buf, 1024)) {
		/* printf("[%.20s] %s", getStateString(state), buf); */
		if (buf[0]=='#')
			continue;
		if (state == ST_NORM) {
			if (!strncmp(buf, "endmesh", 7)) {
				state = ST_STOP;
				break;
			}
			if (!strncmp(buf, "surfaceelements", 15)) {
				state = ST_SURF_INIT;
				continue;
			}
			if (!strncmp(buf, "points", 6)) {
				state = ST_PTS_INIT;
				continue;
			}
			if (!strncmp(buf, "volumeelements", 14)) {
				state = ST_VOL_INIT;
				continue;
			}
			if (!strncmp(buf, "edgesegmentsgi2", 15)) {
				state = ST_CURVE_INIT;
				continue;
			}
		}
		if (state == ST_PTS_INIT) {
			nV = cnt = atoi(buf);
			vert = new double [3*nV];
			i = 0;
			state = cnt?ST_PTS_DATA:ST_NORM;
			continue;
		}
		if (state == ST_VOL_INIT) {
			nT = cnt = atoi(buf);
			tet = new int [4*nT];
			tetmat = new int [nT];
			i = 0;
			state = cnt?ST_VOL_DATA:ST_NORM;
			continue;
		}
		if (state == ST_SURF_INIT) {
			nB = cnt = atoi(buf);
			bnd = new int [3*nB];
			bndmat = new int [nB];
			nBx = 0;
			i = 0;
			state = cnt?ST_SURF_DATA:ST_NORM;
			continue;
		}
		if (state == ST_CURVE_INIT) {
			cnt = atoi(buf);
			i = 0;
			state = cnt?ST_CURVE_DATA:ST_NORM;
			/* Just ignore it */
			continue;
		};
		if (state == ST_PTS_DATA) {
			sscanf(buf, "%lf %lf %lf", vert + 3*i, vert + 3*i + 1, vert + 3*i + 2);
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
		if (state == ST_VOL_DATA) {
			int np;
			sscanf(buf, "%d %d %d %d %d %d", tetmat + i, &np, 
				tet + 4*i, tet + 4*i + 1, tet + 4*i + 2, tet + 4*i + 3);
			tet[4*i]--;
			tet[4*i+1]--;
			tet[4*i+2]--;
			tet[4*i+3]--;
			if (np != 4) {
				log << "high-order tetrahedrons not implemented" << std::endl;
			}
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
		if (state == ST_SURF_DATA) {
			int sn, domin, domout, np;
			sscanf(buf, "%d %d %d %d %d %d %d %d", &sn, bndmat + nBx, &domin, &domout, &np, 
				bnd + 3*nBx, bnd + 3*nBx + 1, bnd + 3*nBx + 2);
			bnd[3*nBx]--;
			bnd[3*nBx+1]--;
			bnd[3*nBx+2]--;
			if (domout == 0) 
				nBx ++;
			if (np != 3) {
				log << "high-order faces not implemented" << std::endl;
			}
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
		if (state == ST_CURVE_DATA) {
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
	}
	f.close();
	if (state == ST_STOP) {
		fromVol(log, nV, nBx, nT, vert, bnd, tet, bndmat, tetmat);
	} else {
		log << "Parse vol file failed: state = `" <<
			getStateString(state) << "' at the end" << std::endl;
		throw 0;
		/* Set Mesh object to valid state so ~Mesh() could safely destroy it */
	}
	if (vert) delete[] vert;
	if (bnd) delete[] bnd;
	if (bndmat) delete[] bndmat;
	if (tet) delete[] tet;
	if (tetmat) delete[] tetmat;
}

#define REALVEC(x,i) ((realbytes == 4) ? x.s[i] : x.d[i])

char *swap(int *v) {
	union {
		char p[4];
		int w;
	} x;
	x.w = *v;
	char s;
	s = x.p[0]; x.p[0] = x.p[3]; x.p[3] = s;
	s = x.p[1]; x.p[1] = x.p[2]; x.p[2] = s;
	*v = x.w;
	return (char *)v;
}

char *swap(float *v) {
	union {
		char p[4];
		float w;
	} x;
	x.w = *v;
	char s;
	s = x.p[0]; x.p[0] = x.p[3]; x.p[3] = s;
	s = x.p[1]; x.p[1] = x.p[2]; x.p[2] = s;
	*v = x.w;
	return (char *)v;
}

char *swap(double *v) {
	union {
		char p[8];
		double w;
	} x;
	x.w = *v;
	char s;
	s = x.p[0]; x.p[0] = x.p[7]; x.p[7] = s;
	s = x.p[1]; x.p[1] = x.p[6]; x.p[6] = s;
	s = x.p[2]; x.p[2] = x.p[5]; x.p[5] = s;
	s = x.p[3]; x.p[3] = x.p[4]; x.p[4] = s;
	*v = x.w;
	return (char *)v;
}

void Mesh::saveVtk(
	std::ostream &log, 
	const char *fn, int realbytes, 
	const char *cellfmt, const char *pointfmt, ...) const 
{
	typedef union {
		double *d;
		float *s;
	} realptr;

	va_list args;
	va_start(args, pointfmt);
	char id[1024];

	std::ofstream f(fn, std::ios::binary | std::ios::out);
	if (!f) {
		log << "error opening file `" << fn << "'\n";
		throw 1;
	}
	f << "# vtk DataFile Version 3.0\n"
	  << "Generated output\n"
	  << "BINARY\n"
	  << "DATASET UNSTRUCTURED_GRID\n"
	  << "POINTS " << nVert << " double" << std::endl;
	for (int i=0; i<nVert; i++) {
		double x = vertices[i]->r.x;
		double y = vertices[i]->r.y;
		double z = vertices[i]->r.z;
		f	.write(swap(&x), sizeof(double))
			.write(swap(&y), sizeof(double))
			.write(swap(&z), sizeof(double));
	}
	f << "\nCELLS " << nElems << " " << 5*nElems << std::endl;
	for (int i=0; i<nElems; i++) {
		if (elements[i]->type != EL_TETRAHEDRON)
			log << "Unsupported element type. File will be corrupted" 
				<< std::endl;
		Tetrahedron *el = static_cast<Tetrahedron *>(elements[i]);
		int g = 4;
		f.write(swap(&g), sizeof(int));
		for (int j = 0; j < 4; j++) {
			int idx = el->p[0]->index;
			f.write(swap(&idx), sizeof(int));
		}
	}
	f << "\nCELL_TYPES " << nElems << std::endl;
	int ct = 10;
	char *sct = swap(&ct);
	for (int i=0; i<nElems; i++) 
		f.write(sct, sizeof(int));

	f << "\nCELL_DATA " << nElems << std::endl;
	const char *p = cellfmt;
	char *q = id;

	while (*p) {
		if (*p == '%') {
			p++;
			if (!*p)
				break;
			if (*p == 's') {
				realptr data = va_arg(args, realptr);
				*q = 0;
				f	<< "\nSCALARS " << id << " double\n"
					<<"LOOKUP_TABLE default" << std::endl;
				for (int i=0; i<nElems; i++) {
					double v = REALVEC(data, i);
					f.write(swap(&v), sizeof(double));
				}
				q = id;
				p++;
				continue;
			}
			if (*p == 'v') {
				realptr d1 = va_arg(args, realptr);
				realptr d2 = va_arg(args, realptr);
				realptr d3 = va_arg(args, realptr);
				*q = 0;
				f << "\nVECTORS " << id << " double" << std::endl;
				for (int i=0; i<nElems; i++) {
					double vx = REALVEC(d1, i);
					double vy = REALVEC(d2, i);
					double vz = REALVEC(d3, i);
					f	.write(swap(&vx), sizeof(double))
						.write(swap(&vy), sizeof(double))
						.write(swap(&vz), sizeof(double));
				}
				q = id;
				p++;
				continue;
			}
			if (*p == 't') {
				realptr dxx = va_arg(args, realptr);
				realptr dxy = va_arg(args, realptr);
				realptr dxz = va_arg(args, realptr);
				realptr dyy = va_arg(args, realptr);
				realptr dyz = va_arg(args, realptr);
				realptr dzz = va_arg(args, realptr);
				*q = 0;
				f << "\nTENSORS " << id << " double" << std::endl;
				for (int i=0; i<nElems; i++) {
					double vxx = REALVEC(dxx, i);
					double vxy = REALVEC(dxy, i);
					double vxz = REALVEC(dxz, i);
					double vyy = REALVEC(dyy, i);
					double vyz = REALVEC(dyz, i);
					double vzz = REALVEC(dzz, i);
					size_t sd = sizeof(double);
					f	
		.write(swap(&vxx), sd).write(swap(&vxy), sd).write(swap(&vxz), sd)
		.write(swap(&vxy), sd).write(swap(&vyy), sd).write(swap(&vyz), sd)
		.write(swap(&vxz), sd).write(swap(&vyz), sd).write(swap(&vzz), sd);
				}
				q = id;
				p++;
				continue;
			}
			if (!isgraph(*p)) {
				*q = '_';
				q++;
				p++;
				continue;
			}
		}
		*q = *p;
		p++;
		q++;
	}

	f << "\nPOINT_DATA " << nVert << std::endl;

	p = pointfmt;
	q = id;

	while (*p) {
		if (*p == '%') {
			p++;
			if (!*p)
				break;
			if (*p == 's') {
				realptr data = va_arg(args, realptr);
				*q = 0;
				f	<< "\nSCALARS " << id << " double\n"
					<<"LOOKUP_TABLE default" << std::endl;
				for (int i=0; i<nVert; i++) {
					double v = REALVEC(data, i);
					f.write(swap(&v), sizeof(double));
				}
				q = id;
				p++;
				continue;
			}
			if (*p == 'v') {
				realptr d1 = va_arg(args, realptr);
				realptr d2 = va_arg(args, realptr);
				realptr d3 = va_arg(args, realptr);
				*q = 0;
				f << "\nVECTORS " << id << " double" << std::endl;
				for (int i=0; i<nVert; i++) {
					double vx = REALVEC(d1, i);
					double vy = REALVEC(d2, i);
					double vz = REALVEC(d3, i);
					f	.write(swap(&vx), sizeof(double))
						.write(swap(&vy), sizeof(double))
						.write(swap(&vz), sizeof(double));
				}
				q = id;
				p++;
				continue;
			}
			if (*p == 't') {
				realptr dxx = va_arg(args, realptr);
				realptr dxy = va_arg(args, realptr);
				realptr dxz = va_arg(args, realptr);
				realptr dyy = va_arg(args, realptr);
				realptr dyz = va_arg(args, realptr);
				realptr dzz = va_arg(args, realptr);
				*q = 0;
				f << "\nTENSORS " << id << " double" << std::endl;
				for (int i=0; i<nVert; i++) {
					double vxx = REALVEC(dxx, i);
					double vxy = REALVEC(dxy, i);
					double vxz = REALVEC(dxz, i);
					double vyy = REALVEC(dyy, i);
					double vyz = REALVEC(dyz, i);
					double vzz = REALVEC(dzz, i);
					size_t sd = sizeof(double);
					f	
		.write(swap(&vxx), sd).write(swap(&vxy), sd).write(swap(&vxz), sd)
		.write(swap(&vxy), sd).write(swap(&vyy), sd).write(swap(&vyz), sd)
		.write(swap(&vxz), sd).write(swap(&vyz), sd).write(swap(&vzz), sd);
				}
				q = id;
				p++;
				continue;
			}
			if (!isgraph(*p)) {
				*q = '_';
				q++;
				p++;
				continue;
			}
		}
		*q = *p;
		p++;
		q++;
	}
	f.close();
}

Mesh::~Mesh() {
	for (int i = 0; i < nVert; i++)
		delete vertices[i];
	for (int i = 0; i < nElems; i++)
		delete elements[i];
	for (int i = 0; i < nFaces; i++)
		delete faces[i];
}

double Mesh::quality() const {
	double qual = 1, r;
	for (int i=0; i<nElems; i++) {
		if ((r = elements[i]->quality) < qual) 
			qual = r;
	}
	return qual;
}

bool Mesh::checkVertexIndex(int &wrong) const {
	for (int i=0; i<nVert; i++) 
		if (vertices[i]->index != i) {
			wrong = i;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkFaceIndex(int &wrong) const {
	for (int i=0; i<nFaces; i++) 
		if (faces[i]->index != i) {
			wrong = i;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkFlippedFace(int &wrong) const {
	for (int i=0; i<nFaces; i++) 
		if (faces[i]->flip->flip != faces[i]) {
			wrong = i;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkFlippedOrient(int &wrong) const {
	for (int i=0; i<nFaces; i++) {
		double cosv = faces[i]->flip->normal.dot(faces[i]->normal);
		if (std::fabs(cosv + 1) > 1e-10) {
			wrong = i;
			return false;
		}
	}
	wrong = -1;
	return true;
}

bool Mesh::checkFaceBoundaryCondidion(int &wrong, bool &isOuter) const {
	for (int i=0; i<nFaces; i++)
		if ((faces[i]->element == 0) ^ (faces[i]->borderType != -1)) {
			wrong = i;
			isOuter = !faces[i]->element;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkFaceSurface(int &wrong) const {
	for (int i=0; i<nFaces; i++)
		if (faces[i]->surface <= 0) {
			wrong = i;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkElementVolume(int &wrong) const {
	for (int i=0; i<nElems; i++)
		if (elements[i]->volume <= 0) {
			wrong = i;
			return false;
		}
	wrong = -1;
	return true;
}

bool Mesh::checkElementFaceNormals(int &wrong, int &faceno) const {
/*
	Element faces should be directed inside, so boundary faces 
	could be directed outside.
	Check specific to tetrahedron meshes
*/

	for (int i=0; i<nElems; i++) {
		if (elements[i]->type != EL_TETRAHEDRON)
			continue;

		Tetrahedron *t = static_cast<Tetrahedron *>(elements[i]);
		for (int j=0; j<4; j++) {
			int k = (j+1) & 3; 
			Vector r(t->p[j]->r);
			r.sub(t->p[k]->r);
			r.scale(t->f[j]->surface/3);
			double fv = r.dot(t->f[j]->normal);
			if (std::fabs(fv - t->volume) > 1e-12 * std::fabs(t->volume)) {
				wrong = i;
				faceno = j;
				return false;
			}
		}
	}
	wrong = -1;
	faceno = -1;
	return true;
}

bool Mesh::checkVertexBoundaryList(
	int &wrong, int &faceno, int &vertno, int &cnt) const
{
	bool check, ret = true;
	cnt = 0;
	wrong = faceno = vertno = -1;
	for (int i=0; i<nVert; i++) {
		typedef std::vector<std::pair<Face *, int> >
			::const_iterator iter_t;
		iter_t p = vertices[i]->bnds.begin();
		for (;p != vertices[i]->bnds.end(); p++) {
			cnt++;
			if (p->first->type != FC_TRIANGLE)
				continue;
			TriFace *fc = static_cast<TriFace *>(p->first);
			check = fc->p[p->second]->index == i;
			if (!check) {
				wrong = i;
				faceno = fc->index;
				vertno = p->second;
				ret = false;
			}
		}
	}
	return ret;
}

bool Mesh::checkVertexElementsList(
	int &wrong, int &tetno, int &vertno, int &cnt) const
{
	bool check, ret = true;
	cnt = 0;
	wrong = tetno = vertno = -1;
	for (int i=0; i<nVert; i++) {
		typedef std::vector<std::pair<Element *, int> >
			::const_iterator iter_t;
		iter_t p = vertices[i]->elems.begin();
		for (;p != vertices[i]->elems.end(); p++) {
			cnt++;
			if (p->first->type != EL_TETRAHEDRON)
				continue;
			Tetrahedron *tet = static_cast<Tetrahedron *>(p->first);
			check = tet->p[p->second]->index == i;
			if (!check) {
				wrong = i;
				tetno = tet->index;
				vertno = p->second;
				ret = false;
			}
		}
	}
	return ret;
}

bool Mesh::check(std::ostream &log) const {
	bool ok = true, lastcheck;
	int wrong;

	ok &= lastcheck = checkVertexIndex(wrong);
	if (!lastcheck)
		log << "Vertex index is corrupted at vertex #" << wrong << std::endl; 

	ok &= lastcheck = checkFaceIndex(wrong);
	if (!lastcheck)
		log << "Face index is corrupted at face #" << wrong << std::endl;

	ok &= lastcheck = checkFlippedFace(wrong);
	if (!lastcheck)
		log << "Face " << wrong
			<< " twice filpped is not the same" << std::endl;

	ok &= lastcheck = checkFlippedOrient(wrong);
	if (!lastcheck)
		log << "Face flip not opposed to face" 
			<<" (probably wrong oriented face)" << std::endl;

	bool isOuter;
	ok &= lastcheck = checkFaceBoundaryCondidion(wrong, isOuter);
	if (!lastcheck)
		log << (isOuter ? "Outer" : "Inner") << " face #" << wrong
			<< (isOuter ? " doesn't have" : " has") << " boundary condition"
			<< std::endl;

	ok &= lastcheck = checkFaceSurface(wrong);
	if (!lastcheck)
		log << "Face with #" << wrong << "has negative surface  = " 
			<< faces[wrong]->surface << std::endl;

	ok &= lastcheck = checkElementVolume(wrong);
	if (!lastcheck)
		log << "Element with #" << wrong << "has negative volume  = " 
			<< elements[wrong]->volume << std::endl;

	int faceno;
	ok &= lastcheck = checkElementFaceNormals(wrong, faceno);
	if (!lastcheck)
		log << "Element #" << wrong << " has " << faceno << "-th face "
			<< "wrongly oriented" << std::endl;

	int tetno, vertno, cnt;
	ok &= lastcheck = checkVertexElementsList(wrong, tetno, vertno, cnt);
	if (!lastcheck)
		log << "Element #" << tetno << " has " << vertno << "-th vertex "
			<< "incorrectly listed in vertex #" << wrong << " elems list"
			<< std::endl;

	lastcheck = (cnt == 4*nElems);
	ok &= lastcheck;
	if (!lastcheck)
		log << "Incidental total elems list has wrong size " 
			<< cnt << " vs" << 4*nElems 
			<< std::endl;

	ok &= lastcheck = checkVertexBoundaryList(wrong, faceno, vertno, cnt);
	if (!lastcheck)
		log << "Boundary face #" << faceno << " has " 
			<< vertno << "-th vertex "
			<< "incorrectly listed in vertex #" << wrong << " bnds list"
			<< std::endl;

	lastcheck = (cnt == 3*(nFaces - 4*nElems));
	ok &= lastcheck;
	if (!lastcheck)
		log << "Incidental total boundary faces list has wrong size " 
			<< cnt << " vs " << 3*(nFaces - 4*nElems)
			<< std::endl;

	return ok;
}
