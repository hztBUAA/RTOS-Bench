#! /bin/python3

"""!
@file base.py
@ingroup utils
@author Mattia Nicolella
@brief General procedures for a benchmark test

This script contains general procedures  that will be used by other tests to generate data.

If executed not as a module will let the user choose what kind of test perform and handle the test execution.

Available test at the moment are:
- `WCET`: Worst Case Execution Time.
- `sched`: Schedulablity test.
- `WSS`: minimum Working Set Size

@note This script might require administrative privileges (to change scheduling policy and move processes in other cores).

Dependencies:
- Python 3.5+
- lscpu
- grep
- sort
- wc
- ps
- taskset
- mkdir
- schedulability.py (for `sched` execution)
- WCET.py (for `sched` and `WCET` execution)
- graph.py as an optional dependency

@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
"""

import argparse
import csv
import datetime
import os
import signal
import subprocess

try:
    import graph

    graph_imported = True
except ImportError:
    graph_imported = False


def convert_corelist_to_list(corelist):
    """! @brief Converts a corelist (string) in a python list.

    @param[in] corelist The corelist to convert.
    @returns A python list of all the cores in the corelist.
    """
    tmp = corelist.split(",")
    core_list = []
    for elem in tmp:
        if "-" in elem:
            tmp2 = elem.split("-")
            for i in range(int(tmp2[0]), int(tmp2[1]) + 1):
                core_list.append(i)
        else:
            core_list.append(int(elem))
    return core_list


def start_interfering(deadline, bmarks, num_cpus, target_core, system_core, fifo_prio):
    """! @brief Launches interfering benchmarks.

    @param[in] deadline The deadline (and period) to give to the interfering benchmarks.
    @param[in] bmarks The list of interfering benchmarks executables.
    @param[in] num_cpus The number of available physical cores.
    @param[in] target_core The corelist on which the benchmark will run, which must not have an interfering benchmark.
    @param[in] system_core The corelist on which the benchmark will run, which must not have an interfering benchmark.
    @param[in] fifo_prio The priority of the interfering benchmarks
    @details
    The interfering benchmarks can be executed on all the available cores, excluding the first, the decision of which benchmark to place on which core is left to the scheduler.
    The benchmarks will continue to be executed until they receive a `SIGINT`.
    @returns The list of process id associated to the spawned benchmarks or `None` on error.
    """
    print("Starting interfering benchmarks")
    cores = []
    system_core_list = convert_corelist_to_list(system_core)
    target_core_list = convert_corelist_to_list(target_core)
    # exclude core 0 and the target core
    if len(bmarks) > num_cpus - 2:
        print(
            "WARNING:You have more interfering benchmarks than physical cores, this will assign more that one benchmark to the same core, with fifo scheduling and the same priority."
        )
    i = 0
    # we want the core array to be as long the the list of interfering benchmarks.
    while len(cores) < len(bmarks):
        # We don't want interfering benchmarks on core 0.
        core = (i % (num_cpus - 1)) + 1
        # or on the core that will run the target benchmark.
        if core not in target_core_list and core not in system_core_list:
            cores.append(str(core))
        i += 1
    bmark_processes = []
    for i in range(0, len(bmarks)):
        if bmarks[i][1] != []:
            interf_args = [
                bmarks[i][0],
                "-d",
                str(deadline),
                "-p",
                str(deadline),
                "-l",
                "1",
                "-c",
                cores[i],
                "-f",
                str(fifo_prio),
                "-b",
                f'"{" ".join(bmarks[i][1])}"',
            ]
            interf_args = " ".join(interf_args)
        try:
            bmark_processes.append(
                subprocess.Popen(
                    interf_args,
                    shell=True,
                )
            )
        except Exception as e:
            print("Error while starting interfering benchmarks", e)
            stop_interfering(bmark_processes)
            return None
    print("done")
    return bmark_processes


def stop_interfering(processes):
    """! @brief Stops the running interfering benchmarks.

    @param[in] processes The list of interfering benchmark that are in execution.
    @details
    The interfering benchmarks are stopped with a `SIGINT`.
    """
    print("Stopping interfering benchmarks")
    for process in processes:
        process.send_signal(signal.SIGINT)
    print("done")


