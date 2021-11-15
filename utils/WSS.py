"""!
@file WSS.py
@ingroup utils
@brief Test to compute the minimum working set size
@details

This script will execute a minimum working size test by doing the following:
- Use `base.test_init()` to initialize the enviroment.
- Exceute the minimum WSS test
- Cleanup the enviroment by calling `base.test_teardown()`

As a test result some files will be created, these are described `wss_test()`

Dependecies:
- base.py

@author Mattia Nicolella
"""

import csv
import subprocess
import os
import base


def execute(params):
    """!
    @brief Execute the working set size test on the given benchmarks.
    @param[in,out] params Parameters dictionary provided by `base.test_init()`.
    @details Will call `wss_test()` for each element of `params.args.benchmarks`.
    @returns The updated `params` dictionary with {"res":0} on success, or {"res":-1} on failure.
    """
    args = params.get("args")
    if args is None:
        print("ERROR: Missing argument dictionary to execute the wss test!")
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
    for i in range(0, len(args.benchmarks)):
        res = wss_test(
            args.benchmarks[i][0],
            args.benchmarks[i][1],
            args.tasks_num,
            args.output[i],
            args.prefix,
            args.postfix,
            cores[0] - 1,
            sched_params,
        )
        if res < 0:
            break
        else:
            res = 0
    params.update({"res": res})
    return params


def wss_test(bmark, bmark_args, tests, output, prefix, postfix, core, sched_params):
    """!
    @brief Perform a minimum working set size test.
    @param[in] bmark Benchmark on which the test should be executed.
    @param[in] bmark_args Benchmark arguments.
    @param[in] test Number of tests to execute for each working set size.
    @param[in] output The output folder.
    @param[in] prefix The generated file prefix.
    @param[in] postfix The generated file postfix.
    @param[in] core Physical core on which the test will be executed.
    @param[in] sched_params Scheduling attributes.
    @details The function will run a number of tests, specified in `params` constraining the benchmark available memory to 1MB.
    If all the test succeed the available memory limit will be halved. Otherwise the memory limit will be doubled.
    The execution will stop when the smallest amount of memory to run a benchmark is determined.

    Results will be saved in a file called: `working_set_size_test.csv`

    @returns the minimum wss, failures in the benchmark execution treated as a wrong wss size.
    """
    current_wss = 2 ** 20
    last_wss = 0
    failed_tests = 0
    bmark_name = os.path.basename(bmark)
    filename = os.path.join(
        output, prefix + bmark_name + "_min_wss_test" + postfix + ".csv"
    )
    while current_wss != last_wss or failed_tests != 0:
        print(f"\n\n{bmark_name} current wss:{current_wss}, last wss:{last_wss}")
        failed_tests = 0
        try:
            subprocess.run(
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
                + ["-b"]
                + bmark_args,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            failed_tests += 1
            last_wss = current_wss
            current_wss = last_wss * 2
        else:
            if current_wss // 2 != last_wss and current_wss // 2 > 0:
                last_wss = current_wss
                current_wss = last_wss // 2
            else:
                last_wss = current_wss
    with open(filename, "w") as file:
        writer = csv.writer(file)
        writer.writerow(["benchmark", "minimum wss (bytes)"])
        writer.writerow([bmark_name, current_wss])
    return current_wss


if __name__ == "__main__":
    parser = base.parser_init()
    params = base.test_init(parser)
    execute(params)
    base.test_teardown(params)
