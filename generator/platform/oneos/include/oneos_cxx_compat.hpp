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

#ifdef ONEOS_PLATFORM
namespace std {

struct rtbench_oneos_ostream {
    template <typename T>
    rtbench_oneos_ostream &operator<<(const T &)
    {
        return *this;
    }

    rtbench_oneos_ostream &operator<<(rtbench_oneos_ostream &(*fn)(rtbench_oneos_ostream &))
    {
        return fn(*this);
    }
};

struct rtbench_oneos_setprecision {
    int value;
};

static rtbench_oneos_ostream cout;
static rtbench_oneos_ostream cerr;

static inline rtbench_oneos_ostream &endl(rtbench_oneos_ostream &os)
{
    return os;
}

static inline rtbench_oneos_setprecision setprecision(int value)
{
    rtbench_oneos_setprecision precision = {value};
    return precision;
}

} // namespace std
#endif