def detect_cores():
    """! @brief Detect physical and logical cores.

    @returns The number of available physical and logical cores in the machine, as a tuple (phys,logic). -1 is given on error.
    """
    # get the number of available cores
    try:
        phys_cores = int(
            subprocess.check_output(
                "lscpu -b -p=Core,Socket | grep -v '^#' | sort -u | wc -l",
                text=True,
                shell=True,
            )
        )
    except Exception as e:
        print("Error during retrieval of number physical of cores: ", e)
        return -1
    try:
        logic_cores = int(
            subprocess.check_output(
                "grep -c ^processor /proc/cpuinfo", text=True, shell=True
            )
        )
    except Exception as e:
        print("Error during retrieval of number logical of cores: ", e)
        return -1
    return (phys_cores, logic_cores)


def move_processes(corelist):
    """! @brief Move processes according to the provided corelist.

    @param[in] corelist The list of cores (as a `taskset` compatible string) on which processes are allowed to run.
    @details
    Moves all the existing processes (assumed to not be part of the testing set) according to the provided corelist, to avoid interference as much as possible.

    @note Even as root, not all processes can be moved between cores. This types of errors will be silently ignored.
    @returns `-1` on errors which are not ignored, `0` otherwise.
    """
    print(f"moving processes on corelist {corelist}")
    # get the number of processes
    try:
        task_list = subprocess.check_output(["ps", "-e", "-o", "pid"], text=True)
    except Exception as e:
        print("Error during retrieval of process list: ", e)
        return -1
    # after having the pid list, we can use tasklist to set the cpu affinity to everything but core 0
    for pid in task_list.split("\n"):
        pid = pid.strip()
        if pid not in ("PID", " ", ""):
            try:
                subprocess.check_output(
                    ["taskset", "-acp", corelist, pid], stderr=subprocess.DEVNULL
                )
            except Exception:
                pass
    print("done")
    return 0


def handle_bmark_list(bmark_list):
    """! @brief Split the list of benchmark in executable,arguments tuples.

    @param[in] bmark_list The list of benchmark in the `"executable:arg1,arg2,..."` format.
    @returns a list of benchmark in the `["executable","arg1 arg2 ..."]` format.
    """
    new_list = []
    for bmark in bmark_list:
        if ":" in bmark:
            tmp = bmark.split(":")
            new_list.append((tmp[0], tmp[1].split(",")))
        else:
            new_list.append((bmark, []))
    return new_list


