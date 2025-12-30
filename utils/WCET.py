#! /bin/python3
"""!
@file WCET.py
@ingroup wcet
@author Mattia Nicolella
@brief Procedures for a benchmark worst case execution test (WCET)

This script will execute a worst case execution test on the given benchmark by:
- Using `base.test_init()` to initialize the environment of the test
- Calling `worst_case_exec_test()` which will execute the test
- Restore the environment status using `base.test_teardown()`

This script will create some files in the output folder, as described by `worst_case_execution_test()`.

Dependencies:
- base.py
- graph.py optional

@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
"""


import csv
import os
import subprocess

import base

try:
    import graph

    graph_imported = True
except ImportError:
    graph_imported = False


def worst_case_exec_test(
    bmark,
    bmark_args,
    worst_case_tests,
    output,
    prefix,
    postfix,
    last_core,
    sched_params,
    timestamp,
    interference,
    threshold=0,
    deadline=0.001,
):
    """!  @brief Finds the worst case execution time using only the first core.

    @param[in] bmark The target benchmark.
    @param[in] bmark_args The arguments for the target benchmark.
    @param[in] worst_case_tests The number of tests to execute for detecting the worst case scenario execution time.
    @param[in] output The output folder for the generated files.
    @param[in] prefix The prefix to prepend to the generated files.
    @param[in] postfix The postfix to append to the generated files.
    @param[in] last_core The core on which the benchmarks will be executed, usually the last physical core.
    @param[in] sched_params Parameters that tune the benchmark scheduling attributes (a list with CLI options and it's attributes).
    @param[in] timestamp The timestamp of the test.
    @param[in] interference If the test has interference, this will change the csv filename, adding `interfering`.
    @param[in] threshold The percentage of missed deadline that can make the test still succeed.
    @param[in] deadline The minimum deadline for the WCET test.
    @details
    The worst case execution time will be the longest time a single task has run without missing the deadline.
    To do so a number of tasks (`worst_case_tests`) is run and if at least one misses the deadline then the deadline be increased and the test restarted.
    After the all tasks have completed their execution, the output file will be scanned to find the maximum execution time.

    The maximum execution time will be saved in clock cycles and in seconds.
    The found worst case execution time will be exported into a file called `[benchmark executable name]_worst_case_exec.csv`
    All the taken test will be exported into a file called `[benchmark executable name]_worst_case_exec_test.csv`.

    If previous test files are found then the test if performed and the new WCET will be computed including also the previous WCET.
    However, only the tests taken in the current executions can be found in `[benchmark executable name]_worst_case_exec_test.csv`.
    @returns The found worst case execution time in seconds or -1 in case of error.
    """
    if threshold > 1:
        print("ERROR: The threshold for failed tests is greater than 100%")
        return -1
    if deadline < 0:
        print("ERROR: The  deadline for failed tests is less than 0")
        return -1

    fails_count = worst_case_tests * threshold + 1
    bmark_name = os.path.basename(bmark)
    runtimes = []
    interf_str = ""
    if interference:
        interf_str = "interfering_"
    fname = os.path.join(
        output, prefix + interf_str + "worst_case_exec_res" + postfix + ".csv"
    )
    test_fname = prefix + "worst_case_exec_test_" + timestamp + postfix + ".csv"
    file_exists = os.path.isfile(fname)
    worst_file = open(fname, "a")
    writer = csv.writer(worst_file)
    print(f"{fname} opened.")
    worst = 0
    worst_time = 0
    if not file_exists:
        writer.writerow(
            [
                "timestamp",
                "benchmark",
                "arguments",
                "worst_in_clock",
                "worst_in_seconds",
                "runtimes_in_seconds",
                "acceptance_threshold",
                "tests_number",
            ]
        )
    print(f"\nstarting worst case execution test for {bmark_name} {bmark_args}")
    while fails_count > worst_case_tests * threshold:
        print(
            f"deadline value: {deadline}, current WCET: {worst_time}, executing {worst_case_tests} tasks"
        )
        fails_count = 0
        runtimes = []
        bmark_cmdline = (
            [
                os.path.abspath(bmark),
                "-d",
                str(deadline),
                "-p",
                str(deadline),
                "-l",
                "2",
                "-c",
                str(last_core),
                "-t",
                str(worst_case_tests),
                "-o",
                os.path.abspath(
                    os.path.join(
                        output,
                        test_fname,
                    )
                ),
            ]
            + sched_params
            + ["-b"]
            + [f'"{" ".join(bmark_args)}"']
        )
        bmark_cmdline = " ".join(bmark_cmdline)
        try:
            subprocess.run(bmark_cmdline, check=True, shell=True)
            test_file = open(
                os.path.join(
                    output,
                    test_fname,
                ),
                "r",
            )
        except Exception as e:
            print("Error opening worst case execution test report file", e)
            return -1, -1
        reader = csv.DictReader(test_file, delimiter=",")
        worst_time = 0
        for row in reader:
            elapsed_secs = float(row["job_elapsed(seconds)"])
            elapsed_clocks = float(row["job_elapsed(clock_cycles)"])
            deadline = int(row["deadline_status(1=met)"])
            if deadline == 0:
                fails_count += 1
            runtimes.append(elapsed_secs)
            if elapsed_secs > worst_time:
                worst_time = elapsed_secs
                worst = elapsed_clocks
        test_file.close()
        if fails_count > worst_case_tests * threshold:
            print(
                f"{fails_count} deadlines were missed increasing deadline threshold: {threshold}, test_performed:{worst_case_tests} missed deadlines:{fails_count} max missable deadlines {threshold * worst_case_tests}"
            )
            deadline = worst_time
    print(
        f"done, test results:{worst} clock cycles {worst_time} seconds\n threshold: {threshold}, test_performed:{worst_case_tests} missed deadlines:{fails_count} max missable deadlines {threshold * worst_case_tests}"
    )
    writer.writerow(
        [
            timestamp,
            bmark,
            base.stringify_list(bmark_args),
            worst,
            worst_time,
            base.stringify_list(runtimes),
            threshold,
            worst_case_tests,
        ]
    )
    worst_file.close()
    return worst_time, runtimes


