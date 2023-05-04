#! /bin/python3
"""!
@file overhead.py
@ingroup overhead
@author Mattia Nicolella
@brief Framework overhead measure.
@details


This script will measure the framework overhead by using the overhead.c benchmark.
This script will create some files in the output folder, as described by `overhead_test()`.

Dependencies:
- numpy
- base.py
- graph.py optional

@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
"""

import csv
import os
import subprocess

import numpy as np

import base

try:
    import graph

    graph_imported = True
except ImportError:
    graph_imported = False


def overhead_test(
    tests,
    output,
    prefix,
    postfix,
    last_core,
    sched_params,
    timestamp,
    interference,
    bmark_path=os.path.dirname(__file__),
):
    """!  @brief Finds the framework overhead.

    @param[in] bmark_path The path to the rt-bench utils folder.
    @param[in] tests The number of tests to execute for detecting the worst case scenario execution time.
    @param[in] output The output folder for the generated files.
    @param[in] prefix The prefix to prepend to the generated files.
    @param[in] postfix The postfix to append to the generated files.
    @param[in] last_core The core on which the benchmarks will be executed, usually the last physical core.
    @param[in] sched_params Parameters that tune the benchmark scheduling attributes (a list with CLI options and it's attributes).
    @param[in] timestamp The timestamp of the test.
    @param[in] interference If the test has interference, this will change the csv filename, adding `interfering`.
    @details
    @return history of overheads in seconds.
    """
    bmark = os.path.join(bmark_path, "overhead")
    runtimes_clocks = []
    runtimes_secs = []
    interf_str = ""
    if interference:
        interf_str = "interfering_"
    fname = os.path.join(
        output, prefix + interf_str + "overhead_res" + postfix + ".csv"
    )
    test_fname = prefix + "overhead_test_" + timestamp + postfix + ".csv"
    file_exists = os.path.isfile(fname)
    worst_file = open(fname, "a")
    writer = csv.writer(worst_file)
    print(f"{fname} opened.")
    worst_clocks = 0
    worst_secs = 0
    deadline = 0.001
    if not file_exists:
        writer.writerow(
            [
                "timestamp",
                "min_overhead_in_clock",
                "mean_overhead_in_clock",
                "worst_overhead_in_clock",
                "std_clock",
                "min_overhead_in_seconds",
                "mean_overhead_in_seconds",
                "worst_overhead_in_seconds",
                "std_seconds",
                "tests_number",
                "overheads_clock",
                "overheads_seconds",
            ]
        )
    print("\nstarting overhead test")
    cmdline = [
        bmark,
        "-d",
        str(deadline),
        "-p",
        str(deadline),
        "-l",
        "2",
        "-c",
        str(last_core),
        "-t",
        str(tests),
        "-o",
        os.path.join(
            output,
            test_fname,
        ),
    ] + sched_params
    cmdline = " ".join(cmdline)
    subprocess.run(cmdline, shell=True, check=True)
    try:
        test_file = open(
            os.path.join(
                output,
                test_fname,
            ),
            "r",
        )
    except Exception as e:
        print("Error opening execution test report file", e)
    reader = csv.DictReader(test_file, delimiter=",")
    for row in reader:
        elapsed_secs = float(row["job_elapsed(seconds)"])
        elapsed_clocks = float(row["job_elapsed(clock_cycles)"])
        runtimes_secs.append(elapsed_secs)
        runtimes_clocks.append(elapsed_clocks)
        if elapsed_secs > worst_secs:
            worst_secs = elapsed_secs
            worst_clocks = elapsed_clocks
    test_file.close()
    mean_clocks = np.mean(runtimes_clocks)
    std_clocks = np.std(runtimes_clocks)
    mean_secs = np.mean(runtimes_secs)
    min_secs = min(runtimes_secs)
    min_clocks = min(runtimes_clocks)
    std_secs = np.std(runtimes_secs)
    print("done, test results:")
    print(f"Min ovehead: {min_secs} seconds, {min_clocks} clock cycles")
    print(f"Mean ovehead: {mean_secs} seconds, {mean_clocks} clock cycles")
    print(f"Worst ovehead: {worst_secs} seconds, {worst_clocks} clock cycles")
    print(f"STD: {std_secs} seconds, {std_clocks} clock cycles")
    writer.writerow(
        [
            timestamp,
            min_clocks,
            mean_clocks,
            worst_clocks,
            std_clocks,
            min_secs,
            mean_secs,
            worst_secs,
            std_secs,
            tests,
            base.stringify_list(runtimes_clocks),
            base.stringify_list(runtimes_secs),
        ]
    )
    worst_file.close()
    return runtimes_secs


def execute(params):
    """! @brief Execute the overhead test.

    @param[in,out] params The parameter dictionary provided by `base.test_init()`.
    @details

    In case of success, the `params` dictionary will be updated with a new key, `overheads`, which will contain the list of
    overheads measured.

    A graph of all the test will be produced in each output folder in png and svg formats.

    @returns The updated `params` dictionary with `{"res":0}` on success, or `{"res":-1}` on failure.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the overhead test!")
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
            print("ERROR: Missing scheduling parameters to execute the overhead test!")
            params.update({"res": -1})
            return params
        cores = params.get("cores")
        if cores is None:
            print("ERROR: Missing corelist to execute the overhead test!")
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
        # get the list of overheads
        exec_times = overhead_test(
            args["tasks_num"],
            args["output"][0],
            args["prefix"],
            args["postfix"],
            args["target_core"],
            sched_params,
            timestamp,
            len(args["interfering"]) > 0,
        )
        params.update(
            {
                "res": 0,
                "Overhead": {
                    "min_overhead_in_seconds": min(exec_times),
                    "worst_in_seconds": max(exec_times),
                    "mean_in_seconds": np.mean(exec_times),
                    "std_in_seconds": np.std(exec_times),
                    "runtimes_in_seconds": exec_times,
                },
            }
        )
    if args["draw_graph"] != "no":
        params = base.draw_and_save_graph(
            draw_graph,
            params,
            "Overhead",
            "Overhead",
            ["runtimes_in_seconds"],
            [convert_read_runtimes],
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
    log_scale = False
    if runtimes is None:
        print("ERROR: Not enough data to plot a WCET graph")
        return None
    WCET_graph = graph.violinplot(
        runtimes,
        ylabel="Runtime (seconds)",
        title="Overhead" if not interference else " Overhead with interference",
        log_scale=log_scale,
    )
    return WCET_graph


if __name__ == "__main__":
    parser = base.parser_init("A script to perform a framework overhead test")
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