def parser_init(description="A script to perform various tests"):
    """! @brief Initialize an argument parser with a set of common arguments.

    @returns The initialized `ArgumentParser` object.
    """
    # set up the argument parser
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument(
        "-u",
        "--utilization-increase",
        metavar="utilization",
        type=float,
        help="How much the utilization should increase at each test step (only for schedulability) used also the utilization initial value.",
        default=0.05,
        choices=map(lambda x: x / 100.0, range(1, 100)),
        required=False,
        dest="util_inc",
    )

    parser.add_argument(
        "-tn",
        "--tasks-number",
        metavar="tasks-num",
        type=int,
        help="The number of tasks to be executed for each test step. (WCET test excluded)",
        default=100,
        required=False,
        dest="tasks_num",
    )

    parser.add_argument(
        "-wt",
        "--worst-case-tests",
        metavar="tests-num",
        type=int,
        help="The number of tasks to be executed when searching for the worst case execution time.",
        default=1000,
        required=False,
        dest="worst_case_tests",
    )

    parser.add_argument(
        "-wd",
        "--worst-case-threshold",
        metavar="0.0-1.0",
        type=float,
        help="The percentage of missed deadlines that can be allowed when searching for the worst case execution time.",
        default=0,
        required=False,
        dest="worst_case_threshold",
    )

    parser.add_argument(
        "-wdline",
        "--worst-case-deadline",
        metavar=">0.0",
        type=float,
        help="The minumum deadline for the worst case execution time.",
        default=0.001,
        required=False,
        dest="worst_case_deadline",
    )
    parser.add_argument(
        "-i",
        "--interfering-bmarks",
        metavar="bmark-exec:arg1,arg2,...",
        nargs="+",
        type=str,
        help="A list of interfering benchmarks executables, with their arguments.  This option can be specified multiple times.",
        default=[],
        required=False,
        dest="interfering",
    )

    parser.add_argument(
        "-b",
        "--bmarks",
        metavar="bmark-exec:arg1,arg2,...",
        nargs="+",
        type=str,
        default=[],
        help="A list of  benchmarks executables, with their arguments. A schedulability test will be performed on each of these benchmarks.\n This option can be specified multiple times.",
        required=False,
        dest="benchmarks",
    )

    parser.add_argument(
        "-o",
        "--output",
        metavar="path",
        nargs="+",
        type=str,
        help="The location where all the generated output files will be located. This option can be repeated for each target benchmark.",
        required=False,
        default=["./"],
        dest="output",
    )

    parser.add_argument(
        "-pre",
        "--prefix",
        metavar="prefix",
        type=str,
        help="A prefix to set on the generated files",
        required=False,
        default="",
        dest="prefix",
    )

    parser.add_argument(
        "-c",
        "--target_core",
        metavar="core1,core2-coren",
        type=str,
        help="Corelist on which the target benchmark will be executed. If not provided is the last physical core.",
        required=False,
        default=None,
        dest="target_core",
    )
    parser.add_argument(
        "-sc",
        "--system_core",
        metavar="core1,core2-coren",
        type=str,
        help="Corelist on which the system processes will be moved. If not provided is core 0.",
        required=False,
        default="0",
        dest="system_core",
    )

    parser.add_argument(
        "-fi",
        "--fifo-interfering",
        metavar="fifo-prio",
        type=int,
        help="Priority for the SCHED_FIFO scheduler of the interfering benchmarks.",
        required=False,
        default=None,
        dest="fifo_interfering",
    )

    parser.add_argument(
        "-f",
        "--fifo",
        metavar="fifo-prio",
        type=int,
        help="Priority for the SCHED_FIFO scheduler of the target benchmark.",
        required=False,
        default=None,
        dest="fifo",
    )

    parser.add_argument(
        "-D",
        "--sched_deadline",
        metavar="sched_deadline_deadline",
        type=int,
        help="Deadline in nanoseconds for the SCHED_DEADLINE scheduler of the target benchmark, will override --fifo.",
        required=False,
        default=None,
        dest="sched_deadline",
    )

    parser.add_argument(
        "-T",
        "--sched_runtime",
        metavar="sched_deadline_runtime",
        type=int,
        # we set the target core is was not set by the use="Runtime in nanoseconds for the SCHED_DEADLINE scheduler of the target benchmark, will override --fifo.",
        required=False,
        default=None,
        dest="sched_runtime",
    )

    parser.add_argument(
        "-P",
        "--sched_period",
        metavar="sched_deadline_period",
        type=int,
        help="Period in nanoseconds for the SCHED_DEADLINE scheduler of the target benchmark, will override --fifo.",
        required=False,
        default=None,
        dest="sched_period",
    )

    parser.add_argument(
        "-post",
        "--postfix",
        metavar="postfix",
        type=str,
        help="A postfix to set on the generated files",
        required=False,
        default="",
        dest="postfix",
    )

    parser.add_argument(
        "-g",
        "--draw-graph",
        metavar="graph",
        type=str,
        nargs="?",
        help="If tests should also draw graphs. Choices are 'yes', 'no' (default), 'only'. Selecting 'only' will make the test only draw graphs from the csv files specified in --graph-inputs",
        default="no",
        choices=["no", "yes", "only"],
        required=False,
        const="yes",
        dest="draw_graph",
    )
    parser.add_argument(
        "-gi",
        "--graph-inputs",
        metavar="file1.csv file2.csv ...",
        type=str,
        nargs="+",
        help="A space separated list of csv files from a previous test, that will used when '-g only' is specified to create one graph per file",
        default=[],
        required=False,
        dest="graph_inputs",
    )
    return parser


