/**
 * @file platform_abstraction.h
 * @brief Platform abstraction layer for rt-bench
 * @details This header defines the platform abstraction interface that allows
 *          rt-bench to run on different RTOS platforms (Linux, RT-Thread, etc.)
 */

#ifndef PLATFORM_ABSTRACTION_H
#define PLATFORM_ABSTRACTION_H

#include <inttypes.h>

/* Platform detection */
#if defined(SYLIXOS_PLATFORM)
    /* SylixOS is POSIX-compatible; SylixOS.h is optional and only needed
     * for SylixOS-specific extensions. The platform layer uses standard
     * POSIX APIs (timer_create, sem_init, pthread, clock_gettime, sigaction). */
    #ifdef SYLIXOS_ROOT
        #include <SylixOS.h>
    #endif
    #define RTBENCH_PLATFORM_SYLIXOS
#elif defined(ONEOS_PLATFORM)
    /* OneOS: POSIX-lite profile; prefer POSIX APIs when available */
    #define RTBENCH_PLATFORM_ONEOS

    /* Auto-detect OneOS V2.0 targets (uses musl libc with pre-defined types) */
    #if defined(__aarch64__) || defined(_M_ARM64)
        #define ONEOS_V2_ARM64 1
    #endif
    #if defined(__loongarch__) || defined(__loongarch64) || defined(__loongarch_lp64)
        #define ONEOS_V2_LOONGARCH64 1
    #endif
    #if defined(__riscv) && (__riscv_xlen == 64)
        #define ONEOS_V2_RISCV64 1
    #endif
    #if defined(ONEOS_V2_ARM64) || defined(ONEOS_V2_LOONGARCH64) || defined(ONEOS_V2_RISCV64)
        #define ONEOS_V2_MUSL_LIBC 1
    #endif

    /* For musl libc (OneOS V2.0 targets), these types are already
     * defined in bits/alltypes.h. Only define for non-musl systems (V1.x ARM32). */
    #if !defined(ONEOS_V2_MUSL_LIBC)
        #ifndef _CLOCK_T_DECLARED
        typedef unsigned long clock_t;
        #define _CLOCK_T_DECLARED
        #endif
        #ifndef _CLOCKID_T_DECLARED
        typedef unsigned long clockid_t;
        #define _CLOCKID_T_DECLARED
        #endif
        #ifndef _TIMER_T_DECLARED
        typedef unsigned long timer_t;
        #define _TIMER_T_DECLARED
        #endif
    #endif

    #ifndef _SUSECONDS_T_DECLARED
    typedef long suseconds_t;
    #define _SUSECONDS_T_DECLARED
    #endif
    #ifndef _PID_T_DECLARED
    typedef int pid_t;
    #define _PID_T_DECLARED
    #endif
#elif defined(DONGTU_PLATFORM)
    /* Dongtu Intewell/DTOS family: treat as POSIX-compatible where provided */
    #define RTBENCH_PLATFORM_DONGTU
    #ifndef _CLOCK_T_DECLARED
    typedef unsigned long clock_t;
    #define _CLOCK_T_DECLARED
    #endif
    #ifndef _SUSECONDS_T_DECLARED
    typedef long suseconds_t;
    #define _SUSECONDS_T_DECLARED
    #endif
    #ifndef _CLOCKID_T_DECLARED
    typedef unsigned long clockid_t;
    #define _CLOCKID_T_DECLARED
    #endif
    #ifndef _TIMER_T_DECLARED
    typedef unsigned long timer_t;
    #define _TIMER_T_DECLARED
    #endif
    #ifndef _PID_T_DEFINED
    typedef unsigned long pid_t;
    #define _PID_T_DEFINED
    #endif
#elif defined(RUIHUA_PLATFORM)
    /* Ruihua RTOS (RHRTOS/RHOS): POSIX extensions assumed when building rt-bench */
    #define RTBENCH_PLATFORM_RUIHUA
    #ifndef _CLOCK_T_DECLARED
    typedef unsigned long clock_t;
    #define _CLOCK_T_DECLARED
    #endif
    #ifndef _SUSECONDS_T_DECLARED
    typedef long suseconds_t;
    #define _SUSECONDS_T_DECLARED
    #endif
    #ifndef _CLOCKID_T_DECLARED
    typedef unsigned long clockid_t;
    #define _CLOCKID_T_DECLARED
    #endif
    #ifndef _TIMER_T_DECLARED
    typedef unsigned long timer_t;
    #define _TIMER_T_DECLARED
    #endif
    /* ReWorks/newlib provides pid_t through sys/types.h. */
#elif defined(RT_THREAD_PLATFORM)
    /* Provide POSIX-ish typedef guards for toolchains that hide them */
    #ifndef _CLOCK_T_DECLARED
    typedef unsigned long clock_t;
    #define _CLOCK_T_DECLARED
    #endif
    #ifndef _SUSECONDS_T_DECLARED
    typedef long suseconds_t;
    #define _SUSECONDS_T_DECLARED
    #endif
    #ifndef _CLOCKID_T_DECLARED
    typedef unsigned long clockid_t;
    #define _CLOCKID_T_DECLARED
    #endif
    #ifndef _TIMER_T_DECLARED
    typedef unsigned long timer_t;
    #define _TIMER_T_DECLARED
    #endif
    #ifndef _PID_T_DECLARED
    typedef int pid_t;
    #define _PID_T_DECLARED
    #endif
    #include <rtthread.h>
    #define RTBENCH_PLATFORM_RTTHREAD
