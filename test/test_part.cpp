#include "vtk_stream.h"
#include "graph.h"
#include "mesh.h"
#include <iostream>
#include <cstdio>

using namespace mesh3d;

int main() {
	try {
		const index num_parts = 4;

		std::fstream mf("mesh.m3d", std::ios::in | std::ios::binary);
		mesh m(mf);
		bool res = m.check(&std::cout);
		std::cout << "Mesh check: " << (res ? "OK" : "failed") << std::endl;

		tet_graph tg(m);
		tg.partition(num_parts);

		std::vector<float> u(m.tets().size());
		for (index i = 0; i < m.tets().size(); i++) {
			u[i] = tg.colors(i);
		}
		vtk_stream vtk("mesh.vtk");
		vtk.write_header(m, "Coloring");
		vtk.append_cell_data(u.data(), "u");
		vtk.close();

		for (int domain = 0; domain < num_parts; domain++) {
			char buf[128];
			sprintf(buf, "part%d.vtk", domain);
			mesh part(m, domain, tg);
			res = part.check(&std::cout);
			std::cout << "Part check: " << (res ? "OK" : "failed") << std::endl;

			vtk_stream vtk2(buf);
			vtk2.write_header(part, buf);
			std::vector<float> a(part.vertices().size());
			for (index i = 0; i < part.vertices().size(); i++) {
				a[i] = part.vertices(i).aliases().size();
			}
			vtk2.append_point_data(a.data(), "aliases");
			vtk2.close();

			sprintf(buf, "part%d.m3d", domain);
			std::fstream opf(buf, std::ios::out | std::ios::binary);
			part.serialize(opf);
			opf.close();

			sprintf(buf, "part%d.txt", domain);
			std::fstream tpf(buf, std::ios::out);
			part.dump(tpf);
			tpf.close();
		}
	} catch (std::exception &e) {
		std::cerr << "Exception occured: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
