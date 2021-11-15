#! /bin/python3
"""!
@file WCET.py
@ingroup utils
@author Mattia Nicolella
@brief Procedures for a benchmark worst case execution test (WCET)

This script will execute a schedulability test on the given benchmark by:
- Using `base.test_init()` to initialize the environment of the test
- Execute a WCET test
- Restore the environment status using `base.test_teardown()`

This script will create some files in the output folder, as described by `worst_case_execution_test()`.

Dependencies:
- base.py
"""


import csv
import os
import subprocess

import base


def worst_case_exec_test(
    bmark,
    bmark_args,
    worst_case_tests,
    output,
    prefix,
    postfix,
    last_core,
    sched_params,
):
    """!
    @brief Finds the worst case execution time using only the first core.
    @param[in] bmark The target benchmark.
    @param[in] bmark_args The arguments for the target benchmark.
    @param[in] worst_case_tests The number of tests to execute for detecting the worst case scenario execution time.
    @param[in] output The output folder for the generated files.
    @param[in] prefix The prefix to prepend to the generated files.
    @param[in] postfix The postfix to append to the generated files.
    @param[in] last_core The core on which the benchmarks will be executed, usually the last physical core.
    @param[in] sched_params Parameters that tune the benchmark scheduling attributes (a list with CLI options and it's attributes).
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
    deadline = 0.001
    fails_count = 1
    bmark_name = os.path.basename(bmark)
    try:
        worst_file = open(
            os.path.join(
                output, prefix + bmark_name + "_worst_case_exec" + postfix + ".csv"
            )
        )
        prev_reader = csv.reader(worst_file)
        next(prev_reader)
        row = next(prev_reader)
        worst = int(row[0])
        worst_time = float(row[1])
        worst_file.close()
        print(f"read {worst} clock cycles : {worst_time} seconds")
    except Exception as e:
        print(
            f"{prefix}{bmark_name}_worst_case_exec{postfix}.csv not found, creating it",
            e,
        )
        worst = 0
        worst_time = 0
    worst_file = open(
        os.path.join(
            output, prefix + bmark_name + "_worst_case_exec" + postfix + ".csv"
        ),
        "w",
    )
    writer = csv.writer(worst_file)
    writer.writerow(["benchmark", "worst_in_clock", "worst_in_seconds"])
    print(f"\nstarting worst case execution test for {bmark_name}")
    while fails_count > 0:
        print(
            f"deadline value: {deadline}, current WCET: {worst_time}, executing {worst_case_tests} tasks"
        )
        fails_count = 0
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
                str(worst_case_tests),
                "-o",
                os.path.join(
                    output,
                    prefix + bmark_name + "_worst_case_exec_test" + postfix + ".csv",
                ),
            ]
            + sched_params
            + ["-b"]
            + bmark_args
        )
        try:
            test_file = open(
                os.path.join(
                    output,
                    prefix + bmark_name + "_worst_case_exec_test" + postfix + ".csv",
                ),
                "r",
            )
        except Exception as e:
            print("Error opening worst case execution test report file", e)
            return -1
        reader = csv.reader(test_file, delimiter=",")
        # skip the header
        next(reader)
        for row in reader:
            if int(row[10]) == 0 and float(row[9]) != 0:
                fails_count += 1
            if float(row[9]) > worst_time:
                worst_time = float(row[9])
                worst = int(row[4])
        test_file.close()
        if fails_count > 0:
            print(f"{fails_count} benchmark failed, increasing deadline")
            deadline *= 10
    print(f"done, test results:{worst} clock cycles {worst_time} seconds\n")
    writer.writerow([bmark_name, worst, worst_time])
    worst_file.close()
    return worst_time


def execute(params):
    """!
    @brief Execute the WCET test on the given benchmarks
    @param[in,out] params The parameter dictionary provieded by `base.test_init()`.
    @details

    In case of success, the `params` dictionary will be updated with a new key, `worst_runtimes`, which will contain the list of
    WCETs for the given benchmarks (in the same order of the benchmark list in `params["args"].benchmarks`).

    @returns The updated `params` dictionary with `{"res":0}` on success, or `{"res":-1}` on failure.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the WCET test!")
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
    last_core = cores[0] - 1
    # get the list of worst case runtimes
    worst_runtimes = []
    for i in range(0, len(args.benchmarks)):
        WCET = worst_case_exec_test(
            args.benchmarks[i][0],
            args.benchmarks[i][1],
            args.worst_case_tests,
            args.output[i],
            args.prefix,
            args.postfix,
            last_core,
            sched_params,
        )
        if WCET < 0:
            params.update({"res:": WCET})
            return params
        worst_runtimes.append(WCET)
    params.update({"res": 0, "worst_runtimes": worst_runtimes})
    return params


if __name__ == "__main__":
    parser = base.parser_init()
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