#elif defined(LINUX_PLATFORM) || defined(__linux__)
    #define RTBENCH_PLATFORM_LINUX
#else
    #error "Unsupported platform. Define SYLIXOS_PLATFORM, ONEOS_PLATFORM, DONGTU_PLATFORM, RUIHUA_PLATFORM, RT_THREAD_PLATFORM, or use Linux."
#endif

/* ============================================================================
 * Timer Abstraction
 * ============================================================================ */

/**
 * @brief Timer handle (opaque type)
 */
typedef void* rtbench_timer_t;

/**
 * @brief Timer signal types
 */
typedef enum {
    RTBENCH_TIMER_DEADLINE = 0,
    RTBENCH_TIMER_PERIOD = 1
} rtbench_timer_type_t;

/**
 * @brief Timer callback function type
 */
typedef void (*rtbench_timer_callback_t)(void *user_data);

/**
 * @brief Create a timer
 * @param timer_type Type of timer (DEADLINE or PERIOD)
 * @param callback Callback function to call when timer expires
 * @param user_data User data to pass to callback
 * @return Timer handle on success, NULL on failure
 */
rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
                                      rtbench_timer_callback_t callback,
                                      void *user_data);

/**
 * @brief Set timer interval and start it
 * @param timer Timer handle
 * @param sec Seconds
 * @param nsec Nanoseconds
 * @return 0 on success, negative on failure
 */
int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec);

/**
 * @brief Delete a timer
 * @param timer Timer handle
 * @return 0 on success, negative on failure
 */
int rtbench_timer_delete(rtbench_timer_t timer);

/* ============================================================================
 * Synchronization Abstraction
 * ============================================================================ */

/**
 * @brief Semaphore handle (opaque type)
 */
typedef void* rtbench_sem_t;

/**
 * @brief Create a semaphore
 * @param initial_value Initial semaphore value
 * @return Semaphore handle on success, NULL on failure
 */
rtbench_sem_t rtbench_sem_create(unsigned int initial_value);

/**
 * @brief Wait on a semaphore
 * @param sem Semaphore handle
 * @return 0 on success, negative on failure
 */
int rtbench_sem_wait(rtbench_sem_t sem);

/**
 * @brief Post to a semaphore
 * @param sem Semaphore handle
 * @return 0 on success, negative on failure
 */
int rtbench_sem_post(rtbench_sem_t sem);

/**
 * @brief Destroy a semaphore
 * @param sem Semaphore handle
 * @return 0 on success, negative on failure
 */
int rtbench_sem_destroy(rtbench_sem_t sem);

/* ============================================================================
 * Scheduling Abstraction
 * ============================================================================ */

/**
 * @brief Scheduling policy
 */
typedef enum {
    RTBENCH_SCHED_FIFO = 0,
    RTBENCH_SCHED_DEADLINE = 1
} rtbench_sched_policy_t;

/**
 * @brief Set thread priority (for FIFO scheduling)
 * @param priority Priority value
 * @return 0 on success, negative on failure
 */
int rtbench_set_priority(unsigned int priority);

/**
 * @brief Set deadline scheduling parameters
 * @param runtime Runtime in nanoseconds
 * @param deadline Deadline in nanoseconds
 * @param period Period in nanoseconds
 * @return 0 on success, negative on failure
 */
int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period);

/**
 * @brief Set CPU affinity
 * @param cpu_mask CPU mask (bitmap)
 * @return 0 on success, negative on failure
 */
int rtbench_set_affinity(uint32_t cpu_mask);

/* ============================================================================
 * Timestamp Abstraction
 * ============================================================================ */

/**
 * @brief Get CPU timestamp counter (in clock cycles)
 * @return Timestamp counter value
 */
unsigned long long rtbench_get_rdtsc(void);

/**
 * @brief Get timestamp in seconds
 * @return Timestamp in seconds (as long double)
 */
long double rtbench_get_timestamp(void);

/* ============================================================================
 * Signal/Event Abstraction (for compatibility)
 * ============================================================================ */

/**
 * @brief Signal types
 */
typedef enum {
    RTBENCH_SIG_DEADLINE = 0,
    RTBENCH_SIG_PERIOD = 1,
    RTBENCH_SIG_QUIT = 2
} rtbench_signal_t;

/**
 * @brief Signal handler function type
 */
typedef void (*rtbench_signal_handler_t)(rtbench_signal_t sig, void *context);

/**
 * @brief Register a signal handler
 * @param signal Signal type
 * @param handler Handler function
 * @return 0 on success, negative on failure
 */
int rtbench_signal_register(rtbench_signal_t signal, 
                            rtbench_signal_handler_t handler);

#endif /* PLATFORM_ABSTRACTION_H */
