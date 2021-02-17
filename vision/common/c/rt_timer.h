/** \file timer_with_signal.h
 * A general implementation of a real time timer, where expiration triggers a SIGRTMIN and SIGINT is used to stop and destroy the timer.
*/

#include <signal.h>
#include <time.h>

/// This struct is used to keep data and parameters necessary to the initialization and tear down of the data used by the function that will handle the SIGRTMIN.
typedef struct _execution_data{
	timer_t timer; ///< The timer structure which will hold the created timer.
	int parameters_num; ///< The number of parameters passed to the init and teardown functions.
	void** parameters; ///< The parameters array that will be passed to the the init and teardown functions.
	int (*init)(int parameters_num,void** parameters); ///< The init function, called before the tmer creation.
	void(*execution)(int parameters_num, void** parameters); ///< The execution function, which will be used as the SIGRTMIN handler.
	void (*teardown)(int parameters_num,void** parameters); ///< The teardown function, which will be called after the timer has been destroyed.
} execution_data;

/** \brief The function that will handle the timer creation, the periodic execution and the clean up.
 * \param[in] edata The struct containing the execution data used to create the timer.
 * \param[in] deadline_sec The timer deadline in seconds.
 * \param[in] deadline_nsec The timer deadline in nanoseconds.
 * \return 0 in case of success and an error code otherwise.
 */
int start_benchmark(execution_data* edata, long deadline_sec,long deadline_nsec);
