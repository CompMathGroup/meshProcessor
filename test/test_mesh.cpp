#include "vol_mesh.h"
#include "mesh.h"
#include <iostream>
#include <memory>

using namespace mesh3d;

int main() {
	std::unique_ptr<simple_mesh> sm;
	try {
		sm = std::unique_ptr<simple_mesh>(new vol_mesh("mesh.vol"));
		mesh m(*sm.get());
	} catch (std::exception &e) {
		std::cerr << "Exception occured: " << e.what() << std::endl;
	}
	return 0;
}
