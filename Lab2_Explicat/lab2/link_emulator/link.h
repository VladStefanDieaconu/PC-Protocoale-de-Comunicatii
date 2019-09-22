#ifndef LINK
#define LINK
#include "lib.h"

typedef struct {
  msg* m;
  unsigned long long finish_time;
} msg_in_flight;

#endif
