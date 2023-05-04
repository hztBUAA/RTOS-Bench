#! /bin/python3
"""!
@file schedulability.py
@ingroup sched
@author Mattia Nicolella
@brief Procedures for a benchmark schedulability test

This script will execute a schedulability test on the given benchmark by:
- Using `base.test_init()` to initialize the environment of the test
- Execute a preliminary WCET test through `WCET.execute()`
- Start the interfering benchmarks if the user has supplied them through `base.start_interfering()`
- Execute the schedulability test
- Restore the environment status using `base.test_teardown()`

This test will create several csv files as described in `sched_test()` and `WCET.worst_case_exec_test()`.

Dependencies:
- base.py
- WCET.py
- graph.py optional

@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
"""


import csv
import os
import subprocess

import WCET

import base

try:
    import graph

    graph_imported = True
except ImportError:
    graph_imported = False


def sched_test(
    bmark,
    bmark_args,
    worst_case,
    util_inc,
    tasks_num,
    output,
    prefix,
    postfix,
    last_core,
    sched_params,
    timestamp,
    interference,
):
    """!
    @brief schedulability test for a single benchmark.
    @param[in] bmark The benchmark executable.
    @param[in] bmark_args Arguments for the target benchmark.
    @param[in] worst_case Worst case execution time of the target benchmark.
    @param[in] util_inc Increase of the utilization (percentage) at every step of the test. Also used to set the minimum utilization during the test.
    @param[in] tasks_num Number of tasks to execute at each step of the test.
    @param[in] output Output folder of for the data files.
    @param[in] prefix Prefix to prepend to all generated files.
    @param[in] postfix Postfix to append to all generated files.
    @param[in] last_core The core where the target benchmark will be executed, should be the last physical core.
    @param[in] sched_params Parameters that tune the benchmark scheduling attributes (a list with CLI options and it's attributes).
    @param[in] timestamp The timestamp of the test.
    @param[in] interference If the test has interference, this will change the csv filename, adding `interfering`.
    @details
    The test starts with a deadline equal to the worst case execution time, the this deadline will progressively decrease by a percentage (given by the user) until it reaches 0.
    For each of these steps the task execution will be aggregated in a mean and the number of tasks that did complete without missing the deadline are recorded, along with the number
    of tasks launched per step.

    Test is executed on the last available physical core after the environment has been prepared by `base.test_init()`.

    The benchmark is instructed to log data in a set of files called: `[benchmark executable name]_sched_test_x.csv` where `x` is the expected utilization percentage, while the number of scheduled processes at each step will be recorded in `[benchmark executable name]_sched_test_res.csv`.
    @returns a dictionary with two keys (`utilization`) and (`successfully_scheduled`) on success, `None` on error.
    """
    res = {}
    res_util = []
    res_sched = []
    utilization = util_inc
    deadline = worst_case / utilization
    bmark_name = os.path.basename(bmark)
    interf_str = ""
    if interference:
        interf_str = "interfering_"
    res_fname = os.path.join(
        output, prefix + interf_str + "sched_test_res" + postfix + ".csv"
    )
    file_exists = os.path.isfile(res_fname)
    try:
        res_file = open(res_fname, "a")
    except Exception as e:
        print("Cannot open file for storing schedulability test results ", e)
        return None
    writer = csv.writer(res_file)
    if not file_exists:
        writer.writerow(
            [
                "timestamp",
                "benchmark",
                "arguments",
                "utilization",
                "mean_utilization",
                "successfully_scheduled",
                "total_started",
                "periods_elapsed",
            ]
        )
    print(f"\n\nStarting schedulability test for {bmark_name} {bmark_args}")
    while utilization <= 1:
        sum_utilization = 0
        print(
            f"\ntest  with {100*utilization:.3g}% utilization, deadline: {deadline} seconds"
        )
        log_fname = os.path.join(
            output, f"{prefix}sched_test_{utilization:.3g}{postfix}.csv"
        )
        bmark_cmdline = (
            [
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
                str(tasks_num),
                "-o",
                log_fname,
            ]
            + sched_params
            + ["-b", f'"{" ".join(bmark_args)}"']
        )
        bmark_cmdline = " ".join(bmark_cmdline)
        try:
            subprocess.run(bmark_cmdline, shell=True, check=True)
        except Exception as e:
            print("Error during schedulability test ", e)
            return None
        scheduled = 0
        periods = 0
        try:
            log_file = open(log_fname)
        except Exception as e:
            print("Cannot open benchmark result file ", e)
            return None
        reader = csv.DictReader(log_file)
        for row in reader:
            periods += 1
            if int(row["deadline_status(1=met)"]) == 1:
                scheduled += 1
            sum_utilization += float(row["job_utilization"])
        log_file.close()
        print(
            f"successfully scheduled {scheduled} over {tasks_num} tasks ({scheduled/tasks_num*100}%) in {periods} periods, mean utilization: {sum_utilization/periods}"
        )
        args_str = base.stringify_list(bmark_args)
        writer.writerow(
            [
                timestamp,
                bmark,
                args_str,
                str(utilization),
                str(sum_utilization / tasks_num),
                str(scheduled),
                str(tasks_num),
                str(periods),
            ]
        )
        res_util.append(utilization)
        res_sched.append(scheduled)

        utilization = round(utilization + util_inc, 3)
        deadline = worst_case / utilization
    res_file.close()
    res.update({"utilization": res_util})
    res.update({"successfully_scheduled": res_sched})
    return res


