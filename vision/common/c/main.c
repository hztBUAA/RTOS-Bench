#include "periodic_benchmark.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>

/** @file main.c
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
	struct execution_options *parsed_args = state->input;
	switch (key) {
		//default values for arguments and options
	case ARGP_KEY_INIT:
		parsed_args->deadline_nsec = 0;
		parsed_args->deadline_sec = 0;
		parsed_args->args_num = 0;
		parsed_args->args = NULL;
		parsed_args->output_path = NULL;
		break;
	case ARGP_KEY_ARG:
		//we want to directly grab the argument list, after the options have been parsed
		if (state->arg_num == 0) {
			//so we use the index to the next argument to retrieve the position of the first argument in argv
			parsed_args->args = state->argv + (state->next - 1);
			//we compute the number of elements in argv from the first argument to the end of the array
			parsed_args->args_num = state->argc - (state->next - 1);
			//and we modify the next argument to finish scanning argv
			state->next = state->argc;
		} else {
			argp_error(state, "Error parsing arguments");
		}
		break;
	case 's':
		parsed_args->deadline_sec = strtol(arg, NULL, 10);
		if (errno != 0) {
			argp_failure(state, EXIT_FAILURE, errno,
				     "Error during deadline (seconds) parsing");
		}
		break;
	case 'n':
		parsed_args->deadline_nsec = strtol(arg, NULL, 10);
		if (errno != 0) {
			argp_failure(
				state, EXIT_FAILURE, errno,
				"Error during deadline (nanoseconds) parsing");
		}
		break;
	case 'o':
		parsed_args->output_path = arg;
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
	case ARGP_KEY_END:
		if (parsed_args->args_num < 1)
			argp_error(state, "Not enough arguments");
		if (parsed_args->deadline_nsec == 0 &&
		    parsed_args->deadline_sec == 0)
			argp_error(
				state,
				"Deadline in seconds and deadline in nanoseconds cannot be both 0");
		// if an output path is not specified we will use the input folder path (specified in the first argument)
		if (parsed_args->output_path == NULL) {
			parsed_args->output_path = parsed_args->args[0];
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
	int res = 0;
	struct execution_options parsed_args;

	//argp variables
	const char *argp_doc =
		"Run a benchmark periodically, trying to meet the given deadline.";
	const char *argp_args_doc =
		"input_data_folder [additional arguments...]";
	struct argp_option argp_options[] = {
		{ 0, 0, 0, 0, "Arguments:", 1 },
		{ "input_data_folder", 0, 0, OPTION_NO_USAGE | OPTION_DOC,
		  "Path to the folder where benchmark input data is located." },
		{ "additional_arguments", 0, 0, OPTION_NO_USAGE | OPTION_DOC,
		  "Additional arguments relayed directly to the benchmark." },
		{ 0, 0, 0, 0,
		  "Deadline options (at least one is required):", 2 },
		{ "deadline-sec", 's', "sec", 0,
		  "The deadline in seconds, when not specified it is assumed to be 0." },
		{ "deadline-nsec", 'n', "nsec", 0,
		  "An optional deadline specification in nanoseconds, which can be used in conjunction with the deadline in seconds. If not specified it is assumed to be 0." },
		{ 0, 0, 0, 0, "Reporting options:", 3 },
		{ "log-level", 'l', "log-lvl", 0,
		  "Log level, can be one of the following:\n1 - Print only errors.\n2 - Print benchmark stats only to output file.\n3 - Print benchmark stats also on stdout.\n4 - Print also informative messages.\nDefault is 3." },
		{ "output", 'o', "output_path", 0,
		  "Where the info on the benchmark execution will be written. If not supplied, the input folder path will be used." },
		{ 0, 0, 0, 0, "Informational options:", -1 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	//initializing argp struct
	struct argp argp = { 0 };
	argp.args_doc = argp_args_doc;
	argp.doc = argp_doc;
	argp.parser = parse_opt;
	argp.options = argp_options;

	//parsing parameters
	res = argp_parse(&argp, argc, argv, 0, 0, &parsed_args);
	if (res != 0) {
		perror("Error during argument parsing");
		return EXIT_FAILURE;
	}

	//benchmark initialization
	res = periodic_benchmark(&parsed_args);
	if (res < 0) {
		return EXIT_FAILURE;
	}
}
