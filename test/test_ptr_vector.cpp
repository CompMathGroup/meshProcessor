#include "common.h"
#include <iostream>

using namespace mesh3d;

static int allocated;

struct foo {
	int v;
	foo(int v) : v(v) { allocated += sizeof(int); }
	~foo() { allocated -= sizeof(int); }
};

void test() {
	ptr_vector<int> foo(10);

	for (int i = 0; i < 10; i++)
		foo.bind(i, new int(i*i));

	foo.bind(5, new int(100));

	for (int i = 0; i < 10; i++)
		std::cout << foo[i] << std::endl;
}

int main() {
	allocated = 0;

	try {
		test();
	} catch (...) {
		return 1;
	}

	return allocated != 0;
}