def test_init(parser):
    """! @brief Function that will set up the machine for executing the test.

    @param[in] parser The parser created by `parser_init()`.
    @details
    This function will:
    - parse the given input arguments;
    - detect the number of available cores on the machine;
    - move all processes to the first physical core (core 0);

    This function will return a dictionary called `params` which will contain:
    - `args`: The parsed CLI arguments.
    - `cores`: The number of cores detected by `detect_cores()`.
    - `sched_params`: A list with the scheduling parameters of the target benchmark.
    - `res`: The result of the last operation.
    - `timestamp`: The timestamp of the test in ISO format (with `-` instead of `:` and `.` to avoid filesystem problems).

    @returns a dictionary called `params`.
    """
    # initialize the dictionary that will be returned
    params = {
        "res": 0,
        "cores": None,
        "int_processes": None,
        "args": None,
        "sched_params": [],
        "timestamp": datetime.datetime.now()
        .isoformat()
        .replace(":", "-")
        .replace(".", "-"),
    }
    # parse arguments
    args = parser.parse_args()
    # we convert the Namespace to a dictionary since we need to modify some parameters
    args = vars(args)
    if not graph_imported:
        args["draw_graph"] = "no"

    sched_deadline_vals = [
        args["sched_deadline"] is not None,
        args["sched_runtime"] is not None,
        args["sched_period"] is not None,
    ]
    # handle scheduling parameters
    if (
        args["draw_graph"] != "only"
        and args["fifo"] is None
        and not any(sched_deadline_vals)
    ):
        print(
            "Missing scheduling parameters! Provide SCHED_FIFO or SCHED_DEADLINE parameters!"
        )
        params.update({"res": -1})
        return params
    if args["fifo"] is not None and any(sched_deadline_vals):
        print("ERROR: cannot specify options for both SCHED_FIFO and SCHED_DEADLINE")
        params.update({"res": -1})
        return params
    if any(sched_deadline_vals) and not all(sched_deadline_vals):
        print("ERROR: missing options for SCHED_DEADLINE")
        params.update({"res": -1})
        return params
    if args["fifo_interfering"] is None and args["interfering"] != []:
        print("ERROR: missing SCHED_FIFO priority for interfering benchmarks")
        params.update({"res": -1})
        return params
    # we use SCHED_FIFO if nothing from SCHED_DEADLINE has been specified
    if all(sched_deadline_vals) is False:
        sched_params = ["-f", str(args["fifo"])]
    else:
        print(sched_deadline_vals)
        sched_params = [
            "-D",
            args["sched_deadline"],
            "-P",
            args["sched_period"],
            "-T",
            args["sched_runtime"],
        ]
    params.update({"sched_params": sched_params})
    # adjust the list of benchmarks and interfering benchmarks
    args["benchmarks"] = handle_bmark_list(args["benchmarks"])
    args["interfering"] = handle_bmark_list(args["interfering"])

    output_len = len(args["output"])
    if output_len > 0 and len(args["benchmarks"]) > output_len:
        print(
            "A single folder has been specified as output path for more than one target benchmark, test will continue but it may overwrite previous files."
        )
    for i in range(0, len(args["benchmarks"])):
        if output_len == 0:
            args["output"].append(os.path.dirname(args["benchmarks"][i][0]))
        elif output_len == len(args["benchmarks"]):
            subprocess.run(["mkdir", "-p", args["output"][i]])
        else:
            if i > 0:
                args["output"].append(args["output"][0])
    if (
        len(args["benchmarks"]) == 0
        and args["draw_graph"] != "only"
        and args.get("test") != "overhead"
    ):
        print("ERROR: Missing benchmark list!")
        params.update({"res": -1})
        return params
    if args["draw_graph"] == "only" and len(args["graph_inputs"]) == 0:
        print("ERROR: Missing input files for graph generation!")
        params.update({"res": -1})
        return params
    if args["draw_graph"] != "only":
        # we detect the physical cores
        cores = detect_cores()
        if cores == -1:
            params.update({"res:": cores})
            return params
        params.update({"cores": cores})
        # we set the target core is was not set by the user
        if args["target_core"] is None:
            args["target_core"] = str(cores[0] - 1)
        # we move all processes to the first core
        move_processes(args["system_core"])
    params.update({"args": args})
    return params


