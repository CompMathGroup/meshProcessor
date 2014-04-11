#include "vtk_stream.h"

using namespace mesh3d;

namespace mesh3d {

template<>
void vtk_stream::put<float>(float v) {
	union {
		float w;
		uint32_t i;
		char c[4];
	} x;
	x.w = v;
	x.i = __builtin_bswap32(x.i);
	o.write(&x.c[0], sizeof(x));
}

template<>
void vtk_stream::put<double>(double v) {
	union {
		double w;
		uint64_t i;
		char c[8];
	} x;
	x.w = v;
	x.i = __builtin_bswap64(x.i);
	o.write(&x.c[0], sizeof(x));
}

template<>
void vtk_stream::put<uint32_t>(uint32_t v) {
	union {
		uint32_t i;
		char c[4];
	} x;
	x.i = __builtin_bswap32(v);
	o.write(&x.c[0], sizeof(x));
}

template<>
void vtk_stream::put<int32_t>(int32_t v) {
	union {
		int32_t i;
		char c[4];
	} x;
	x.i = __builtin_bswap32(v);
	o.write(&x.c[0], sizeof(x));
}

template<>
void vtk_stream::put<uint64_t>(uint64_t v) {
	union {
		uint64_t i;
		char c[8];
	} x;
	x.i = __builtin_bswap64(v);
	o.write(&x.c[0], sizeof(x));
}

template<>
void vtk_stream::put<int64_t>(int64_t v) {
	union {
		int64_t i;
		char c[8];
	} x;
	x.i = __builtin_bswap64(v);
	o.write(&x.c[0], sizeof(x));
}

template<>
const std::string vtk_stream::name<float>() const { return "float"; }

template<>
const std::string vtk_stream::name<double>() const { return "double"; }

template<>
const std::string vtk_stream::name<int>() const { return "int"; }

template<>
const std::string vtk_stream::name<unsigned int>() const { return "unsigned_int"; }

}

vtk_stream::vtk_stream(const char *fn) : o(fn, std::ios::out | std::ios::binary) {
	if (!o)
		throw std::invalid_argument("Could not open file `" + std::string(fn) + "'");
	header_written = false;
	cell_data_written = false;
	point_data_written = false;
}

void vtk_stream::write_header(const mesh &m, const std::string &comment) {
	if (header_written)
		throw std::logic_error("Header is already written");

	o 	<< "# vtk DataFile Version 3.0\n"
		<< comment << "\n"
		<< "BINARY\n"
		<< "DATASET UNSTRUCTURED_GRID\n"
		<< "POINTS " << m.vertices().size() << " double" << std::endl;

	const ptr_vector<vertex> &verts = m.vertices();
	for (index i = 0; i < verts.size(); i++) {
		const vertex &v = verts[i];
		put(v.r().x);
		put(v.r().y);
		put(v.r().z);
	}

	const ptr_vector<tetrahedron> &tets = m.tets();
	o << "\nCELLS " << tets.size() << " " << 5 * tets.size() << std::endl;
	for (index i = 0; i < tets.size(); i++) {
		const tetrahedron &t = tets[i];
		uint32_t num_vertex = 4;
		put(num_vertex);
		for (int j = 0; j < 4; j++) {
			uint32_t vidx = t.p(j).idx();
			put(vidx);
		}
	}
	o << "\nCELL_TYPES " << tets.size() << std::endl;
	uint32_t tet_cell_type = 10;
	for (index i = 0; i < tets.size(); i++)
		put(tet_cell_type);

	header_written = true;
	nV = verts.size();
	nT = tets.size();
}
