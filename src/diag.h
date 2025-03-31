#pragma once

#include "config.h"

#ifdef DEBUG
  #include <stdinout.h>
  #define DIAG(format, ...) printf(format, ##__VA_ARGS__)
#else
  #define DIAG(format, ...)
#endif


