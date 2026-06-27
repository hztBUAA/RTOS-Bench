/*
 * OneOS dynamic modules are built with C++ workloads but without exception
 * support. Some libstdc++ throw helpers otherwise pull libsupc++ eh_globals.o,
 * which contains dynamic TLS relocations unsupported by the RISC-V module
 * loader. Keep those exceptional paths non-returning and TLS-free.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

extern "C" void *__cxa_get_globals(void)
{
    static unsigned char eh_globals[256];
    return eh_globals;
}

extern "C" void *__cxa_get_globals_fast(void)
{
    return __cxa_get_globals();
}

namespace std {

static void rtbench_oneos_abort(void)
{
    abort();
}

void __throw_bad_exception(void) { rtbench_oneos_abort(); }
void __throw_bad_alloc(void) { rtbench_oneos_abort(); }
void __throw_bad_cast(void) { rtbench_oneos_abort(); }
void __throw_bad_typeid(void) { rtbench_oneos_abort(); }
void __throw_logic_error(char const *) { rtbench_oneos_abort(); }
void __throw_domain_error(char const *) { rtbench_oneos_abort(); }
void __throw_invalid_argument(char const *) { rtbench_oneos_abort(); }
void __throw_length_error(char const *) { rtbench_oneos_abort(); }
void __throw_out_of_range(char const *) { rtbench_oneos_abort(); }
void __throw_out_of_range_fmt(char const *, ...) { rtbench_oneos_abort(); }
void __throw_runtime_error(char const *) { rtbench_oneos_abort(); }
void __throw_range_error(char const *) { rtbench_oneos_abort(); }
void __throw_overflow_error(char const *) { rtbench_oneos_abort(); }
void __throw_underflow_error(char const *) { rtbench_oneos_abort(); }
void __throw_ios_failure(char const *) { rtbench_oneos_abort(); }
void __throw_ios_failure(char const *, int) { rtbench_oneos_abort(); }
void __throw_system_error(int) { rtbench_oneos_abort(); }

} // namespace std
