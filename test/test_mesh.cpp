#include "vol_mesh.h"
#include "vtk_stream.h"
#include "mesh.h"
#include <iostream>
#include <memory>

using namespace mesh3d;

int main() {
	try {
		vol_mesh vm("mesh.vol");
		mesh m(vm);
		bool res = m.check(&std::cout);
		std::cout << "Mesh check: " << (res ? "OK" : "failed") << std::endl;
		std::fstream f("mesh.m3d", std::ios::binary | std::ios::out);
		m.serialize(f);
		f.close();
		std::vector<float> u(m.vertices().size());
		std::vector<int> vc(m.vertices().size());
		for (index i = 0; i < m.vertices().size(); i++) {
			u[i] = m.vertices(i).r().norm();
			vc[i] = m.vertices(i).color();
		}
		std::vector<vec<double> > w(m.tets().size());
		std::vector<int> tc(m.tets().size());
		for (index i = 0; i < m.tets().size(); i++) {
			const vector &r = m.tets(i).center();
			w[i].x = r.z;
			w[i].y = -r.y;
			w[i].z = r.x;
			tc[i] = m.tets(i).color();
		}
		vtk_stream vtk("mesh.vtk");
		vtk.write_header(m, "Test");
		vtk.append_cell_data(w.data(), "w");
		vtk.append_cell_data(tc.data(), "tet_color");
		vtk.append_point_data(u.data(), "u");
		vtk.append_point_data(vc.data(), "vert_color");
		vtk.close();
	} catch (std::exception &e) {
		std::cerr << "Exception occured: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
