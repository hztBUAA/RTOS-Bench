#include "platform_macro.h"

#if defined(DONGTU_PLATFORM)

#include <pthread.h>
#include "les.h"


void intewell_stub(void)
{
	LES_stub();
}
#endif

