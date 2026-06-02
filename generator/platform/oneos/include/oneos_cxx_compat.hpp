#pragma once

#include <algorithm>
#include <cassert>
#include <limits>

/* Some OneOS BSP headers expose C-style min/max macros. Preload standard C++
 * declarations, then clear the macros before Eigen/ECL headers are parsed. */
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
