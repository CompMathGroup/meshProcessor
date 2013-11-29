#include "vol_mesh.h"
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace mesh3d;

/** Parser states */
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

#define HAS_PREFIX(string, prefix) (0 == strncmp((string), (prefix), sizeof(prefix) / sizeof(prefix[0]) - 1))

const std::string getStateString(State st) {
#define STATE_CASE(x) case x: return #x
	switch (st) {
		STATE_CASE(ST_NORM);
		STATE_CASE(ST_SURF_INIT);
		STATE_CASE(ST_SURF_DATA);
		STATE_CASE(ST_VOL_INIT);
		STATE_CASE(ST_VOL_DATA);
		STATE_CASE(ST_PTS_INIT);
		STATE_CASE(ST_PTS_DATA);
		STATE_CASE(ST_CURVE_INIT);
		STATE_CASE(ST_CURVE_DATA);
		STATE_CASE(ST_STOP);
		default: 
		return "(error state)";
	}
#undef STATE_CASE
}

double get_double(char *&p) {
	char *q;
	double ret = strtod(p, &q);
	p = q;
	return ret;
}

int get_int(char *&p) {
	char *q;
	long ret = strtol(p, &q, 10);
	p = q;
	return ret;
}

long long get_ll(char *&p) {
	char *q;
	long long ret = strtoll(p, &q, 10);
	p = q;
	return ret;
}

vol_mesh::vol_mesh(const char *fn) {
	std::ifstream f(fn, std::ios::in);

	if (!f)
		throw std::invalid_argument("Could not read file `" + std::string(fn) +"'");
	
	char buf[1024];
	nV = 0;
	nB = 0;
	nT = 0;
	int iext = 0;
	int i = 0, cnt = 0;

	State state = ST_NORM;

	while (!!f.getline(buf, 1024)) {
		if (buf[0] == '#')
			continue;
		if (state == ST_NORM) {
			if (HAS_PREFIX(buf, "endmesh")) {
				state = ST_STOP;
				break;
			}
			if (HAS_PREFIX(buf, "surfaceelements")) {
				state = ST_SURF_INIT;
				continue;
			}
			if (HAS_PREFIX(buf, "points")) {
				state = ST_PTS_INIT;
				continue;
			}
			if (HAS_PREFIX(buf, "volumeelements")) {
				state = ST_VOL_INIT;
				continue;
			}
			if (HAS_PREFIX(buf, "edgesegmentsgi2")) {
				state = ST_CURVE_INIT;
				continue;
			}
		}
		if (state == ST_PTS_INIT) {
			nV = cnt = atoi(buf);
			vert.resize(3 * nV);
			i = 0;
			state = cnt ? ST_PTS_DATA : ST_NORM;
			continue;
		}
		if (state == ST_VOL_INIT) {
			nT = cnt = atoi(buf);
			tet.resize(4 * nT);
			tetmat.resize(nT);
			i = 0;
			state = cnt ? ST_VOL_DATA : ST_NORM;
			continue;
		}
		if (state == ST_SURF_INIT) {
			nB = cnt = atoi(buf);
			bnd.resize(3 * nB);
			bndmat.resize(nB);
			iext = 0;
			i = 0;
			state = cnt ? ST_SURF_DATA : ST_NORM;
			continue;
		}
		if (state == ST_CURVE_INIT) {
			cnt = atoi(buf);
			i = 0;
			state = cnt ? ST_CURVE_DATA : ST_NORM;
			/* Just ignore it */
			continue;
		};
		if (state == ST_PTS_DATA) {
			// sscanf(buf, "%lf %lf %lf", &vert[3*i], &vert[3*i + 1], &vert[3*i + 2]);
			char *p = buf;
			vert[3*i + 0] = get_double(p);
			vert[3*i + 1] = get_double(p);
			vert[3*i + 2] = get_double(p);
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
		if (state == ST_VOL_DATA) {
			int np;
			// sscanf(buf, "%lld %d %lld %lld %lld %lld", &tetmat[i], &np, 
			// 		&tet[4*i], &tet[4*i + 1], &tet[4*i + 2], &tet[4*i + 3]);
			char *p = buf;
			tetmat[i] = get_ll(p);
			np = get_int(p);
			tet[4*i + 0] = get_ll(p) - 1;
			tet[4*i + 1] = get_ll(p) - 1;
			tet[4*i + 2] = get_ll(p) - 1;
			tet[4*i + 3] = get_ll(p) - 1;
			if (np != 4)
				throw std::domain_error("High-order tetrahedrons not implemented");
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
		if (state == ST_SURF_DATA) {
			int domout, np;
			//sscanf(buf, "%d %lld %d %d %d %lld %lld %lld", &sn, &bndmat[iext], &domin, &domout, &np, 
			//		&bnd[3*iext], &bnd[3*iext + 1], &bnd[3*iext + 2]);
			char *p = buf;
			get_int(p);
			bndmat[iext] = get_ll(p);
			get_int(p);
			domout = get_int(p);
			np = get_int(p);
			bnd[3*iext + 0] = get_ll(p) - 1;
			bnd[3*iext + 1] = get_ll(p) - 1;
			bnd[3*iext + 2] = get_ll(p) - 1;
			if (domout == 0) 
				iext++;
			if (np != 3)
				throw std::domain_error("High-order faces not implemented");
			if (++i == cnt) {
				state = ST_NORM;
				nB = iext;
			}
			continue;
		}
		if (state == ST_CURVE_DATA) {
			if (++i == cnt)
				state = ST_NORM;
			continue;
		}
	}
	f.close();
	if (state != ST_STOP)
		throw std::logic_error("Parse vol file failed: state = `" + 
			getStateString(state) + "' at the end of file");
}

vol_mesh::~vol_mesh() {
}

mesh3d::index vol_mesh::num_vertices() const {
	return nV;
}

mesh3d::index vol_mesh::num_tetrahedrons() const {
	return nT;
}

mesh3d::index vol_mesh::num_bnd_faces() const {
	return nB;
}

const double *vol_mesh::vertex_coord(mesh3d::index i) const {
	return &vert[3 * i];
}

const mesh3d::index *vol_mesh::tet_verts(mesh3d::index i) const {
	return &tet[4 * i];
}
	
const mesh3d::index *vol_mesh::bnd_verts(mesh3d::index i) const {
	return &bnd[3 * i];
}

mesh3d::index vol_mesh::tet_material(mesh3d::index i) const {
	return tetmat[i];
}

mesh3d::index vol_mesh::bnd_material(mesh3d::index i) const {
	return bndmat[i];
}
