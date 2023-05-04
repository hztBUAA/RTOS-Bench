#include "periodic_benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>
#include <math.h>
#include <fenv.h>
#include "logging.h"
#include <string.h>
#include "sched_attr.h"

#include <inttypes.h>
#include <sched.h>

#ifdef JSON_SUPPORT
#include <json-c/json.h>
#endif

/** @file main.c
 * @ingroup generator
 * @author Mattia Nicolella
 * @brief Benchmark entry point.
 * @details Will handle the benchmark startup and its parameters.
 *
 * **Dependencies**:
 * - Glibc.
 * - Linux syscalls, namely `set_schedattr` and `get_schedattr`.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

/**
 * @brief Set sched_deadline policy for current thread.
 * @returns 0 on success, < 0 on failure
 *
 * @details
 * @note `sched_setattr()` is not provided as wrapper in most glibc.
 *
 * @note  When using the sched_deadline policy, the task/thread is descheduled
 * as soon as the deadline is reached (hard reservation). This might impact
 * the performance/possibly correctness of the computation. For example,
 * consider disparity with a scheduling policy FIFO at rt-prio 50. Disparity
 * will run to completion (within the "soft reservation" enforced by the -d 1,
 * -p 1 seconds of rt-bench).
 * 
 * @note `# ./disparity -d 1 -p 1 -f 50 -b . .`
 * `5211492644212,5213189418717,5211566941556,5213189418717,74297344,3050.207200004,3051.207129868,3050.250938539,3051.207129868,0.043738535,1,0.0437,0.0437`
 *
 * @note When giving a combination of parameters period = 500us, deadline = 400 us,
 * expected runtime 300us, the results might be considerably different.
 *
 * @note `# ./disparity -d 1 -p 1 -P 500000 -D 400000 -T 300000 -b . .`
 * `5108693043772,5110389822935,5109043819868,5110389822935,350776096,2989.623136457,2990.623058494,2989.829806155,2990.623058494,0.206669698,1,0.207,0.207`
 */
static int set_sched_deadline(
	/* IN: period (see chrt or include/linux/sched/types.h */
	uint64_t period,
	/* IN: deadline (see chrt or include/linux/sched/types.h */
	uint64_t deadline,
	/* IN: runtime (see chrt or include/linux/sched/types.h */
	uint64_t runtime)
{
	int ret;
	struct rtbench_sched_attr attr = { 0 };

	/* Keep compatibility with chrt, at least the period must be != 0 */
	if (period == 0) {
		return -1;
	}

	if (deadline == 0) {
		deadline = period;
	}

	if (runtime == 0) {
		runtime = deadline;
	}

	attr.size = sizeof(struct rtbench_sched_attr);
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = runtime;
	attr.sched_deadline = deadline;
	attr.sched_period = period;

	/* NOTE: sched_setattr() is not provided as wrapper in most glibc */
	ret = sched_setattr(0, &attr, 0);
	if (ret != 0) {
		return ret;
	}

	/* Try to read the info back */
	attr.size = sizeof(attr);
	attr.sched_policy = 0;
	attr.sched_runtime = 0;
	attr.sched_deadline = 0;
	attr.sched_period = 0;

	ret = sched_getattr(0, &attr, sizeof(attr), 0);
	if (ret != 0) {
		return ret;
	}

	elogf(LOG_LEVEL_INFO,
	      "\nsize: %u, policy: %u, flags: %lu, prio: %u"
	      "\nT: %lu, D: %lu, P: %lu\n",
	      attr.size, attr.sched_policy, attr.sched_flags,
	      attr.sched_priority, attr.sched_runtime, attr.sched_deadline,
	      attr.sched_period);

	return ret;
}

/**
 * @brief Set sched_fifo as scheduling policy.
 * @param[in] prio The priority of this benchmark (see `chrt` or `include/linux/sched/types`).
 * @returns
 *   0 on success
 *   < 0 on failure

 * @note If this function fails when the program is run by the root user try `ulimit -r unlimited` and 
 * `echo $$ > /sys/fs/cgroup/cpu/tasks` from the shell that will run the program. See [this stackoverflow question](https://stackoverflow.com/questions/9313428/getting-eperm-when-calling-pthread-create-for-sched-fifo-thread-as-root-on-lin) for details  
 */
