#pragma once

/*
 * Dongtu's RTCore include set and the bundled libstdc++ headers do not agree
 * on include order for _mbstate_t.  Pull wchar.h in before C++ standard
 * headers such as <cstdlib>/<algorithm> need the type through stdlib.h.
 */
#ifdef __cplusplus
#include <wchar.h>
#endif

/*
 * RTCore exposes short U/L helper macros from low-level headers.  Several
 * workload C++ libraries use U and L as ordinary identifiers.
 */
#ifdef U
#undef U
#endif
#ifdef L
#undef L
#endif
#ifdef round_down
#undef round_down
#endif
