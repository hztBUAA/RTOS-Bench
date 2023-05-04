#!/bin/python3
"""!
@file WSS.py
@ingroup wss
@brief Test to compute the minimum working set size
@details

This script will execute a minimum working size test by doing the following:
- Use `base.test_init()` to initialize the enviroment.
- Exceute the minimum WSS test
- Cleanup the enviroment by calling `base.test_teardown()`

As a test result some files will be created, these are described `wss_test()`

Dependecies:
- base.py
- graph.py optional

@author Mattia Nicolella

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


def execute(params):
    """!
    @brief Execute the working set size test on the given benchmarks.
    @param[in,out] params Parameters dictionary provided by `base.test_init()`.
    @details Will call `wss_test()` for each element of `params.args.benchmarks`.
    A graph of the test will be produced in each output folder in both png and svg formats.
    @returns The updated `params` dictionary with {"res":0} on success, or {"res":-1} on failure.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the wss test!")
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
            print("ERROR: Missing scheduling parameters to execute the wss test!")
            params.update({"res": -1})
            return params
        cores = params.get("cores")
        if cores is None:
            print("ERROR: Missing corelist to execute the wss test!")
            params.update({"res": -1})
            return params
        bmark_wss = []
        bmarks = []
        for i in range(0, len(args["benchmarks"])):
            res = wss_test(
                args["benchmarks"][i][0],
                args["benchmarks"][i][1],
                args["tasks_num"],
                args["output"][i],
                args["prefix"],
                args["postfix"],
                args["target_core"],
                sched_params,
                timestamp,
            )
            if res < 0:
                break
            else:
                bmark_wss.append(res)
                bmarks.append(
                    os.path.basename(args["benchmarks"][i][0])
                    + "\n"
                    + os.path.basename(args["benchmarks"][i][1][0])
                )
                res = 0
            params.update(
                {
                    "res": res,
                    "WSS": {
                        "minimum_wss(bytes)": bmark_wss,
                        "legend": bmarks,
                    },
                }
            )
    if args["draw_graph"] != "no":
        params = base.draw_and_save_graph(
            draw_graph,
            params,
            "WSS",
            "WSS",
            ["minimum_wss(bytes)"],
            conv=[int],
        )
    return params


def draw_graph(data, interference=False, old_graph=None):
    """! @brief Draws a bar graph.

    @param[in] data The data that needs to be plotted.
    @param[in] interference If there is interference in the graph, this will only change the graph title
    @param[in] old_graph A previous graph object on which the bars will be added.
    @returns The graph object
    """
    bmark_wss = data.get("minimum_wss(bytes)")
    # if we get a list of lists we flatten in to a single list.
    if hasattr(bmark_wss[0], "__iter__"):
        tmp_list = []
        for elem in bmark_wss:
            tmp_list.append(elem[0])
        bmark_wss = tmp_list
    legend = data.get("legend")
    bmarks = []
    for elem in legend:
        bmarks.append(elem.replace(" - ", "\n"))
    min_data = min(bmark_wss)
    max_data = max(bmark_wss)
    log_scale = abs(max_data / min_data) >= 10**2
    WSS_graph = graph.bar(
        bmark_wss,
        bmarks,
        "Minimum working set size (bytes)",
        "Working set size",
        log_scale=log_scale,
        graph=old_graph,
    )
    return WSS_graph


def wss_test(
    bmark,
    bmark_args,
    tests,
    output,
    prefix,
    postfix,
    core,
    sched_params,
    timestamp,
):
    """! @brief Perform a minimum working set size test.

    @param[in] bmark Benchmark on which the test should be executed.
    @param[in] bmark_args Benchmark arguments.
    @param[in] tests Number of tests to execute for each working set size.
    @param[in] output The output folder.
    @param[in] prefix The generated file prefix.
    @param[in] postfix The generated file postfix.
    @param[in] core Physical core on which the test will be executed.
    @param[in] sched_params Scheduling attributes.
    @param[in] timestamp The timestamp of the test.
    @details The function will run a number of tests, specified in `params` constraining the benchmark available memory to 1KB.
    If all the test succeed the available memory limit will be halved. Otherwise the memory limit will be doubled.
    The execution will stop when the smallest amount of memory to run a benchmark is determined.

    Results will be saved in a file called: `working_set_size_test.csv`

    @returns the minimum wss in bytes, failures in the benchmark execution treated as a wrong wss size.
    """
    current_wss = 1
    last_wss = 0
    wss_lower_bound = 1
    wss_upper_bound = 0
    failed_tests = 0
    bmark_name = os.path.basename(bmark)
    filename = os.path.join(output, prefix + "min_wss_test" + postfix + ".csv")
    while current_wss != last_wss:
        print(
            f"\n\n{bmark_name} {bmark_args} current wss:{current_wss}, last wss:{last_wss}"
        )
        failed_tests = 0
        bmark_cmdline = (
            [
                bmark,
                "-d",
                "1",
                "-p",
                "1",
                "-l",
                "1",
                "-c",
                str(core),
                "-t",
                str(tests),
                "-m",
                str(current_wss),
            ]
            + sched_params
            + ["-b", f'"{" ".join(bmark_args)}"']
        )
        bmark_cmdline = " ".join(bmark_cmdline)
        try:
            subprocess.run(bmark_cmdline, check=True, shell=True)
        except subprocess.CalledProcessError as e:
            # we failed at least one test
            failed_tests += 1
            wss_lower_bound = current_wss
            print(f"Cannot run WSS test {e}")
        else:
            # all tests succeeded, so we can save this upper bound
            wss_upper_bound = current_wss
        # if we already have an upper bound we try to reduce it
        last_wss = current_wss
        if wss_upper_bound > 0:
            sum_wss = wss_lower_bound + wss_upper_bound
            current_wss = sum_wss // 2
        else:
            # if we don't have an upper bound we double the current wss
            current_wss = last_wss * 2
        print(f"\nnext wss {current_wss} last wss {last_wss}")
        print(f"wss lower bound {wss_lower_bound}  wss upper bound {wss_upper_bound}")
    file_exists = os.path.isfile(filename)
    with open(filename, "a") as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(
                ["timestamp", "benchmark", "arguments", "minimum_wss(bytes)"]
            )
        writer.writerow(
            [timestamp, bmark, base.stringify_list(bmark_args), current_wss]
        )
    return current_wss


if __name__ == "__main__":
    parser = base.parser_init("A script to perform working set size test")
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
