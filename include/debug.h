/* (very) simple debug definitions overriden */

#include <stdio.h>

#if defined(DEBUG_LEVEL)
#define dprintf(level, ...) if (level >= DEBUG_LEVEL) printf(__VA_ARGS__)
#else
#define dprintf(level, ...)
#endif