def test_teardown(params):
    """! @brief The function that will revert the environment to the state it had before the benchmark test.

    @param[in] params: A dictionary containing the parsed arguments, the list of interfering benchmarks and the number of detected cores. Provided by `test_init()`.
    @details
    This function will:
    - stop the interfering benchmarks it they were started.
    - restore the default core affinity for all the other processes.

    To stop the interfering benchmarks `params` needs to contain a key called `int_processes`, which will contain the return value of `start_interfering()`.

    @return 0 on success, -1 on error.
    """
    args = params.get("args")
    int_processes = params.get("int_processes")
    if args["draw_graph"] != "only":
        cores = params.get("cores")
        if cores is None or args is None:
            print("ERROR: Missing parameters for test_teardown()")
            return -1

        # we restore the normal execution of the tasks
        move_processes(f"0-{cores[1]-1}")
    # we stop the interfering benchmarks if necessary
    if args["interfering"] != [] and int_processes is not None:
        stop_interfering(int_processes)
    return 0


def stringify_list(data_list):
    """! @brief Transform a list of items in as string, separating them with ;.

    @param[in] data_list The list of items to transform.
    @return A string containing all list items separated by ;
    """
    args_str = ""
    args_num = len(data_list)
    for i in range(0, args_num):
        args_str += str(data_list[i])
        if i < args_num - 1:
            args_str += ";"
    return args_str


def unstringify_list(list_str):
    """! @brief Turns a string created by stringify_list() in back into a list.

    @param[in] list_str The string to turn back into a list.
    @returns The list derived from the string.
    """
    return list_str.split(";")


def reduce_args(args):
    """! @brief Reduces the list of arguments int a single string, concatenating their basename.

    @param[in] args The list of arguments.
    @return a string with the reduced arguments.
    """
    args_str = ""
    args_len = len(args)
    for i in range(0, args_len):
        args_str += os.path.basename(args[i])
        if i < args_len - 1:
            args_str += "_"
    return args_str


def save_graph(sched_graph, outputs, name):
    """! @brief Saves the given graph to file.

    @param[in] sched_graph The graph to save.
    @param[in] outputs A list of paths where the graph needs to be saved.
    @param[in] name The filename to use.
    """
    for output in outputs:
        graph.export_graph(
            sched_graph,
            os.path.join(output, name),
        )
    graph.teardown()


