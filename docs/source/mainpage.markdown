rt-bench
========

## Introduction

[TOC]

rt-bench is a collection of popular benchmarks for real-time applications which have been restructured to be executed periodically.

The available benchmarks sets, documented in the Modules section, are:

- [San Diego Vision Benchmarks](@ref #SD-VBS)

All available benchmarks share a set of files, described in the [Base](@ref #base) module, that provide some basic but essential facilities,
such as logging functions and the logic to make execution periodic.

## Usage
Each set of benchmarks has specific compilation and usage instructions in the module description, refer to these instruction and to the benchamrk specification for a correct usage.

In addition all the benchmarks take the same input arguments and options (which are handled by the [Base](@ref #base) module).
These arguments and options are described below and in the benchmark help message:

### Period and deadline options:
- `-d`, `--deadline=secs`: The benchmark deadline in seconds. Can be an integer, float or in scientific notation. Must be less or equal than the benchmark period. __Required__.
- `-p`, `--period=secs`: The benchmark period, in seconds. Can be an integer, float or in scientific notation. __Required__.

### Execution options:
- `-c`, `--core-affinity=core1,core2,...`: The benchmark core affinity, expressed as a comma separated list. A single core id is also accepted. If not provided the OS will decide on which core(s) the benchmark can run.
- `-m`, `--mem-limit=bytes[GMK]`: The maximum amount of dynamic memory allocated during the periodic execution. If exceeded, the benchmark will crash. Specified as an integer plus an optional magnitude modifier:
	- `K`=kilobytes
	- `M`=megabytes
	- `G`=gigabytes

	Without a magnitude modifier specified the value is assumed to be in bytes. 0 means no memory limit, and it is the default setting.
- ``-t``, ``--tasks-number=integer>=0``   The number of tasks to be executed. 0 means until the program receives a SIGINT. Default is 0.

### Reporting options:
- `-l`, `--log-level=log-lvl`: Log level, can be one of the following:
	- `1`: Print only errors.
	- `2`: Print benchmark stats to output file in csv format.
	- `3`: Print benchmark stats to ``stdout`` in csv format.
	- `4`: Print informative messages on ``stdout`` and debug messages on ``stderr``.

	Default is 3.

	See `print_benchmark_timing()` for an explanation on the format used in log levels 2 and 3.

- `-o`, `--output=output_path`: Where the info on the benchmark execution will be written. If not supplied, `./timing.csv` will be used.

### Benchmark arguments and options:
- `-b`, `--bmark-args=arg opt ...`: A space-separated list of arguments and options that will be relayed as it is to the benchmark. It must be specified as the last option, since everything after it will be given directly to the benchmark routine.

### Informational options:

- `-h`, `-?`, `--help`: Give this help list
- `--usage`: Give a short usage message

### Enabling performance counters
Counters have been implemented only for Cortex A53 processors.
Add `CORE=CORTEX_A53` to the make command to enable performance counters.
