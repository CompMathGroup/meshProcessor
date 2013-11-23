#include "common.h"
#include <iostream>

using namespace mesh3d;

int main() {
	ptr_vector<int> foo(10);

	for (int i = 0; i < 10; i++)
		foo.bind(i, new int(i*i));

	foo.bind(5, new int(100));

	for (int i = 0; i < 10; i++)
		std::cout << foo[i] << std::endl;
	
	return 0;
}
