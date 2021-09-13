#include "periodic_benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>
#include <math.h>
#include <fenv.h>
#include "logging.h"
#include <string.h>

/** @file main.c
 * @ingroup base
 * @author Mattia Nicolella
 * @brief Benchmark entry point.
 * @details Will handle the benchmark startup and its parameters.
 */

/** @brief Parse cli options and arguments via argp.
 * @param[in] key The parsed key (e.g. s if the parameters is -s 100) .
 * @param[in] arg The value associated with the parsed key.
 * @param[in,out] state The argp parser state when this function it's called.
 * @returns 0 or an error code.
 * @details This function is invoked every time argp encounters a parameter, and it will identify the parsed parameter and store it accordingly.
 */
static int parse_opt(int key, char *arg, struct argp_state *state)
{
	int res = 0, log_level = LOG_LEVEL_INFO;
	double time_spec;
	long seconds, nanoseconds;
	struct execution_options *parsed_args = state->input;
	size_t preallocation = 0;
	char preallocation_magnitude = '\0';
	int affinity_core;
	char *affinity_substr = NULL;
	unsigned long long tasks = 0;
	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);
	switch (key) {
		//default values for arguments and options
	case ARGP_KEY_INIT:
		memset(parsed_args, 0, sizeof(struct execution_options));
		CPU_ZERO(&parsed_args->core_affinity);
		break;
	case 'b':
		//we want to directly grab the argument list, after the -b flag
		if (state->arg_num == 0) {
			//so we use the index to the next argument to retrieve the position of the next argument in argv after -b
			parsed_args->args = state->argv + (state->next - 1);
			//we compute the number of elements in argv from the first argument after -b to the end of the array
			parsed_args->args_num = state->argc - (state->next - 1);
			//and we modify the next argument to finish scanning argv
			state->next = state->argc;
		} else {
			argp_error(
				state,
				"Error parsing benchmark arguments and options");
		}
		break;
	case 'm':
		/* the argument should contain the number of bytes to preallocate and an order of magnitude
		 * K for kilobytes, M for megabytes and G for gigabytes
		 * e.g 1G = 1 gigabyte preallocated.*/
		res = sscanf(arg, "%zu%1c", &preallocation,
			     &preallocation_magnitude);
		if (res < 1 || res == EOF) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Error during preallocation argument parsing");
		}
		res = 0;
		/*we convert the parsed value in bytes save it as an execution option.
		 * breaks are omitted to obtain a proper conversion in bytes. */
		switch (preallocation_magnitude) {
		case 'g':
		case 'G':
			preallocation *= 1024;
		case 'm':
		case 'M':
			preallocation *= 1024;
		case 'k':
		case 'K':
			preallocation *= 1024;
		case '\0':
			parsed_args->bytes_to_preallocate = preallocation;
			break;
		default:
			argp_error(state, "Preallocation magnitude invalid");
		}
		break;
	case 'd':
	case 'p':
		//common operations to convert the deadline or period from a decimal number to itimerspec values
		time_spec = strtod(arg, NULL);
		if (errno != 0) {
			argp_failure(state, EXIT_FAILURE, errno,
				     "Error during period or deadline parsing");
		}

		seconds = lround(trunc(time_spec));
		res = fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW |
				   FE_UNDERFLOW);
		if (res != 0) {
			argp_error(state, "Error during conversion in seconds");
		}
		nanoseconds =
			lround((time_spec - trunc(time_spec)) * 1000000000);
		res = fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW |
				   FE_UNDERFLOW);
		if (res != 0) {
			argp_error(state,
				   "Error during conversion in nanoseconds");
		}
		//assignment of the parsed values depends on the key
		switch (key) {
		case 'd':
			parsed_args->parsed_deadline = time_spec;
			parsed_args->deadline_sec = seconds;
			parsed_args->deadline_nsec = nanoseconds;
			break;
		case 'p':
			parsed_args->parsed_period = time_spec;
			parsed_args->period_sec = seconds;
			parsed_args->period_nsec = nanoseconds;
			break;
		}
		break;
	case 'o':
		parsed_args->output_path =
			malloc(sizeof(char) * strlen(arg) + 1);
		if (parsed_args->output_path == NULL) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Can't allocate memory for output filename.");
		}
		strncpy(parsed_args->output_path, arg,
			sizeof(char) * strlen(arg) + 1);
		break;
	case 'l':
		log_level = atoi(arg);
		if (log_level >= LOG_LEVEL_ERR &&
		    log_level <= LOG_LEVEL_TRACE) {
			benchmark_verbosity = log_level;
		} else {
			argp_error(state, "Wrong log level supplied.");
		}
		break;
	case 'c':
		// we create the mask based on what cores the user has specified.
		affinity_substr = arg;
		//we read one core id at a time and we insert it in the mask
		while (affinity_substr != NULL) {
			res = sscanf(affinity_substr, "%d%*s", &affinity_core);
			if (res < 1 || res == EOF) {
				argp_failure(
					state, EXIT_FAILURE, errno,
					"Error during preallocation argument parsing");
			}
			res = 0;
			CPU_SET(affinity_core, &parsed_args->core_affinity);
			//we search for other cores and prepare the sscanf input to read the next core id.
			affinity_substr = strstr(affinity_substr, ",");
			if (affinity_substr != NULL &&
			    affinity_substr[0] == ',') {
				affinity_substr++;
			}
		}
		if (CPU_COUNT(&parsed_args->core_affinity) == 0) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Error during core affinity argument parsing, no core selected.");
		}
		break;
	case 't':
		errno = 0;
		tasks = strtoull(arg, NULL, 10);
		if (errno != 0) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Error during number of tasks argument parsing.");
		}
		parsed_args->tasks_to_launch = tasks;
		break;
	case 'h':
		argp_state_help(state, stdout,
				ARGP_HELP_USAGE | ARGP_HELP_LONG);
		exit(EXIT_SUCCESS);
		break;
	case ARGP_KEY_END:
		if (parsed_args->deadline_nsec == 0 &&
		    parsed_args->deadline_sec == 0)
			argp_error(state, "Missing required deadline value.");
		if (parsed_args->period_sec == 0 &&
		    parsed_args->period_nsec == 0)
			argp_error(state, "Missing required period value.");
		if (parsed_args->parsed_deadline > parsed_args->parsed_period) {
			argp_error(
				state,
				"Deadlines longer than period are not supported.");
		}
		break;
	default:
		res = ARGP_ERR_UNKNOWN;
	}
	return res;
}

