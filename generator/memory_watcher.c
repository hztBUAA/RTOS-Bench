/** @file memory_watcher.c
 * @ingroup generator
 * @brief Memory watcher (Linux) with RT-Thread/SylixOS stubs.
 */

#include "logging.h"

#if defined(RT_THREAD_PLATFORM) || defined(SYLIXOS_PLATFORM) ||               \
	defined(ONEOS_PLATFORM) || defined(DONGTU_PLATFORM) ||                    \
	defined(RUIHUA_PLATFORM)

/* RT-Thread/SylixOS stub: not supported, keep no-op to satisfy links. */
void start_memory_watcher(size_t bytes_to_preallocate)
{
	(void)bytes_to_preallocate;
	elogf(LOG_LEVEL_TRACE,
	      "Memory watcher is not supported on this platform, skipping.\n");
}

void stop_memory_watcher(void)
{
}

#else /* Linux/Posix implementation with --wrap support */

#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

enum memory_watcher_states {
	MEMORY_WATCHER_DISABLED = 0,
	MEMORY_WATCHER_ENABLED
};

static enum memory_watcher_states memory_watcher_status =
	MEMORY_WATCHER_DISABLED;
static const void *initial_program_break = NULL;

void start_memory_watcher(size_t bytes_to_preallocate)
{
	int res;
	void *dummy_alloc = NULL;
	if (bytes_to_preallocate > 0) {
		if (memory_watcher_status == MEMORY_WATCHER_DISABLED) {
			elogf(LOG_LEVEL_TRACE,
			      "Starting  memory watcher, bytes to preallocate: %zu.\n",
			      bytes_to_preallocate);
			res = mallopt(M_TOP_PAD, bytes_to_preallocate);
			if (res == 0) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot preallocate %zu bytes.\n",
				      bytes_to_preallocate);
				exit(-1);
			}
			res = mallopt(M_MMAP_MAX, 0);
			if (res == 0) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot disable mmap based allocation.\n");
				exit(-1);
			}
			dummy_alloc = malloc(bytes_to_preallocate);
			if (dummy_alloc == NULL) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot allocate dynamic memory, aborting.\n");
				exit(-1);
			}
			free(dummy_alloc);
			initial_program_break = sbrk(0);
			if (initial_program_break == (void *)-1) {
				perror("Cannot find the program break during memory watcher setup.");
				exit(-1);
			}
			memory_watcher_status = MEMORY_WATCHER_ENABLED;
			elogf(LOG_LEVEL_TRACE,
			      "Memory watcher enabled, initial program break:%p.\n",
			      initial_program_break);
		} else {
			elogf(LOG_LEVEL_ERR,
			      "Attempt to configure the memory watcher when it's already started.\n");
			exit(-1);
		}
	}
}

void stop_memory_watcher()
{
	int res;
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		elogf(LOG_LEVEL_TRACE, "Stopping memory watcher.\n");
		memory_watcher_status = MEMORY_WATCHER_DISABLED;
		res = mallopt(M_TOP_PAD, 128 * 1024);
		if (res == 0) {
			elogf(LOG_LEVEL_ERR,
			      "Cannot reset M_TOP_PAD, after stopping memory watcher.\n");
			exit(-1);
		}
		res = mallopt(M_MMAP_MAX, 65536);
		if (res == 0) {
			elogf(LOG_LEVEL_ERR,
			      "Cannot enable mmap based allocation after stopping memory watcher.\n");
			exit(-1);
		}
	} else {
		elogf(LOG_LEVEL_TRACE,
		      "Attempt to stop the memory watcher when it wasn't started.\n");
	}
}

extern void *__real_malloc(size_t size);
extern void __real_free(void *pointer);

void *__wrap_malloc(size_t size)
{
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		void *current_program_break = NULL;
		void *pointer = __real_malloc(size);
		if (pointer != NULL) {
			current_program_break = sbrk(0);
			if (current_program_break != initial_program_break) {
				elogf(LOG_LEVEL_ERR,
				      "Malloc detected, allocation size %zu bytes\n",
				      size);
				free(pointer);
				exit(-1);
			} else {
				return pointer;
			}
		} else {
			elogf(LOG_LEVEL_ERR,
			      "Malloc detected, but failed.  Allocation size %zu bytes\n",
			      size);
			exit(-1);
		}
	} else {
		return __real_malloc(size);
	}
}

void __wrap_free(void *pointer)
{
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		elogf(LOG_LEVEL_ERR,
		      "Use of free() after enabling the memory watcher is not allowed, aborting.\n");
		exit(-1);
	} else {
		return __real_free(pointer);
	}
}

extern void *__real_mmap(void *addr, size_t len, int prot, int flags,
			 int fildes, off_t off);

void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fildes,
		  off_t off)
{
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		elogf(LOG_LEVEL_ERR,
		      "Use of mmap() after enabling the memory watcher is not allowed, aborting.\n");
		exit(-1);
	} else {
		return __real_mmap(addr, len, prot, flags, fildes, off);
	}
}

#endif /* RT_THREAD_PLATFORM || SYLIXOS_PLATFORM || ONEOS_PLATFORM || DONGTU_PLATFORM || RUIHUA_PLATFORM */
