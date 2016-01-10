#include <cstdio>
#include <cstdlib>

#include <locic/Support/Compiler.hpp>
#include <locic/Support/ErrorHandling.hpp>

namespace locic {
	
	/// This function calls abort(), and prints the optional message to stderr.
	/// Use the locic_unreachable macro (that adds location info), instead of
	/// calling this function directly.
	void locic_unreachable_internal(const char *msg, const char *file,
	                                unsigned line) {
		if (msg != nullptr) {
			printf("%s\n", msg);
		}
		printf("UNREACHABLE executed");
		if (file != nullptr) {
			printf(" at %s:%u", file, line);
		}
		printf("!\n");
		abort();
#ifdef LOCIC_BUILTIN_UNREACHABLE
		// Windows systems and possibly others don't declare abort() to be noreturn.
		LOCIC_BUILTIN_UNREACHABLE;
#endif
	}
	
}
