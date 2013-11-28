#include "common.h"
#include <iostream>
#include <cstdlib>

void mesh3d::assert(bool condition, const std::string message, const std::string file, const int line) {
	if (condition)
		return;

	std::cerr << "Assertation `" << message << "' failed in file " << file << " on line " << line << std::endl;
	abort();
}
