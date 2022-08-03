RT-Bench
========

[TOC]

RT-Bench is a collection of popular benchmarks for real-time applications which
have been restructured to be executed periodically.

RT-Bench is licensed under [MIT](LICENSES/MIT.txt) license and
integrates benchmark suites that are licensed according to the information
contained in the corresponding folders.

RT-Bench is developed by researchers and collaborators affiliated with the
Cyber-Physical Systems Lab at [Boston University](https://cs-people.bu.edu/rmancuso/)
with contributions from the Chair of Cyber-Physical System in Production Engineering at [TUM](https://rtsl.cps.mw.tum.de/).

## How to use the documentation

Navigating the documentation of the project can be counterintuitive at first, so
this section will guide the user towards making the most of the available documentation.

All the documentation is accessible from the sidebar, and includes:
- Tutorials and explanation pages which will cover the framework in general.
- A todo list
- A buglist
- A Modules page, from which the documentation specific to the 
  [RT-Bench Generator](@ref #generator), the [utilities](@ref #utils) and [benchmarks](@ref #benchmarks) can be accessed.
- Files and data structures documentation (reachable also from the Modules page).

### Quick links
- [Available Benchmarks](@ref #benchmarks)
- [Usage guide](1-Usage.markdown)
- [Compilation guide](2-Building_with_the_framework.markdown)
- [Guide on how to add benchmarks](3-Extending_rt-bench.markdown)
- [Utilities](@ref #utils)

## Features

- Periodic execution of the benchmark with POSIX.4 real-time signals.
- Statistic gathering for each of the jobs executed, including:
  - Period start / end timestamps.
  - Deadline timestamp.
  - Job end timestamp (job starts at the beginning of the period).
  - Period Utilization.
  - Period Density.
  - Perf counters value, (only for CORTEX_A53).
- Several log levels, to log statistics in csv files or in a terminal with different levels of detail.
- Pinning of the process on a single core or set of cores.
- Scheduling policy change.
- Constraining dynamic memory allocations during the execution phase.
- Periodic monitoring of the L2 refills Perf counter (only for CORTEX_A53).
- Automated scripts to perform the following tests:
  - Framework overhead.
  - Empirical minimum working set size.
  - Empirical worst case execution test.
  - Empirical schedulability test.
- Plotting functions for the most common type of graphs, integrated in the test scripts and usable from csv inputs.

## Design and Principles

This section will explain the reasoning behind RT-Bench and present at a high level
of abstraction how the framework works.

The framework lives fully in userspace and is composed by the [RT-Bench Generator](@ref #generator) and by a collection of scripts that  compose the [Utils](@ref #utils) optional layer.

@image html rt-bench-structure.svg "RT-Bench control flow graph"
@image latex rt-bench-structure.pdf "RT-Bench control flow graph" width=10cm

### Motivation and Principles

Many popular benchmark suites do not exhibit real-time features and have to be
restructured to integrate these features.
RT-Bench is a framework that implements real-time features in a generic fashion, to allow different benchmarks (described in [Available Benchmarks](@ref #benchmarks))
to have the features out-of-the-box and accessible via CLI.

To implement the mentioned features, RT-Bench follow some core principles:

- Real-time system abstraction
  Target benchmark is executed periodically and stats are collected for each period.
- Common interface
  All the benchmark report the same basic statistics and have the same CLI interface.
- Extensibility
  Adding benchmark is easy, more details on how to do this are in [Extending RT-Bench](3-Extending_rt-bench.markdown).
- Compatibility
  RT-Bench is designed to be compatible with multiple platforms.
  Moreover, compatible benchmark do have their execution logic intact,
  so it's possible to compare their output with the output of their original version.

### Benchmark Design

To adhere to the above-mentioned principles, the benchmarks are required to implement their logic in the following functions:

- `benchmark_init`: Initialization of the benchmark environment, executed only once.
- `benchmark_execution`: Execution of the benchmark routines, executed periodically. 
- `benchmark_teardown`: Cleanup of the benchmark environment, executed before exiting.

It is thus sufficient to split the benchmark `main` function into these have a compatible benchmark.
The effort to convert benchmark in this way depends on the benchmark logic, however for the whole
[San Diego Vision Benchmarks](@ref #SD-VBS) suite the conversion process, took ~300 SLOCs per benchmark.

**Note**: RT-Bench makes no assumption on what is executed by these functions, 
so individual benchmarks may have additional dependencies or behave in a non-standard way.
These details will be documented in each the benchmark module page.

Once the benchmark is converted, compiled and linked against the RT-Bench generator
the RT-Benchmark generator (specifically `periodic_benchmark.c`) will handle the
execution in the following way:

1. The RT-Bench environment is initialized, (including timers for the deadline and the period).
  1. If reading Perf counters is supported in the current platform and the user 
  enables the counter monitoring, a thread can be created.
  2. The monitoring thread will continuously check if a `benchmark_execution` is running.
2. `benchmark_execution` is executed to prepare the benchmark environment.
3. The period timer is started.
4. When a new period starts `benchmark_execution` is executed.
  1. If supported and enabled, the monitoring thread will periodically sample the Perf counters.
5. Right after the function returns a timestamp is captured.
  1. The monitoring thread also registers that the benchmark is not running anymore, to separate sampling of different jobs.
6. If necessary, the process waits for the end of the period.
7. At the end of the period, the timer handler will take care of collecting and reporting stats
8. Steps 4 to 7 are repeated until the required number of jobs has been executed or a `SIGINT` is received.
9. The `benchmark_teardown` function will clear the benchmark environment and allow for a clean exit.

@image html rt-bench-control-flow.svg "RT-Bench control flow graph"
@image latex rt-bench-control-flow.pdf "RT-Bench control flow graph" width=10cm


The associated paper, available on [ACM Digital Library](https://dl.acm.org/doi/10.1145/3534879.3534888) and [arXiv](https://arxiv.org/abs/2203.11423),
contains more details on the design and some examples of what can be done with the framework.

@author Mattia Nicolella
@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
