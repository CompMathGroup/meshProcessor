#include "vector.h"
#include <iostream>

using namespace mesh3d;

int main() {
	vector v(5), e1(1, 0, 0), e2(0, 1, 0), e3(0, 0, 1);

	std::cout << "v = " << v << std::endl;
	std::cout << "e1 = " << e1 << std::endl;
	std::cout << "e2 = " << e2 << std::endl;
	std::cout << "e3 = " << e3 << std::endl;

	v += 2 * e3;

	std::cout << "v = " << v << std::endl;

	double x;

	x = norm2((e1 % e2) - e3);
	std::cout << "||e1 x e2 - e3||^2 = " << v << std::endl;

	if (fabs(x) > 1e-28)
		return 1;

	x = triple(e1, e2, e3);
	std::cout << "(e1, e2, e3) = " << v << std::endl;

	if (fabs(x - 1) > 1e-14)
		return 1;

	return 0;
}
