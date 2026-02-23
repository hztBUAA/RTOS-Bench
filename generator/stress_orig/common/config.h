/* applications/stress-ng/config.h */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define STRESS_SIMPLE_MODE

/* atomic */
#define DEFAULT_ATOMIC_THREADS  4

/* bitops */
#define DEFAULT_BITOPS_LOOPS    1000

/* bsearch */
#define MIN_BSEARCH_SIZE    128
#define DEFAULT_BSEARCH_SIZE (256 * 1024)
#define MAX_BSEARCH_SIZE    (2 * 1024 * 1024)

/* context */
#define DEFAULT_CONTEXT_THREADS (2)

/* copy-file */
#define MIN_COPY_FILE_BYTES     (4 * 1024)
#define DEFAULT_COPY_FILE_BYTES (64 * 1024)
#define MAX_COPY_FILE_BYTES     (4 * 1024 * 1024)

/* cpu */
#define DEFAULT_CPU_LOAD 100

/* dentry */
#define MIN_DENTRIES        1
#define DEFAULT_DENTRIES    32
#define MAX_DENTRIES        512

/* fp */
#define DEFAULT_FP_LOOPS  (2048)

/* fstat */
#define DEFAULT_FSTAT_FILES (32)

/* hdd */
#define MIN_HDD_BYTES       (4 * 1024)
#define DEFAULT_HDD_BYTES   (64 * 1024)
#define MAX_HDD_BYTES       (8 * 1024 * 1024)

/* malloc */
#define MIN_MALLOC_BYTES    16
#define DEFAULT_MALLOC_BYTES (4 * 1024)
#define MAX_MALLOC_BYTES    (8 * 1024 * 1024)
#define MIN_MALLOC_MAX      32
#define MAX_MALLOC_MAX      (64 * 1024)
#define DEFAULT_MALLOC_MAX  256

/* matrix */
#define MIN_MATRIX_SIZE     16
#define DEFAULT_MATRIX_SIZE 64
#define MAX_MATRIX_SIZE     128

/* memcpy */
#define DEFAULT_MEMCPY_MEMSIZE      (32 * 1024)
#define DEFAULT_MEMCPY_LOOPS        (64)

/* memthrash */
#define MIN_MEM_SIZE        (1 * 1024)
#define DEFAULT_MEM_SIZE    (64 * 1024)
#define MAX_MEM_SIZE        (4 * 1024 * 1024)

/* open */
#define DEFAULT_OPEN_MAX    32
#define MIN_OPEN_MAX        4
#define MAX_OPEN_MAX        256

/* pipe */
#define MIN_PIPE_DATA_SIZE      (64)
#define DEFAULT_PIPE_DATA_SIZE  (4096)
#define MAX_PIPE_DATA_SIZE      (64 * 1024)

/* prime */
#define DEFAULT_PRIME_START 10000000ULL

/* ptr-chase */
#define DEFAULT_PTR_CHASE_PAGES (4096)
#define MIN_PTR_CHASE_PAGES     (4)

/* qsort */
#define MIN_QSORT_SIZE      128
#define MAX_QSORT_SIZE      (1 * 1024 * 1024)
#define DEFAULT_QSORT_SIZE  (1024)

/* rename */
#define MIN_RENAME_FILE_SIZE    0
#define DEFAULT_RENAME_FILE_SIZE 16
#define MAX_RENAME_FILE_SIZE    (256 * 1024)

/* stack */
#define DEFAULT_STACK_SIZE      (16 * 1024)
#define STACK_SIZE_MIN          (2048)
#define STACK_SIZE_MAX          (64 * 1024)

/* str */
#define MIN_STR_SIZE        32
#define DEFAULT_STR_SIZE    256
#define MAX_STR_SIZE        (64 * 1024)

/* stream */
#define DEFAULT_STREAM_ELEM     (1024)
#define MIN_STREAM_ELEM         (128)
#define MAX_STREAM_ELEM         (256 * 1024)

/* trig */
#define DEFAULT_TRIG_LOOPS  (5000)

/* unlink */
#define DEFAULT_UNLINK_FILES   32
#define MAX_UNLINK_FILES        256

/* vecmath */
#define DEFAULT_VECMATH_LOOPS   100

/* vm */
#define MIN_VM_BYTES        (4 * 1024)
#define DEFAULT_VM_BYTES    (256 * 1024)
#define MAX_VM_BYTES        (16 * 1024 * 1024)

#endif