static int set_sched_fifo_prio(unsigned int prio)
{
	int ret;
	struct rtbench_sched_attr attr = { 0 };

	/* cap prio to max */
	if (prio > sched_get_priority_max(SCHED_FIFO)) {
		prio = sched_get_priority_max(SCHED_FIFO);
	}

	attr.sched_policy = SCHED_FIFO;
	attr.sched_priority = prio;

	/* NOTE: sched_setattr() is not provided as wrapper in most glibc */
	ret = sched_setattr(0, &attr, 0);
	if (ret != 0) {
		return ret;
	}

	/* Try to read the info back */
	attr.size = sizeof(attr);
	attr.sched_policy = 0;
	attr.sched_runtime = 0;
	attr.sched_deadline = 0;
	attr.sched_period = 0;

	ret = sched_getattr(0, &attr, sizeof(attr), 0);
	if (ret != 0) {
		return ret;
	}

	elogf(LOG_LEVEL_INFO,
	      "\nsize: %u, policy: %u, flags: %lu, prio: %u"
	      "\nT: %lu, D: %lu, P: %lu\n",
	      attr.size, attr.sched_policy, attr.sched_flags,
	      attr.sched_priority, attr.sched_runtime, attr.sched_deadline,
	      attr.sched_period);

	return ret;
}

#ifdef JSON_SUPPORT
/** @brief Convert the long options to their short version.
 * @param[in] arg The option to shorten.
 * @returns The shortened option.
 * @details In case the translation is not available the function will call will use the `exit` syscall to terminate the program with an error.
 */
static char field_to_abbrv_mapping(char *arg)
{
	if (!strcmp(arg, "deadline"))
		return 'd';
	else if (!strcmp(arg, "period"))
		return 'p';
	else if (!strcmp(arg, "core-affinity"))
		return 'c';
	else if (!strcmp(arg, "mem-limit"))
		return 'm';
	else if (!strcmp(arg, "tasks-number"))
		return 't';
	else if (!strcmp(arg, "sched-deadline"))
		return 'D';
	else if (!strcmp(arg, "fifo"))
		return 'f';
	else if (!strcmp(arg, "sched-period"))
		return 'P';
	else if (!strcmp(arg, "sched-runtime"))
		return 'T';
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	else if (!strcmp(arg, "memory-profiling-enable"))
		return 'M';
	else if (!strcmp(arg, "memory-profiling-core"))
		return 'C';
	else if (!strcmp(arg, "memory-profiling-time-bucket"))
		return 'B';
#endif
	else if (!strcmp(arg, "log-level"))
		return 'l';
	else if (!strcmp(arg, "output"))
		return 'o';
	else if (!strcmp(arg, "bmark-args"))
		return 'b';
	else {
		printf("Invalid/unsupported parameter \"%s\" in configuration file!\n",
		       arg);
		exit(0);
	}
}
#endif

/** @brief Parse cli or JSON options and arguments.
 * @param[in] key The parsed key (e.g. s if the parameters is -s 100) .
 * @param[in] arg The value associated with the parsed key.
 * @param[in,out] state The argp parser state when this function it's called.
 * @returns 0 or an error code.
 * @details This function is invoked every time argp encounters a parameter, and it will identify the parsed parameter and store it accordingly, excluding the `-g` option.
 * This function decouples parsing all the options from handling the `-g` option.
 */
