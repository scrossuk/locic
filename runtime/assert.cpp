#include <stdio.h>
#include <stdlib.h>

extern "C" void __loci_assert_failed(const char* name) {
	printf("Assertion failed: %s\n", name);
	abort();
}

extern "C" void __loci_unreachable_failed() {
	printf("Unreachable failed!\n");
	abort();
}

