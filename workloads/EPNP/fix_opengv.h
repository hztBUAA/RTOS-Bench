#ifndef FIX_OPENGV_H
#define FIX_OPENGV_H

#include <math.h>
#include <cmath>
#include <stdlib.h>

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

// --- Part 3: 补全缺失的 long double 函数 (extern "C") ---
extern "C" {

#ifndef fabsl
static inline long double fabsl(long double x) { return (long double)std::fabs((double)x); }
#endif

#ifndef logl
static inline long double logl(long double x) { return (long double)std::log((double)x); }
#endif

#ifndef expl
static inline long double expl(long double x) { return (long double)std::exp((double)x); }
#endif

#ifndef powl
static inline long double powl(long double x, long double y) { return (long double)std::pow((double)x, (double)y); }
#endif

#ifndef fmodl
static inline long double fmodl(long double x, long double y) { return (long double)std::fmod((double)x, (double)y); }
#endif

#ifndef atan2l
static inline long double atan2l(long double x, long double y) { return (long double)std::atan2((double)x, (double)y); }
#endif

#ifndef ceill
static inline long double ceill(long double x) { return (long double)std::ceil((double)x); }
#endif

#ifndef floorl
static inline long double floorl(long double x) { return (long double)std::floor((double)x); }
#endif

#ifndef cosl
static inline long double cosl(long double x) { return (long double)std::cos((double)x); }
#endif

#ifndef sinl
static inline long double sinl(long double x) { return (long double)std::sin((double)x); }
#endif

#ifndef tanl
static inline long double tanl(long double x) { return (long double)std::tan((double)x); }
#endif

#ifndef acosl
static inline long double acosl(long double x) { return (long double)std::acos((double)x); }
#endif

#ifndef asinl
static inline long double asinl(long double x) { return (long double)std::asin((double)x); }
#endif

#ifndef atanl
static inline long double atanl(long double x) { return (long double)std::atan((double)x); }
#endif

} // extern "C"

#endif // FIX_OPENGV_H