def draw_and_save_graph(draw_function, params, data_dict_key, name, fields, conv=None):
    """! @brief Draw and save a graph either by using the given data or by parsing csv files.

    @param[in] draw_function the function that draws a graph.
    @param[in] params The parameters dictionary.
    @param[in] data_dict_key The string that represents the dictionary with the test data inside params.
    @param[in] name The graph name.
    @param[in] fields the fields to read from the csv files.
    @param[in] conv The list containing the convertion function for each field.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: missing arguments structure to draw graph!")
        params.update({"res": -1})
        return params
    interf_str = ""
    if args["draw_graph"] != "no":
        if len(args["interfering"]) > 0 or "interfering" in args["graph_inputs"]:
            interf_str = "_interfering"
        if args["draw_graph"] == "only":
            data = parse_res_csv(
                args["graph_inputs"],
                fields,
                conv=conv,
            )
            if data is None:
                print("ERROR: could not read data from input files")
                params.update({"res": -1})
                return params
            for read_timestamp in data.keys():
                drawn_graph = draw_function(
                    data[read_timestamp], interference=interf_str != ""
                )
                save_graph(
                    drawn_graph,
                    args["output"],
                    args["prefix"]
                    + read_timestamp
                    + "_"
                    + name
                    + interf_str
                    + args["postfix"],
                )
        if args["draw_graph"] == "yes":
            data = params.get(data_dict_key)
            if data is None:
                print("ERROR: missing data dictionary to print graph")
                params.update({"res": -1})
                return params
            drawn_graph = draw_function(data, interference=interf_str != "")
            save_graph(
                drawn_graph,
                args["output"],
                args["prefix"] + name + interf_str + args["postfix"],
            )
    params.update({"res": 0})
    return params


def parse_res_csv(inputs, fields, conv=[]):
    """! @brief get data from a a csv which is timestamped.

     @param[in] inputs A list of inputs.
     @param[in] fields A list of csv fields that must be read.
     @param[in] conv A list of functions to convert the fields to a specific datatype. If unspecified alla lists will contain strings.
     @returns The parsed data, in a dictionary with a key for every timestamp. `None` on error.
     @details
     Supported csv file mus have at least 3 columns:
     - `timestamp`: with the test timestamp
     - `benchmark`: with the benchmark executable path
     - `arguments`: with the list of argument of the benchmark, separated by `;`.
     Additionally fields in the csv must be separated by `,`.

     Each timestamp key will locate a dictionary with a list of lists (one sublist per benchmark).
     Each sublist will have as elements the values of the fields that match the benchmark and the timestamp.
     Additionally there will a list called `legend` which will combine the benchmark name and arguments to create a label for each of the above sublists.

     If several input files have the same timestamp data will be aggregated.

     Example: Calling: `parse_res_csv(file.csv,['utilization','num_scheduled'],conv=[float,int])` with a csv generated by schedulability.py could return:
     `data={'2022-02-09T20-26-31-407037':{'utilization':[[0.1, ... , 1] [0.1, ... , 1]], 'num_scheduled':[[100, ..., 0] [100, ..., 0]],'legend':['disparity - cif','mser - vga']}}`.
    @todo This function should be superseded by a wrapper that leverages more powerful parsing libraries.
    """
    if len(inputs) < 1:
        print(
            "ERROR: Wrong number of input files! This test needs at least 1 input file."
        )
        return None
    data = {}
    len_fields = len(fields)
    if conv == []:
        for i in range(0, len_fields):
            conv.append(str)
    for filepath in inputs:
        with open(filepath) as file:
            csv_reader = csv.DictReader(file)
            for row in csv_reader:
                timestamp = row.get("timestamp")
                if timestamp is None:
                    print("ERROR: Missing timestamp value in input file!")
                    print(row)
                    return None
                # we get the corresponding dictionary or we Initialize it
                timestamped_data = data.get(timestamp)
                if timestamped_data is None:
                    data.update({timestamp: {}})
                    timestamped_data = data.get(timestamp)
                    for field in fields:
                        timestamped_data.update({field: []})
                    timestamped_data.update({"legend": []})
                # we create the string for the legend
                bmark = row.get("benchmark")
                if bmark is None:
                    print("ERROR: Missing benchmark name in input file!")
                    print(row)
                    return None
                bmark = os.path.basename(bmark)
                args = row.get("arguments").split(";")
                reduced_args = reduce_args(args)
                legend_str = bmark + " - " + reduced_args
                if legend_str not in timestamped_data["legend"]:
                    timestamped_data["legend"].append(legend_str)
                    # we also create a sublist for this benchmark in each field list
                    for field in fields:
                        timestamped_data[field].append([])
                # we read the data in the row
                for j in range(0, len_fields):
                    read_data = row.get(fields[j])
                    if read_data is None:
                        print(f"ERROR: Missing {fields[j]} value in input file!")
                        print(row)
                        return None
                    timestamped_data[fields[j]][-1].append(conv[j](read_data))

    return data


# Execute the main function when this script is not a module
if __name__ == "__main__":
    parser_obj = parser_init()
    # we add an argument to let the user choose the type of test to execute.
    tests_available = ["all", "WCET", "sched", "WSS", "overhead"]
    help_str = "Determines the type of test to execute:\n\n\t WCET: Worst Case Execution Test\n\tsched: Schedulability test\n\t WSS: minimum working set size test\n\tall: execute all available tests"
    parser_obj.add_argument(
        "-tt",
        "--test-type",
        metavar="test",
        type=str,
        help=help_str,
        choices=tests_available,
        required=False,
        default="all",
        dest="test",
    )
    test_params = test_init(parser_obj)
    if test_params["res"] < 0:
        exit(test_params["res"])
    parsed_args = test_params.get("args")
    if parsed_args["test"] in ["WCET", "all"]:
        import WCET

        WCET.execute(test_params)
        test_teardown(test_params)
    if parsed_args["test"] in ["sched", "all"]:
        import schedulability

        schedulability.execute(test_params)
        test_teardown(test_params)
    if parsed_args["test"] in ["all", "WSS"]:
        import WSS

        WSS.execute(test_params)
        test_teardown(test_params)
    if parsed_args["test"] in ["all", "overhead"]:
        import overhead

        overhead.execute(test_params)
        test_teardown(test_params)