static int interpret_opt(int key, const char *arg, struct argp_state *state)
{
	int res = 0;
	int log_level = LOG_LEVEL_INFO;
	double time_spec;
	long seconds, nanoseconds;
	struct execution_options *parsed_args = state->input;
	size_t preallocation = 0;
	char preallocation_magnitude = '\0';
	int affinity_core;
	char *affinity_substr = NULL;
	unsigned long long tasks = 0;
	char *output_extension = "";
	int arg_len = 0;
	errno = 0;
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	unsigned long long memory_profiling_core_affinity;
#endif

	switch (key) {
	case 'b':
		arg_len = strlen(arg);
		// We get the arg, split the string, and rearrange in args
		if (state->arg_num == 0) {
			parsed_args->args_num = 0;
			// Overprovision the array size. It cannot be as big as the element composing it
			parsed_args->args =
				(char **)malloc(sizeof(char *) * (arg_len + 1));
			char sep[] = " ";
			char *ptr = strtok((char *)arg, sep);
			while (ptr != NULL) {
				parsed_args->args[parsed_args->args_num] =
					(char *)malloc(sizeof(char) *
						       (strlen(ptr) + 1));
				strcpy(parsed_args->args[parsed_args->args_num],
				       ptr);
				parsed_args->args_num++;
				ptr = strtok(NULL, sep);
			}
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
		arg_len = strlen(arg);
		int path_len = 0;
		//add csv extension if needed
		if (strcmp(arg + (arg_len - 4), ".csv") == 0) {
			parsed_args->output_path =
				malloc(sizeof(char) * arg_len + 1);
			path_len = arg_len + 1;
			output_extension = "";
		} else {
			output_extension = ".csv";
			parsed_args->output_path = malloc(
				sizeof(char) *
				(arg_len + strlen(output_extension) + 1));
			path_len = arg_len + strlen(output_extension) + 1;
		}
		if (parsed_args->output_path == NULL) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Can't allocate memory for output filename.");
		}
		snprintf(parsed_args->output_path, path_len, "%s%s", arg,
			 output_extension);
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
		affinity_substr = (char *)arg;
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
	case 'f':
		parsed_args->prio = strtoul(arg, NULL, 0);
		break;
	case 'T':
		parsed_args->runtime = strtoull(arg, NULL, 0);
		break;
	case 'D':
		parsed_args->deadline = strtoull(arg, NULL, 0);
		break;
	case 'P':
		parsed_args->period = strtoull(arg, NULL, 0);
		break;
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	case 'M':
		parsed_args->memory_profiling_enable = strtoul(arg, NULL, 0);
		break;
	case 'C':
		memory_profiling_core_affinity = strtoul(arg, NULL, 0);
		CPU_SET(memory_profiling_core_affinity,
			&parsed_args->memory_profiling_core_affinity);
		break;
	case 'B':
		parsed_args->memory_profiling_time_bucket =
			strtoul(arg, NULL, 0);
		break;
#endif
	default:
		res = ARGP_ERR_UNKNOWN;
	}
	return res;
}

/** @brief Parse cli/JSON options and arguments via argp.
 * @param[in] key The parsed key (e.g. s if the parameters is -s 100) .
 * @param[in] arg The value associated with the parsed key.
 * @param[in,out] state The argp parser state when this function it's called.
 * @returns 0 or an error code.
 * @details This function is invoked every time argp encounters a parameter, and it will identify the parsed parameter and store it accordingly.
 * This function will rely on `interpret_opt()` to parse all the options. It performs some initialization steps and load the JSON file if necessary.
 * Integrity checks and scheduler policy changes are also performed here. 
 */
static int parse_opt(int key, char *arg, struct argp_state *state)
{
	int res = 0;
	struct execution_options *parsed_args = state->input;
	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);
#ifdef JSON_SUPPORT
	json_object *root;