/** @brief The program entry point, which will parse the given parameters and start the benchmark.
 * @param[in] argc Number of given parameters.
 * @param[in] argv given parameters array.
 * @details Parameter passing is done by configuring and using argp, then the parsed parameters are used to initialize the periodic benchmark.
 */
int main(int argc, char **argv)
{
	int res = 0, i;
	struct execution_options parsed_args;

	//argp variables
	const char *argp_doc =
		"Run a benchmark periodically, trying to meet the given deadline.";
	const char *argp_args_doc = "";
	struct argp_option argp_options[] = {
		{ 0, 0, 0, 0, "Period and deadline options:", 1 },
		{ "deadline", 'd', "secs", 0,
		  "The benchmark deadline in seconds. Can be an integer, float or in scientific notation. Required. Must be less or equal than the benchmark period." },
		{ "period", 'p', "secs", 0,
		  "The benchmark period, in seconds. Can be an integer, float or in scientific notation. Required." },
		{ 0, 0, 0, 0, "Execution options:", 2 },
		{ "core-affinity", 'c', "core1,core2,...", 0,
		  "The benchmark core affinity, expressed as a comma separated list. A single core id is also accepted." },
		{ "mem-limit", 'm', "bytes[GMK]", 0,
		  "The maximum amount of dynamic memory allocated during the periodic execution. If exceeded, the benchmark will crash. Specified as an integer plus an optional magnitude modifier: K=kilobytes, M=megabytes, G=gigabytes. Without a magnitude modifier specified the value is assumed to be in bytes. 0 Means no limit, and it is the default setting." },
		{ "tasks-number", 't', "integer>=0", 0,
		  "The number of tasks to be executed. 0 means until the program receives a SIGINT. Default is 0." },
		{ 0, 0, 0, 0, "Reporting options:", 3 },
		{ "log-level", 'l', "log-lvl", 0,
		  "Log level, can be one of the following:\n1 - Print only errors.\n2 - Print benchmark stats to output file.\n3 - Print benchmark stats to stdout.\n4 - Print also informative messages on stderr.\nDefault is 3." },
		{ "output", 'o', "output_path", 0,
		  "Where the info on the benchmark execution will be written. If not supplied, \"./timing.csv\" will be used." },
		{ 0, 0, 0, 0, "Benchmark arguments and options:", 4 },
		{ "bmark-args", 'b', "arg opt ...", 0,
		  "A space-separated list of arguments and options that must be relayed directly to the benchmark. It must be specified after every other option since everything after it will be passed directly to the benchmark routine." },
		{ 0, 0, 0, 0, "Informational options:\n", -1 },
		{ NULL, 'h', NULL, 0, NULL },
		{ 0, 0, 0, 0, 0, 0 }
	};
	//initializing argp struct
	struct argp argp = { 0 };
	argp.args_doc = argp_args_doc;
	argp.doc = argp_doc;
	argp.parser = parse_opt;
	argp.options = argp_options;

	//parsing parameters
	res = argp_parse(&argp, argc, argv, ARGP_IN_ORDER, NULL, &parsed_args);
	if (res != 0) {
		perror("Error during argument parsing");
		return EXIT_FAILURE;
	}

	if (benchmark_verbosity == LOG_LEVEL_TRACE) {
		elogf(LOG_LEVEL_TRACE, "parsed arguments:\n");
		elogf(LOG_LEVEL_TRACE, "\targument number:%d\n",
		      parsed_args.args_num);
		elogf(LOG_LEVEL_TRACE, "\targuments:\n");
		for (i = 0; i < parsed_args.args_num; i++) {
			elogf(LOG_LEVEL_TRACE, "\t  %d - %s\n", i,
			      parsed_args.args[i]);
		}
		elogf(LOG_LEVEL_TRACE, "\tdeadline:%.3g\n",
		      parsed_args.parsed_deadline);
		elogf(LOG_LEVEL_TRACE, "\tdeadline in seconds:%ld\n",
		      parsed_args.deadline_sec);
		elogf(LOG_LEVEL_TRACE, "\tdeadline in nanoseconds:%ld\n",
		      parsed_args.deadline_nsec);
		elogf(LOG_LEVEL_TRACE, "\tperiod:%.3g\n",
		      parsed_args.parsed_period);
		elogf(LOG_LEVEL_TRACE, "\tperiod in seconds:%ld\n",
		      parsed_args.period_sec);
		elogf(LOG_LEVEL_TRACE, "\tperiod in nanoseconds:%ld\n",
		      parsed_args.period_nsec);
		elogf(LOG_LEVEL_TRACE, "\toutput path: %s\n",
		      parsed_args.output_path);
		elogf(LOG_LEVEL_TRACE,
		      "\tmemory to preallocate (in bytes):%zu\n",
		      parsed_args.bytes_to_preallocate);
	}

	//benchmark initialization
	res = periodic_benchmark(&parsed_args);
	if (res < 0) {
		return EXIT_FAILURE;
	}
}
