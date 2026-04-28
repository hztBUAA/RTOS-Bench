#ifndef FIX_OPENGV_H
#define FIX_OPENGV_H

/*
 * Intewell's C++ <cmath> only exposes C99 math names such as std::log2,
 * std::expm1, and std::fma when these libstdc++ feature macros are set before
 * <cmath> is included. The Eclipse make backend does not pass the SConscript
 * defines, so keep them here as well.
 */
#ifndef _GLIBCXX_USE_C99_MATH
#define _GLIBCXX_USE_C99_MATH 1
#endif
#ifndef __riscv
#ifndef _GLIBCXX_USE_C99_MATH_TR1
#define _GLIBCXX_USE_C99_MATH_TR1 1
#endif
#endif

#include <math.h>
#include <cmath>
#include <stdlib.h>

#ifdef U
#undef U
#endif
#ifdef L
#undef L
#endif
#ifdef round_down
#undef round_down
#endif

// --- Part 1: 补全 std 命名空间缺失的数学函数 ---
namespace std {
    using ::isnan;
    using ::isfinite;
    using ::isinf;
    using ::log2;
    using ::round;
    using ::trunc;
    // using ::fma; // 不要引入系统的 fma
    using ::exp2;
    using ::expm1;
    using ::log1p;
    using ::rint;
    using ::cbrt;
    using ::sqrt;
    using ::fabs;
    using ::atan2;
    using ::cos;
    using ::sin;
    using ::pow;

    // 手动提供 fma 模板实现
    template<typename T>
    inline T fma(T x, T y, T z) {
        return x * y + z;
    }
}

// --- Part 2: 补全缺失的宏 (解决 random_generators.cpp 报错) ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef FIX_OPENGV_USE_STUB_LDOUBLE

// --- Part 3: 补全缺失的 long double 函数 (extern "C") ---
extern "C" {
static inline long double fabsl(long double x) { return (long double)std::fabs((double)x); }
static inline long double logl(long double x) { return (long double)std::log((double)x); }
static inline long double expl(long double x) { return (long double)std::exp((double)x); }
static inline long double powl(long double x, long double y) { return (long double)std::pow((double)x, (double)y); }
static inline long double fmodl(long double x, long double y) { return (long double)std::fmod((double)x, (double)y); }
static inline long double atan2l(long double x, long double y) { return (long double)std::atan2((double)x, (double)y); }
static inline long double ceill(long double x) { return (long double)std::ceil((double)x); }
static inline long double floorl(long double x) { return (long double)std::floor((double)x); }
static inline long double cosl(long double x) { return (long double)std::cos((double)x); }
static inline long double sinl(long double x) { return (long double)std::sin((double)x); }
static inline long double tanl(long double x) { return (long double)std::tan((double)x); }
static inline long double acosl(long double x) { return (long double)std::acos((double)x); }
static inline long double asinl(long double x) { return (long double)std::asin((double)x); }
static inline long double atanl(long double x) { return (long double)std::atan((double)x); }
} // extern "C"
#endif
#endif // FIX_OPENGV_H
