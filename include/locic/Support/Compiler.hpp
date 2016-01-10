#ifndef LOCIC_SUPPORT_COMPILER_HPP
#define LOCIC_SUPPORT_COMPILER_HPP

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/// \macro LOCIC_GNUC_PREREQ
/// \brief Extend the default __GNUC_PREREQ even if glibc's features.h isn't
/// available.
#ifndef LOCIC_GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#  define LOCIC_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= \
     ((maj) << 20) + ((min) << 10) + (patch))
# elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define LOCIC_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
# else
#  define LOCIC_GNUC_PREREQ(maj, min, patch) 0
# endif
#endif

#ifdef __GNUC__
#define LOCIC_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define LOCIC_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define LOCIC_ATTRIBUTE_NORETURN
#endif

/// LOCIC_BUILTIN_UNREACHABLE - On compilers which support it, expands
/// to an expression which states that it is undefined behavior for the
/// compiler to reach this point.  Otherwise is not defined.
#if __has_builtin(__builtin_unreachable) || LOCIC_GNUC_PREREQ(4, 5, 0)
# define LOCIC_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
# define LOCIC_BUILTIN_UNREACHABLE __assume(false)
#endif

#endif