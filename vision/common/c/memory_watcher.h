/** @file memory_watcher.h
 * @brief A simple memory watcher that allows preallocation and checks that the heap will not be expanded.
 */
#ifndef MEMORY_WATCHER_H
#define MEMORY_WATCHER_H

#include<stdlib.h>

/** @brief Initializes the memory watcher and preallocates the necessary memory.
 * @param[in] bytes_to_preallocate The amount of memory that must be preallocated.
 */
void start_memory_watcher(size_t bytes_to_preallocate);

///@brief Stops the memory watcher.
void stop_memory_watcher();
#endif
