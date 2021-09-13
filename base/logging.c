/** @file logging.c
 * @ingroup base
 * @brief Implementation of the logging facilities in logging.h.
 * @author Mattia Nicolella
 */

#include "logging.h"

/** @details
 * The benchmark verbosity should be initialized only in during the benchmark startup.
 * It is made available as a global variable since every time the logging macro is invoked, this variable
 * must be checked to determine if the message has to be printed.
 * The default log level is ::LOG_LEVEL_INFO.
 */
enum log_level benchmark_verbosity = LOG_LEVEL_INFO;

/** @details
 * Depending on the chosen log level (`::benchmark_verbosity` value), the benchmark timing can be either be printed in a human-friendly or in a csv-like format.
 * The job is assumed to start when the period starts.\n
 * The verbose output is associated to `::LOG_LEVEL_TRACE`, while the csv-like format is associated to both `::LOG_LEVEL_INFO` and `::LOG_LEVEL_FILE`.
 * When `::LOG_LEVEL_ERR` is set no message will be printed.
 *
 * Both `get_rdtsc()` and `get_timestamp()` are used, to be safe in case only one of these methods is working.
 *
 * The csv-like format is composed by the following elements, which will be printed in the order they are described, separated by commas:
 * 1. period start timestamp (in clock cycles);
 * 2. period end timestamp (in clock cycles);
 * 3. job end timestamp (in clock cycles);
 * 4. timestamp (in clock cycles) of the first deadline since job start, called "job deadline";
 * 5. job elapsed time (in clock cycles);
 * 6. period start (in seconds)
 * 7. period end (in seconds)
 * 8. job end (in seconds)
 * 9. job deadline (in seconds)
 * 10. job elapsed time (in seconds)
 * 11. job deadline status:
 *   - `::DEADLINE_MET` if the job deadline was met.
 *   - `::DEADLINE_MISSED` if the job deadline was missed.
 * 12. job utilization (elapsed (clock cycles) / (period end (clock cycles) - period start (clock cycles) ));
 * 13. job density (elapsed (clock cycles) / (job deadline (clock cycles) - period start (clock cycles) ));
 *
 * Example for a completed job with missed deadline: `84208125098780,84208125153008,84208125296876,84208125153008,198096,32451.516650,32451.516671,32451.516726,32451.516671,0.000076,0,3.66,3.66`
 *
 * Since jobs that exceed the deadline are not killed, there can be jobs that take multiple deadlines and periods to terminate.\n
 * The skipped deadlines and periods can be reported by setting all the elements described to `0`, excluding only the deadline timestamp or the period start timestamp.\n
 * This report is made by printing a line where all the elements but the deadline or the period start are set to 0.\n
 * Example for a deadline skip: `0,0,0,2154695482719,0,0.000000000,0.000000000,0.000000000,821.040738944,0.000000000,0,0,0` 
 */
void print_timing(FILE *file, unsigned long long period_start_clocks,
		  unsigned long long period_end_clocks,
		  unsigned long long job_end_clocks,
		  unsigned long long deadline_clocks, long double period_start,
		  long double period_end, long double job_end,
		  long double deadline)
{
	int deadline_status =
		((job_end > 0 && job_end <= deadline) ||
		 (job_end_clocks > 0 && job_end_clocks <= deadline_clocks)) ?
			      DEADLINE_MET :
			      DEADLINE_MISSED;
	long double elapsed =
		(job_end > period_start) ? job_end - period_start : 0;
	unsigned long long elapsed_clocks =
		(job_end_clocks > period_start_clocks) ?
			      job_end_clocks - period_start_clocks :
			      0;
	double utilization =
		(period_end > period_start) ?
			      (elapsed + 0.0) / (period_end - period_start) :
			      0;
	double utilization_clocks =
		(period_end_clocks > period_start_clocks) ?
			      (elapsed_clocks + 0.0) /
				(period_end_clocks - period_start_clocks) :
			      0;
	double density = (deadline > period_start) ?
				       (elapsed + 0.0) / (deadline - period_start) :
				       0;
	double density_clocks =
		(deadline_clocks > period_start_clocks) ?
			      (elapsed_clocks + 0.0) /
				(deadline_clocks - period_start_clocks) :
			      0;
	double d = (density > 0) ? density : density_clocks;
	double u = (utilization > 0) ? utilization : utilization_clocks;
	switch (benchmark_verbosity) {
	case LOG_LEVEL_TRACE:
		if (job_end != 0) {
			printf("\nJob completed\n");
			printf("period start: %llu clock cycles\t %.9Lf seconds \t-\t period end: %llu clock cycles\t %.9Lf seconds\n",
			       period_start_clocks, period_start,
			       period_end_clocks, period_end);
			printf("job end: %llu clock cycles\t %.9Lf seconds\n",
			       job_end_clocks, job_end);
			printf("job deadline: %llu clock cycles\t %.9Lf seconds \t-\tdeadline status:%d (%d=met)\n",
			       deadline_clocks, deadline, deadline_status,
			       DEADLINE_MET);
			printf("job duration: %llu clock cycles\t %.9Lf seconds\t-\tutilization:%.3g\t-\tdensity:%.3g\n\n",
			       elapsed_clocks, elapsed, u, d);
		} else {
			if (deadline != 0 || deadline_clocks != 0) {
				printf("\n\t Deadline %llu (%.9Lf) skipped\n\n",
				       deadline_clocks, deadline);
			}
			if (period_start != 0 || period_start_clocks != 0) {
				printf("\n\t Period %llu (%.9Lf) skipped\n\n",
				       period_start_clocks, period_start);
			}
		}
		break;
	case LOG_LEVEL_FILE:
		fprintf(file,
			"%llu,%llu,%llu,%llu,%llu,%.9Lf,%.9Lf,%.9Lf,%.9Lf,%.9Lf,%d,%.3g,%.3g\n",
			period_start_clocks, period_end_clocks, job_end_clocks,
			deadline_clocks, elapsed_clocks, period_start,
			period_end, job_end, deadline, elapsed, deadline_status,
			u, d);
		break;
	case LOG_LEVEL_INFO:
		printf("%llu,%llu,%llu,%llu,%llu,%.9Lf,%.9Lf,%.9Lf,%.9Lf,%.9Lf,%d,%.3g,%.3g\n",
		       period_start_clocks, period_end_clocks, job_end_clocks,
		       deadline_clocks, elapsed_clocks, period_start,
		       period_end, job_end, deadline, elapsed, deadline_status,
		       u, d);
		break;
	case LOG_LEVEL_ERR:
		break;
	}
}
