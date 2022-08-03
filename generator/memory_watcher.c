/** @file memory_watcher.c
 * @ingroup generator
 * @brief Implementation of a memory watcher, which will crash the program if it detects and heap extension.
 * @author Mattia Nicolella
 *
 * **Dependencies**:
 * - Glibc.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include "logging.h"

///Enum used to determine the memory watcher states
enum memory_watcher_states {
	MEMORY_WATCHER_DISABLED = 0, ///< The memory watcher is not enabled.
	MEMORY_WATCHER_ENABLED ///< The memory watcher is enabled.
};

/** The status of the memory watcher.
 * Possible values are defined by `::memory_watcher_states`.
 */
static enum memory_watcher_states memory_watcher_status =
	MEMORY_WATCHER_DISABLED;

/** @brief The initial value of the program break.
 * This value is used to determine if the program heap was expanded after an allocation.
 */
static const void *initial_program_break = NULL;

/**
 * Memory preallocation is done via `mallopt()`, using `M_TOP_PAD`.
 * In addition, we need to avoid having `malloc()` use `mmap()`, so `mallopt()` is used to set `M_MMAP_MAX` to `0`.
 * Then, a dummy allocation (a `malloc()` and a `free()`) is performed, to have the requested memory preallocated.
 *
 * To enable the memory watcher, `::memory_watcher_status` is set to `::MEMORY_WATCHER_ENABLED` and
 * the initial value of the program break is stored in `::initial_program_break` via `sbrk(0)`.
 * As a side effect from the memory watcher start, `mmap()` will be disabled.
 */
void start_memory_watcher(size_t bytes_to_preallocate)
{
	int res;
	void *dummy_alloc = NULL;
	if (bytes_to_preallocate > 0) {
		if (memory_watcher_status == MEMORY_WATCHER_DISABLED) {
			elogf(LOG_LEVEL_TRACE,
			      "Starting  memory watcher, bytes to preallocate: %zu.\n",
			      bytes_to_preallocate);
			//preallocate the requested memory
			res = mallopt(M_TOP_PAD, bytes_to_preallocate);
			if (res == 0) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot preallocate %zu bytes.\n",
				      bytes_to_preallocate);
				exit(-1);
			}
			//disable mmap usage
			res = mallopt(M_MMAP_MAX, 0);
			if (res == 0) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot disable mmap based allocation.\n");
				exit(-1);
			}
			// a dummy allocation to have malloc preallocate the requested amount of memory.
			dummy_alloc = malloc(bytes_to_preallocate);
			if (dummy_alloc == NULL) {
				elogf(LOG_LEVEL_ERR,
				      "Cannot allocate dynamic memory, aborting.\n");
				exit(-1);
			}
			free(dummy_alloc);
			//we get the value of the program break after the preallocation.
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

/**
 * To disable the memory watcher is we set `::memory_watcher_status` to `::MEMORY_WATCHER_DISABLED`,
 * to re-enable the use of `mmap()` in `malloc()`, by setting `M_MMAP_MAX` to its default value (`65536`), via `mallopt()` and
 * to reset `M_TOP_PAD` to its default value (`128*1024`) via `mallopt()`.
 */
void stop_memory_watcher()
{
	int res;
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		elogf(LOG_LEVEL_TRACE, "Stopping memory watcher.\n");
		//stop the memory watcher
		memory_watcher_status = MEMORY_WATCHER_DISABLED;
		//reset M_TOP_PAD
		res = mallopt(M_TOP_PAD, 128 * 1024);
		if (res == 0) {
			elogf(LOG_LEVEL_ERR,
			      "Cannot reset M_TOP_PAD, after stopping memory watcher.\n");
			exit(-1);
		}
		//enable mmap usage
		res = mallopt(M_MMAP_MAX, 65536);
		if (res == 0) {
			elogf(LOG_LEVEL_ERR,
			      "Cannot enable mmap based allocation after stopping memory watcher.\n");
			exit(-1);
		}
	} else {
		elogf(LOG_LEVEL_TRACE,
		      "Attempt to disable the memory watcher when it is not enabled.\n");
	}
}

/// The symbol that corresponds to the glibc `malloc()`, after the linker has made the wrapping.
extern void *__real_malloc(size_t size);

/** @brief The wrapped `malloc()` function, where the memory watcher is implemented.
 * @details Every time `malloc()` is invoked, we let the original implementation allocate memory via `__real_malloc()`, then we check,
 * via `sbrk(0)`, if the current program break is different from the value in `::initial_program_break`.
 * When these values differ we free the memory that was allocated, give the user an error message and call `exit(-1)`.
 */
void *__wrap_malloc(size_t size)
{
	void *pointer = NULL, *current_program_break = NULL;
	pointer = __real_malloc(size);
	if (memory_watcher_status == MEMORY_WATCHER_ENABLED) {
		current_program_break = sbrk(0);
		if (current_program_break == (void *)-1) {
			perror("Cannot find the current program break.");
			exit(-1);
		}
		if (current_program_break != initial_program_break) {
			free(pointer);
			elogf(LOG_LEVEL_ERR,
			      "Memory allocation of %zu bytes has caused an heap extension.\ninitial program break: %p\ncurrent program break:%p.\nExecution will be aborted.",
			      size, initial_program_break,
			      current_program_break);
			exit(-1);
		}
	}
	return pointer;
}

///The symbol that corresponds to the real `mmap()`, after the linker has done the wrapping.
extern void *__real_mmap(void *addr, size_t len, int prot, int flags,
			 int fildes, off_t off);

/** @brief Wrapper of `mmap()` which disables the function if the memory watcher is enabled.
 * @details If `mmap()` is called when the memory watcher is enabled, the program will crash using `exit(-1)`.
 */
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
