#include <time.h>
#include <limits.h>

unsigned int more_chaotic_seed() {
  time_t now = time(NULL);
  return (now * now * 1103515245 + now * 12345 + 54321) % UINT_MAX;
}

