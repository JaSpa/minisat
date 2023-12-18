#ifndef Minisat_Assert_h
#define Minisat_Assert_h

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

// Skip all assertions if NDEBUG is defined.
#if defined(NDEBUG)

#    define MINISAT_ASSERT(c, ...) ((void)0)
#    define MINISAT_ASSERT_TEST(c) (1)

#else // defined(NDEBUG)

// Optimized assert check using compiler builtins if available.
#    if !defined(__has_builtin)
#        if __has_builtin(__builtin_expect)
#            define MINISAT_ASSERT_TESTX(c) (__builtin_expect(!(c), 0))
#        endif
#    endif

// Fallback assert check if `__builtin_expect` is not supported.
#    if !defined(MINISAT_ASSERT_TEST)
#        define MINISAT_ASSERT_TEST(c) (!!(c))
#    endif

#    define MINISAT_ASSERT(c, ...)                                                                           \
        (MINISAT_ASSERT_TEST(c)                                                                              \
             ? (void)0                                                                                       \
             : ::Minisat::AssertionFailure(#c, __func__, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__))

#endif // !defined(NDEBUG)

namespace Minisat {

[[noreturn, gnu::format(__printf__, 5, 6)]]
extern void AssertionFailure(const char *assertion, const char *function, const char *file, long line,
                             const char *fmt = nullptr, ...);

} // namespace Minisat

#endif // Minisat_Assert_h
