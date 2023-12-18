#include "minisat/utils/Assert.h"

#include <cstdio>
#include <cstdlib>

void Minisat::AssertionFailure(const char *assertion, const char *function, const char *file, long line,
                               const char *fmt, ...)
{
    fprintf(stderr, "\n~~~ Assertion failed: \"%s\" ~~~\n", assertion);
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fputc('\n', stderr);
    }
    fprintf(stderr, "in function `%s`, file %s, line %ld\n", function, file, line);
    abort();
}
