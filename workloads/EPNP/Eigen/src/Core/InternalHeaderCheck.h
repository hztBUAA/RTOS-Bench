#ifndef EIGEN_CORE_MODULE_H
#error "Please include Eigen/Core instead of including headers inside the src directory directly."
#endif

/* Allow unaligned scalar access on platforms/allocators that don't guarantee Eigen's preferred alignment.
 * This suppresses Eigen's runtime assertion about scalar alignment in Map/Block checks. It's a safe and
 * pragmatic choice for embedded targets where strict SIMD alignment may not be available.
 */
#ifndef EIGEN_ALLOW_UNALIGNED_SCALARS
#define EIGEN_ALLOW_UNALIGNED_SCALARS 1
#endif
