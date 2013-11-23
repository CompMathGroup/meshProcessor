#include "vector.h"
#include <cmath>

using namespace mesh3d;

vector::norm() {
	return sqrt(norm2());
}
