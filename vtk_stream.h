#ifndef __MESH3D__VTK_STREAM_H__
#define __MESH3D__VTK_STREAM_H__

#include <fstream>
#include "common.h"
#include "mesh.h"
#include <stdexcept>
#include <stdint.h>

namespace mesh3d {

/** A class for exporting mesh and accompanying data to vtk file */
class vtk_stream {
	std::ofstream o;
	bool header_written;
	bool cell_data_written;
	bool point_data_written;
	index nV, nT;

	template <class T>
	void put(T v);

	template <class T>
	const std::string name() const;

public:
	vtk_stream(const char *fn);
	void write_header(const mesh &m, const std::string &comment = "comment");

	template <class T>
	void append_cell_data(const T *v, const std::string &id);

	template <class T>
	void append_point_data(const T *v, const std::string &id);

	template <class T>
	void append_cell_data(const vec<T> *v, const std::string &id);

	template <class T>
	void append_point_data(const vec<T> *v, const std::string &id);

	void close() {
		if (!cell_data_written) {
			cell_data_written = true;
			o << "\nCELL_DATA " << nT;
		}
		if (!point_data_written) {
			point_data_written = true;
			o << "\nPOINT_DATA " << nV;
		}
		o.close();
	}

	~vtk_stream() {
		close();
	}
};

template <class T>
void vtk_stream::append_cell_data(const T *v, const std::string &id) {
	if (!header_written)
		throw std::logic_error("Write header first");
	if (point_data_written)
		throw std::logic_error("All cell data should be written prior to point data");
	if (!cell_data_written) {
		cell_data_written = true;
		o << "\nCELL_DATA " << nT;
	}
	o	<< "\nSCALARS " << id << " " << name<T>()
		<< "\nLOOKUP_TABLE default" << std::endl;
	for (index i = 0; i < nT; i++)
		put(v[i]);
}

template <class T>
void vtk_stream::append_cell_data(const vec<T> *v, const std::string &id) {
	if (!header_written)
		throw std::logic_error("Write header first");
	if (point_data_written)
		throw std::logic_error("All cell data should be written prior to point data");
	if (!cell_data_written) {
		cell_data_written = true;
		o << "\nCELL_DATA " << nT;
	}
	o	<< "\nVECTORS " << id << " " << name<T>() << std::endl;
	for (index i = 0; i < nT; i++) {
		put(v[i].x);
		put(v[i].y);
		put(v[i].z);
	}
}

template <class T>
void vtk_stream::append_point_data(const T *v, const std::string &id) {
	if (!header_written)
		throw std::logic_error("Write header first");
	if (!cell_data_written) {
		cell_data_written = true;
		o << "\nCELL_DATA " << nT;
	}
	if (!point_data_written) {
		point_data_written = true;
		o << "\nPOINT_DATA " << nV;
	}
	o	<< "\nSCALARS " << id << " " << name<T>()
		<< "\nLOOKUP_TABLE default" << std::endl;
	for (index i = 0; i < nV; i++)
		put(v[i]);
}

template <class T>
void vtk_stream::append_point_data(const vec<T> *v, const std::string &id) {
	if (!header_written)
		throw std::logic_error("Write header first");
	if (!cell_data_written) {
		cell_data_written = true;
		o << "\nCELL_DATA " << nT;
	}
	if (!point_data_written) {
		point_data_written = true;
		o << "\nPOINT_DATA " << nV;
	}
	o	<< "\nVECTORS " << id << " " << name<T>() << std::endl;
	for (index i = 0; i < nV; i++) {
		put(v[i].x);
		put(v[i].y);
		put(v[i].z);
	}
}

}

#endif
