#include "platform_macro.h"

#if defined(RUIHUA_PLATFORM)
#include <reworks_int.h>

#include <pthread.h>
#include "les.h"

void task_switch_hook(thread_t t1, thread_t t2)
{
	LES_stub();
}
#endif
