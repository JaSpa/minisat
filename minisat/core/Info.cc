#include "minisat/core/Info.h"

const char *const MINISAT_VERSION_INFO =
    "MiniSat [assumptions-at-once, trail-savings] "
#ifdef NDEBUG
    "(assertions off)"
#else
    "(assertions on)"
#endif
    ;