#endif
	switch (key) {
		//default values for arguments and options
	case ARGP_KEY_INIT:
		memset(parsed_args, 0, sizeof(struct execution_options));
		CPU_ZERO(&parsed_args->core_affinity);
		parsed_args->prio = 100;
		parsed_args->runtime = 0;
		parsed_args->period = 0;
		parsed_args->deadline = 0;
		parsed_args->memory_profiling_enable = 0;
		CPU_ZERO(&parsed_args->memory_profiling_core_affinity);
		parsed_args->memory_profiling_time_bucket = 10000000;
		parsed_args->output_path = NULL;
		break;
#ifdef JSON_SUPPORT
	case 'g':
		root = json_object_from_file(arg);
		if (root == NULL) {
			argp_error(
				state,
				"Error: Cannot open JSON configuration file.");
			break;
		}
		printf("%d", root == NULL);
		json_object_object_foreach(root, first, second)
		{
			res = interpret_opt(field_to_abbrv_mapping(first),
					    json_object_get_string(second),
					    state);
			if (res < 0) {
				argp_error(state,
					   "Problem in parsing JSON file.");
				break;
			}
		}
		json_object_put(root);
		break;
#endif
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
		if ((parsed_args->prio != 100) &&
		    ((parsed_args->period > 0) || (parsed_args->deadline > 0) ||
		     (parsed_args->runtime > 0))) {
			argp_error(state,
				   "Incompatible FIFO and DEADINE policies.");
		}

		if ((((parsed_args->deadline > 0) ||
		      (parsed_args->runtime > 0))) &&
		    (parsed_args->period == 0)) {
			argp_error(
				state,
				"--sched-period must be provided for SCHED_DEADLINE policy.");
		}

		/* setup scheduling policies */
		if (parsed_args->prio != 100) {
			res = set_sched_fifo_prio(parsed_args->prio);
			if (res < 0) {
				argp_error(
					state,
					"Error setting sched-fifo prio (are you root?)");
			}
		}

		if (parsed_args->period > 0) {
			res = set_sched_deadline(parsed_args->period,
						 parsed_args->deadline,
						 parsed_args->runtime);
			if (res < 0) {
				argp_error(
					state,
					"Error setting sched-deadline params (are you root?)");
			}
		}
		break;
	default:
		res = interpret_opt(key, arg, state);
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
#ifdef JSON_SUPPORT
		{ 0, 0, 0, 0, "Configuration input:", 1 },
		{ "configuration-file", 'g', "config_path", 0,
		  "Specify the JSON file describing the configuration to use. Following options complement or override the JSON description. Conversely, options specified before are complemented or overwritten." },
#endif
		{ 0, 0, 0, 0, "Period and deadline options:", 2 },
		{ "deadline", 'd', "secs", 0,
		  "The benchmark deadline in seconds. Can be an integer, float or in scientific notation. Required. Must be less or equal than the benchmark period." },
		{ "period", 'p', "secs", 0,
		  "The benchmark period, in seconds. Can be an integer, float or in scientific notation. Required." },
		{ 0, 0, 0, 0, "Execution options:", 3 },
		{ "core-affinity", 'c', "core0,core1,...", 0,
		  "The benchmark core affinity, expressed as a comma separated list. A single core id is also accepted." },
		{ "mem-limit", 'm', "bytes[GMK]", 0,
		  "The maximum amount of dynamic memory allocated during the periodic execution. If exceeded, the benchmark will crash. Specified as an integer plus an optional magnitude modifier: K=kilobytes, M=megabytes, G=gigabytes. Without a magnitude modifier specified the value is assumed to be in bytes. 0 Means no limit, and it is the default setting." },
		{ "tasks-number", 't', "integer>=0", 0,
		  "The number of tasks to be executed. 0 means until the program receives a SIGINT. Default is 0." },
		{ 0, 0, 0, 0, "Scheduling options:\n\n", 4 },
		{ "fifo", 'f', "0<=prio<=99", 0,
		  "Set SCHED_FIFO priority with specified priority. Need root." },
		{ "sched-runtime", 'T', "ns", 0,
		  "Set SCHED_DEADLINE runtime. Alternative to --fifo. Need root." },
		{ "sched-deadline", 'D', "ns", 0,
		  "Set SCHED_DEADLINE deadline. Alternative to --fifo. Need root." },
		{ "sched-period", 'P', "ns", 0,
		  "Set SCHED_DEADLINE period. Alternative to --fifo. Need root. At least --sched-period has to be specified to set sched_deadline params. If deadline is not specified, deadline is set to period. If runtime is not specified, runtime is set to deadline. NOTE: These parameters are different from --period and --deadline used to control the repetitive execution of the thread. To generate valid execution that are not truncated under hard server reservation, period < sched-period and deadline < sched-deadline." },
		{ 0, 0, 0, 0, "Reporting options:", 5 },
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
		{ "memory-profiling-enable", 'M', "bool", 0,
		  "Enables runtime memory profiling. Specify '1' to enable or '0' otherwise." },
		{ "memory-profiling-core", 'C', "core0, core1,...", 0,
		  "Core affinity of the runtime memory profiling thread. If not specified, it matches the 'core-affinity' parameter. Warning: 'memory-profiling-enable' must be asserted for this parameter to take effect." },
		{ "memory-profiling-time-bucket", 'B', "ns", 0,
		  "Period between measurements performed by the runtime memory profiler. If not specified, time bucket of 10ms is set. Warning: 'memory-profiling-enable' must be asserted for this parameter to take effect." },
#endif
		{ "log-level", 'l', "log-lvl", 0,
		  "Log level, can be one of the following:\n1 - Print only errors.\n2 - Print benchmark stats to output file.\n3 - Print benchmark stats to stdout.\n4 - Print also informative messages on stderr.\nDefault is 3." },
		{ "output", 'o', "output_path", 0,
		  "Where the info on the benchmark execution will be written. If not supplied, \"./timing.csv\" will be used." },
		{ 0, 0, 0, 0, "Benchmark arguments and options:", 6 },
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

	// Clean/free buffers
	for (size_t i = 0; i < parsed_args.args_num; i++)
		free(parsed_args.args[i]);
	free(parsed_args.args);

	// return failure if exist
	if (res < 0) {
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}
