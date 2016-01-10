#ifndef LOCIC_SUPPORT_ERRORHANDLING_HPP
#define LOCIC_SUPPORT_ERRORHANDLING_HPP

#include <locic/Support/Compiler.hpp>

namespace locic {
	
	/// This function calls abort(), and prints the optional message to stderr.
	/// Use the locic_unreachable macro (that adds location info), instead of
	/// calling this function directly.
	LOCIC_ATTRIBUTE_NORETURN void
	locic_unreachable_internal(const char *msg=nullptr, const char *file=nullptr,
	                           unsigned line=0);
	
}

/// Marks that the current location is not supposed to be reachable.
/// In !NDEBUG builds, prints the message and location info to stderr.
/// In NDEBUG builds, becomes an optimizer hint that the current location
/// is not supposed to be reachable.  On compilers that don't support
/// such hints, prints a reduced message instead.
///
/// Use this instead of assert(0).  It conveys intent more clearly and
/// allows compilers to omit some unnecessary code.
#ifndef NDEBUG
#define locic_unreachable(msg) \
  ::locic::locic_unreachable_internal(msg, __FILE__, __LINE__)
#elif defined(LOCIC_BUILTIN_UNREACHABLE)
#define locic_unreachable(msg) LOCIC_BUILTIN_UNREACHABLE
#else
#define locic_unreachable(msg) ::locic::locic_unreachable_internal()
#endif

#endif