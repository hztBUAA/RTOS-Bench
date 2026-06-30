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
/* Pull in the real <iostream>/<iomanip> so std::cout/cerr/setprecision are
 * already declared before any workload header sees them.  Then redirect both
 * streams to a null streambuf so all output becomes a no-op at runtime.
 *
 * This avoids the "conflicting declaration" error that occurs when this header
 * is injected via -include (i.e. processed before the TU's own #includes) and
 * the TU later includes <iostream> which re-declares the same names with
 * incompatible types.
 *
 * EIGEN_NO_IO is defined by the build system to suppress Eigen's own IO header,
 * but that also disables Eigen's operator<< for Matrix types.  Since we now use
 * the real std::cout (redirected to a null sink at runtime), we need Eigen's
 * operator<< to compile successfully, so undef EIGEN_NO_IO here.
 */
#ifdef EIGEN_NO_IO
#undef EIGEN_NO_IO
#endif

#include <iostream>
#include <iomanip>
#include <streambuf>

namespace rtbench_oneos_detail {

/* A streambuf that silently discards everything written to it. */
class null_streambuf : public std::streambuf {
protected:
    int overflow(int c) override { return c; }
    std::streamsize xsputn(const char *, std::streamsize n) override { return n; }
};

/* One process-wide instance; intentionally never destroyed. */
inline null_streambuf &get_null_buf()
{
    static null_streambuf buf;
    return buf;
}

/* Install the null sink on first use.  Called once per TU via the static
 * initialiser below, but rdbuf() is idempotent so multiple calls are fine. */
inline int install_null_streams()
{
    std::cout.rdbuf(&get_null_buf());
    std::cerr.rdbuf(&get_null_buf());
    return 0;
}

static const int _sink_init = install_null_streams();

} // namespace rtbench_oneos_detail
#endif /* ONEOS_PLATFORM */
