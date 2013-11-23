#include "vol_mesh.h"
#include <iostream>
#include <memory>

using namespace mesh3d;

int main() {
	std::unique_ptr<simple_mesh> sm;
	try {
		sm = std::unique_ptr<simple_mesh>(new vol_mesh("mesh.vol"));
		std::cout << "nV = " << sm->num_vertices() << std::endl;
		std::cout << "nT = " << sm->num_tetrahedrons() << std::endl;
		std::cout << "nB = " << sm->num_bnd_faces() << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Exception occured: " << e.what() << std::endl;
	}
	return 0;
}
