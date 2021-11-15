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
"""

import argparse
import subprocess
import os
import signal


def start_interfering(deadline, bmarks, num_cpus):
    """!
    @brief Launches interfering benchmarks
    @param[in] deadline The deadline (and period) to give to the interfering benchmarks.
    @param[in] bmarks The list of interfering benchmarks executables.
    @param[in] num_cpus The number of available physical cores.
    @details
    The interfering benchmarks can be executed on all the available cores, excluding the first, the decision of which benchmark to place on which core is left to the scheduler.
    The benchmarks will continue to be executed until they receive a `SIGINT`.
    @returns The list of process id associated to the spawned benchmarks or `None` on error.
    """
    print("Starting interfering benchmarks")
    cpu_range = "0"
    for i in range(1, num_cpus - 1):
        cpu_range += "," + str(i)
    bmark_processes = []
    for i in range(0, len(bmarks)):
        try:
            bmark_processes.append(
                subprocess.Popen(
                    [
                        bmarks[i][0],
                        "-d",
                        str(deadline),
                        "-p",
                        str(deadline),
                        "-l",
                        "1",
                        "-c",
                        cpu_range,
                        "-f",
                        "1",
                        "-b",
                    ]
                    + bmarks[i][1]
                )
            )
        except Exception as e:
            print("Error while starting interfering benchmarks", e)
            stop_interfering(bmark_processes)
            return None
    print("done")
    return bmark_processes


def stop_interfering(processes):
    """!
    @brief Stops the running interfering benchmarks.
    @param[in] processes The list of interfering benchmark that are in execution.
    @details
    The interfering benchmarks are stopped with a `SIGINT`.
    """
    print("Stopping interfering benchmarks")
    for process in processes:
        process.send_signal(signal.SIGINT)
    print("done")


def detect_cores():
    """!
    @brief Detect physical and logical cores.
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
    """!
    @brief Move processes according to the provided corelist.
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
                    ["taskset", "-cp", corelist, pid], stderr=subprocess.DEVNULL
                )
            except Exception:
                pass
    print("done")
    return 0


def handle_bmark_list(bmark_list):
    """!
    @brief Split the list of benchmark in executable,arguments tuples.
    @param[in] bmark_list The list of benchmark in the `"executable:arg1,arg2,..."` format.
    @returns a list of benchmark in the `["executable","arg1 arg2 ..."]` format.
    """
    new_list = []
    for bmark in bmark_list:
        tmp = bmark.split(":")
        new_list.append((tmp[0], tmp[1].split(",")))
    return new_list


def parser_init():
    """!
    @brief Initialize an argument parser with a set of common arguments.
    @returns The initialized `ArgumentParser` object.
    """
    # set up the argument parser
    parser = argparse.ArgumentParser(
        description="A script to perform a schedulability test"
    )
    parser.add_argument(
        "-u",
        "--utilization-increase",
        metavar="utilization",
        type=float,
        help="How much the utilization should increase at each test step.",
        default=0.1,
        choices=map(lambda x: x / 100.0, range(1, 100)),
        required=False,
        dest="util_inc",
    )

    parser.add_argument(
        "-tn",
        "--tasks-number",
        metavar="tasks-num",
        type=int,
        help="The number of tasks to be executed for each test step.",
        default=100,
        required=False,
        dest="tasks_num",
    )

    parser.add_argument(
        "-wt",
        "--worst-case-tests",
        metavar="tests-num",
        type=int,
        help="The number of tasks to be executed when searching for the worst case runtime.",
        default=1000,
        required=False,
        dest="worst_case_tests",
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
        help="A list of  benchmarks executables, with their arguments. A schedulability test will be performed on each of these benchmarks.\n This option can be specified multiple times.",
        required=True,
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
        default=[],
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
        "-f",
        "--fifo",
        metavar="fifo-prio",
        type=int,
        help="Priority for the SCHED_FIFO scheduler of the target benchmark",
        required=False,
        default=1,
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
        help="Runtime in nanoseconds for the SCHED_DEADLINE scheduler of the target benchmark, will override --fifo.",
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
    return parser


def test_init(parser):
    """!
    @brief Function that will set up the machine for executing the test.
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

    @returns a dictionary called `params`.
    """
    # initialize the dictionary that will be returned
    params = {
        "res": 0,
        "cores": None,
        "int_processes": None,
        "args": None,
        "sched_params": [],
    }
    # parse arguments
    args = parser.parse_args()
    sched_deadline_vals = [
        args.sched_deadline is not None,
        args.sched_runtime is not None,
        args.sched_period is not None,
    ]
    # handle scheduling parameters
    if args.fifo != 1 and any(sched_deadline_vals):
        print("Error: cannot specify options for both SCHED_FIFO and SCHED_DEADLINE")
        return -1
    if any(sched_deadline_vals) and not all(sched_deadline_vals):
        print("Error: missing options for SCHED_DEADLINE")
        return -1
    # we use SCHED_FIFO if nothing from SCHED_DEADLINE has been specified
    if all(sched_deadline_vals) is False:
        sched_params = ["-f", str(args.fifo)]
    else:
        print(sched_deadline_vals)
        sched_params = [
            "-D",
            args.sched_deadline,
            "-P",
            args.sched_period,
            "-T",
            args.sched_runtime,
        ]
    params.update({"sched_params": sched_params})
    # adjust the list of benchmarks and interfering benchmarks
    args.benchmarks = handle_bmark_list(args.benchmarks)
    args.interfering = handle_bmark_list(args.interfering)
    # we detect the physical cores
    cores = detect_cores()
    if cores == -1:
        params.update({"res:": cores})
        return params
    params.update({"cores": cores})
    # we move all processes to the first core
    move_processes(f"0")

    output_len = len(args.output)
    if output_len > 0 and len(args.benchmarks) > output_len:
        print(
            "A single folder has been specified as output path for more than one target benchmark, test will continue but it will overwrite previous files."
        )
    for i in range(0, len(args.benchmarks)):
        if output_len == 0:
            args.output.append(os.path.dirname(args.benchmarks[i][0]))
        elif output_len == len(args.benchmarks):
            subprocess.run(["mkdir", "-p", args.output[i]])
        else:
            if i > 0:
                args.output.append(args.output[0])
    params.update({"args": args})
    return params


def test_teardown(params):
    """!
    @brief The function that will revert the environment to the state it had before the benchmark test.
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
    cores = params.get("cores")
    if cores == None or args == None:
        print("ERROR: mising parameters for test_teardown()")
        return -1
    # we stop the interfering benchmarks if necessary
    if args.interfering != [] and int_processes != None:
        stop_interfering(int_processes)
    # we restore the normal execution of the tasks
    move_processes(f"0-{cores[1]-1}")
    return 0


# Execute the main function when this script is not a module
if __name__ == "__main__":
    parser_obj = parser_init()
    # we add an argument to let the user choose the type of test to execute.
    tests_available = ["WCET", "sched", "WSS"]
    help_str = "Determines the type of test to execute:\n\n\t WCET: Worst Case Execution Test\n\tsched: Schedulability test\n\t WSS: minimum working set size test"
    parser_obj.add_argument(
        "-tt",
        "--test-type",
        metavar="test",
        type=str,
        help=help_str,
        choices=tests_available,
        required=True,
        dest="test",
    )
    test_params = test_init(parser_obj)
    parsed_args = test_params.get("args")
    if parsed_args.test == "WCET":
        import WCET

        WCET.execute(test_params)
    elif parsed_args.test == "sched":
        import schedulability

        schedulability.execute(test_params)
    elif parsed_args.test == "WSS":
        import WSS

        WSS.execute(test_params)
    test_teardown(test_params)