def execute(params):
    """! @brief Execute the WCET test on the given benchmarks.

    @param[in,out] params The parameter dictionary provided by
    `base.test_init()`.

    @details

    In case of success, the `params` dictionary will be updated with a new key, `worst_runtimes`, which will contain the list of
    WCETs for the given benchmarks (in the same order of the benchmark list in `params["args"].benchmarks`).

    A graph of all the test will be produced in each output folder in png and svg formats.

    @returns The updated `params` dictionary with `{"res":0}` on success, or `{"res":-1}` on failure.

    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the WCET test!")
        params.update({"res": -1})
        return params
    if args["draw_graph"] != "only":
        timestamp = params.get("timestamp")
        if timestamp is None:
            print("ERROR: Missing test timestamp!")
            params.update({"res": -1})
            return params
        sched_params = params.get("sched_params")
        if sched_params is None:
            print(
                "ERROR: Missing scheduling parameters to execute the schedulability test!"
            )
            params.update({"res": -1})
            return params
        cores = params.get("cores")
        if cores is None:
            print("ERROR: Missing corelist to execute the WCET test!")
            params.update({"res": -1})
            return params
        int_processes = None
        if args["interfering"] != []:
            # if the user requested it, we start the interfering benchmarks
            int_processes = base.start_interfering(
                0.001,
                args["interfering"],
                cores[0],
                args["target_core"],
                args["system_core"],
                args["fifo_interfering"],
            )
            if int_processes == [] and params.get("int_processes") is None:
                print("ERROR: cannot start interfering processes, aborting")
                params.update({"res": -1})
                return params
        params.update({"int_processes": int_processes})
        # get the list of worst case runtimes
        worst_runtimes = []
        runtimes = []
        bmarks = []
        for i in range(0, len(args["benchmarks"])):
            WCET, exec_times = worst_case_exec_test(
                args["benchmarks"][i][0],
                args["benchmarks"][i][1],
                args["worst_case_tests"],
                args["output"][i],
                args["prefix"],
                args["postfix"],
                args["target_core"],
                sched_params,
                timestamp,
                len(args["interfering"]) > 0,
                args["worst_case_threshold"],
                args["worst_case_deadline"],
            )
            if WCET < 0:
                params.update({"res:": WCET})
                return params
            worst_runtimes.append(WCET)
            runtimes.append(exec_times)
            bmarks.append(
                os.path.basename(args["benchmarks"][i][0])
                + "\n"
                + os.path.basename(args["benchmarks"][i][1][0])
            )
        params.update(
            {
                "res": 0,
                "WCET": {
                    "worst_in_seconds": worst_runtimes,
                    "legend": bmarks,
                    "runtimes_in_seconds": runtimes,
                },
            }
        )
    if args["draw_graph"] != "no":
        params = base.draw_and_save_graph(
            draw_graph,
            params,
            "WCET",
            "WCET",
            ["worst_in_seconds", "runtimes_in_seconds"],
            [float, convert_read_runtimes],
        )
    return params


def convert_read_runtimes(data):
    return list(map(float, base.unstringify_list(data)))


def draw_graph(data, interference=False, old_graph=None):
    """! @brief Draw a violin plot.

    @param[in] data The data that needs to be plotted.
    @param[in] interference If there is interference in the graph, this will only change the graph title.
    @param[in] old_graph A previous graph object on which lines will be added.
    @returns The graph object.
    """
    runtimes = data.get("runtimes_in_seconds")
    if hasattr(runtimes[0], "__iter__"):
        if hasattr(runtimes[0][0], "__iter__"):
            tmp_list = []
            for elem in runtimes:
                tmp_list.append(elem[0])
                runtimes = tmp_list
    legend = data.get("legend")
    # min_data = min(list(map(min, runtimes)))
    # max_data = max(list(map(max, runtimes)))
    # log_scale = abs(max_data / min_data) > 100
    log_scale = False
    bmarks = []
    for elem in legend:
        bmarks.append(elem.replace(" - ", "\n"))
    if runtimes is None:
        print("ERROR: Not enough data to plot a WCET graph")
        return None
    WCET_graph = graph.violinplot(
        runtimes,
        labels=bmarks,
        ylabel="Runtime (seconds)",
        title="Worst case execution time"
        if not interference
        else "Worst case execution time with interference",
        log_scale=log_scale,
    )
    return WCET_graph


if __name__ == "__main__":
    parser = base.parser_init("A script to perform a worst case execution time test")
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
