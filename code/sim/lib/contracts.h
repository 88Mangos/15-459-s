#include <assert.h>

#ifdef REQUIRES
#undef REQUIRES
#endif

#ifdef ENSURES
#undef ENSURES
#endif

#define ASSERT(COND) assert(COND)
#define REQUIRES(COND) assert(COND)
#define ENSURES(COND) assert(COND)
