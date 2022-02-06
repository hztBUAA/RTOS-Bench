#! /bin/python3
"""!
@file schedulability.py
@ingroup utils
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
"""


import os
import subprocess
import csv
import datetime
import base
import WCET
import graph


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
):
    """!
    @brief schedulability test for a single benchmark.
    @param[in] bmark The benchmark executable.
    @param[in] bmark_args Arguments for the target benchmark.
    @param[in] worst_case Worst case execution time of the target benchmark.
    @param[in] util_inc Increase of the utilization (percentage) at every step of the test.
    @param[in] tasks_num Number of tasks to execute at each step of the test.
    @param[in] output Output folder of for the data files.
    @param[in] prefix Prefix to prepend to all generated files.
    @param[in] postfix Postfix to append to all generated files.
    @param[in] last_core The core where the target benchmark will be executed, should be the last physical core.
    @param[in] sched_params Parameters that tune the benchmark scheduling attributes (a list with CLI options and it's attributes).
    @details
    The test starts with a deadline equal to the worst case execution time, the this deadline will progressively decrease by a percentage (given by the user) until it reaches 0.
    For each of these steps the task execution will be aggregated in a mean and the number of tasks that did complete without missing the deadline are recorded, along with the number
    of tasks launched per step.

    Test is executed on the last available physical core after the environment has been prepared by `base.test_init()`.

    The benchmark is instructed to log data in a set of files called: `[benchmark executable name]_sched_test_x.csv` where `x` is the expected utilization percentage, while the number of scheduled processes at each step will be recorded in `[benchmark executable name]_sched_test_res.csv`.
    @returns a dictionary with two keys (`utilization`) and (`num_scheduled`) on success, `None` on error.
    """
    res = {}
    res_util = []
    res_sched = []
    deadline = worst_case
    utilization = 1 - (deadline / worst_case)
    bmark_name = os.path.basename(bmark)
    try:
        res_file = open(
            os.path.join(output, prefix + "sched_test_res" + postfix + ".csv"),
            "a",
        )
    except Exception as e:
        print("Cannot open file for storing schedulability test results ", e)
        return None
    writer = csv.writer(res_file)
    writer.writerow(
        [
            "timestamp",
            "benchmark",
            "arguments",
            "utilization",
            "mean_utilization",
            "successfully_scheduled",
            "total_started",
        ]
    )
    print(f"\n\nStarting schedulability test for {bmark_name}")
    while deadline >= 1e-9:
        sum_utilization = 0
        print(
            f"\ntest  with {utilization*100}% utilization, deadline: {deadline} seconds"
        )
        log_fname = os.path.join(
            output, f"{prefix}sched_test_{utilization:.3g}{postfix}.csv"
        )
        try:
            subprocess.run(
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
                + ["-b"]
                + bmark_args
            )
        except Exception as e:
            print("Error during schedulability test ", e)
            return None
        scheduled = 0
        started = 0
        try:
            log_file = open(log_fname)
        except Exception as e:
            print("Cannot open benchmark result file ", e)
            return None
        reader = csv.DictReader(log_file)
        next(reader)
        for row in reader:
            started += 1
            if int(row["deadline_status(1=met)"]) == 1:
                scheduled += 1
            sum_utilization += float(row["job_utilization"])
        log_file.close()
        print(
            f"successfully scheduled {scheduled} over {started} tasks ({scheduled/started*100}%), mean utilization: {sum_utilization/started}"
        )
        writer.writerow(
            [
                datetime.datetime.now(),
                bmark,
                bmark_args,
                str(utilization),
                str(sum_utilization / started),
                str(scheduled),
                str(started),
            ]
        )
        res_util.append(utilization)
        res_sched.append(scheduled)
        deadline = deadline - (util_inc * worst_case)
        utilization = 1 - (deadline / worst_case)
    res_file.close()
    res.update({"utilization": res_util})
    res.update({"num_scheduled": res_sched})
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
    # if the user requested it, we start the interfering benchmarks
    params = WCET.execute(params)
    worst_runtimes = params.get("worst_runtimes")
    if worst_runtimes is None:
        print("ERROR: Missing WCETs to execute the schedulability test!")
        params.update({"res": -1})
        return params
    int_processes = None
    if args.interfering != []:
        int_processes = base.start_interfering(
            min(worst_runtimes), args.interfering, cores[0]
        )
        if int_processes == [] and params.get("int_processes") is None:
            print("ERROR: cannot start interfering processes, aborting")
            params.update({"res": -1})
            return params
    params.update({"int_processes": int_processes})
    # we start the schedulability test for each benchmark
    last_core = cores[0] - 1
    sched_graph = None
    graph_lines = []
    sched_utilization = []
    sched_num_scheduled = []
    for i in range(0, len(args.benchmarks)):
        res = sched_test(
            args.benchmarks[i][0],
            args.benchmarks[i][1],
            worst_runtimes[i],
            args.util_inc,
            args.tasks_num,
            args.output[i],
            args.prefix,
            args.postfix,
            last_core,
            sched_params,
        )
        if res is None:
            params.update({"res": -1})
            break
        graph_lines.append(
            os.path.basename(args.benchmarks[i][0])
            + " - "
            + os.path.basename(args.benchmarks[i][1][0])
        )
        sched_utilization.append(res.get("utilization"))
        sched_num_scheduled.append(res.get("num_scheduled"))
    sched_graph = graph.plot(
        sched_utilization,
        sched_num_scheduled,
        "Utilization",
        "Scheduled",
        "Schedulability test"
        if len(args.interfering) == 0
        else "Schedulability test with interference",
        graph_lines,
    )
    for output in args.output:
        graph.export_graph(
            sched_graph,
            os.path.join(output, args.prefix + "sched"),
        )
    graph.teardown()
    params.update({"res": 0})
    return params


if __name__ == "__main__":
    parser = base.parser_init()
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