def execute(params):
    """!
    @brief Execute the schedulability test on the given benchmarks
    @param[in,out] params The parameter dictionary provided by `base.test_init()` plus the array of WCETs provided by `WCET.execute()`.
    @details
    If the user requested interfering benchmarks, then the `params` dictionary will be updated with the key `int_processes`, which will contain the list
    of interfering processes.
    A graph of the test will be produced in each output folder in png and svg formats.
    @returns The updated `params` dictionary with {"res":0} on success, or {"res":-1} on failure.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the schedulability test!")
        params.update({"res": -1})
        return params
    if args["draw_graph"] != "only":
        timestamp = params.get("timestamp")
        if timestamp is None:
            print("ERROR: Missing test timestamp!")
            params.update({"res": -1})
            return params
        cores = params.get("cores")
        if cores is None:
            print("ERROR: Missing corelist to execute the schedulability test!")
            params.update({"res": -1})
            return params
        sched_params = params.get("sched_params")
        if sched_params is None:
            print(
                "ERROR: Missing scheduling parameters to execute the schedulability test!"
            )
            params.update({"res": -1})
            return params
        # we want a WCET without interference
        interfering = args["interfering"].copy()
        args["interfering"].clear()
        params = WCET.execute(params)
        # but we want interference in the scheudulability test
        args["interfering"].extend(interfering)
        WCET_dict = params.get("WCET")
        if WCET_dict is None:
            print("ERROR: Missing WCET dictionary to execute the schedulability test!")
            params.update({"res": -1})
            return params
        worst_runtimes = WCET_dict.get("worst_in_seconds")
        if worst_runtimes is None:
            print("ERROR: Missing WCETs to execute the schedulability test!")
            params.update({"res": -1})
            return params
        int_processes = None
        if args["interfering"] != []:
            # if the user requested it, we start the interfering benchmarks
            int_processes = base.start_interfering(
                min(worst_runtimes),
                interfering,
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
        # we start the schedulability test for each benchmark
        graph_lines = []
        sched_utilization = []
        sched_num_scheduled = []
        for i in range(0, len(args["benchmarks"])):
            res = sched_test(
                args["benchmarks"][i][0],
                args["benchmarks"][i][1],
                worst_runtimes[i],
                args["util_inc"],
                args["tasks_num"],
                args["output"][i],
                args["prefix"],
                args["postfix"],
                args["target_core"],
                sched_params,
                timestamp,
                len(args["interfering"]) > 0,
            )
            if res is None:
                params.update({"res": -1})
                break
            graph_lines.append(
                os.path.basename(args["benchmarks"][i][0])
                + " - "
                + os.path.basename(args["benchmarks"][i][1][0])
            )
            sched_utilization.append(res.get("utilization"))
            sched_num_scheduled.append(res.get("successfully_scheduled"))
        params.update(
            {
                "res": 0,
                "sched": {
                    "utilization": sched_utilization,
                    "successfully_scheduled": sched_num_scheduled,
                    "legend": graph_lines,
                },
            }
        )
    if args["draw_graph"] != "no":
        params = base.draw_and_save_graph(
            draw_graph,
            params,
            "sched",
            "sched",
            ["utilization", "successfully_scheduled"],
            conv=[float, int],
        )
    return params


def draw_graph(data, interference=False, old_graph=None):
    """! @brief Draw a plot.

    @param[in] data The data that needs to be plotted.
    @param[in] interference If there is interference in the graph, this will only change the graph title
    @param[in] old_graph A previous graph object on which lines will be added.
    @returns The graph object
    """
    sched_utilization = data.get("utilization")
    sched_num_scheduled = data.get("successfully_scheduled")
    legend = data.get("legend")
    if sched_utilization is None or sched_num_scheduled is None:
        print("ERROR: Not enough data to plot a schedulability graph")
        return None
    sched_graph = graph.plot(
        sched_utilization,
        sched_num_scheduled,
        "Utilization",
        "Num. Scheduled",
        "Schedulability test"
        if not interference
        else "Schedulability test with interference",
        legend,
        graph=old_graph,
    )
    return sched_graph


if __name__ == "__main__":
    parser = base.parser_init("A script to perform a schedulability test")
    params = base.test_init(parser)
    if params["res"] < 0:
        exit(params["res"])
    execute(params)
    base.test_teardown(params)
