# Usage

[TOC]

## Compilation

Benchmarks compilation steps are described in the
[Building with the framework](2-Building_with_the_framework.markdown).
In addition, each benchmark set has specific instruction in the relative module
page to build all the benchmarks of the corresponding set. Generally issuing
a `make` command in the benchmark folder should suffice.
Reading the documentation of the related benchmark set, available from
the module in the [Benchmarks page](@ref #benchmarks) will provide benchmark-specific instructions.

## Benchmark executable

Depending on the benchmark it might be necessary to supply benchmark-specific
options. The [Benchmark page](@ref #benchmarks) will present the common options that RT-Bench exposes,
while for the benchmark-specific options and argument the user should refer to
the specific benchmark set module. 
### Benchmark execution

To execute the benchmark is is sufficient to give the executable the required arguments via CLI.
The only parameters required to run a benchmark are period and deadline.

As an example the following commands will be used to run the [disparity](@ref #disparity) benchmark from the [San Diego Vision Benchamrks](@ref #SD-VBS) suite:

- `# disparity -p 1 -d 0.5 -t 2 -c 0 -f 99 -m 1M -l 3 -b .`: Run the benchmark 
  with 1 second period, 0.5 seconds deadline, execute only two jobs, pin the
  process on core 0, use the FIFO scheduler with priority 99, constrain the 
  dynamic memory allocation to 1MB during execution with log level 3 (csv output
  on terminal) and take the input images from the current folder.
- `# disparity -p 1 -d 0.5 -t 2 -c 0 -f 99 -m 10M -l 3 -b .`: Run the benchmark 
  with 1 second period, 0.5 seconds deadline, execute only two jobs, pin the
  process on core 0, use the FIFO scheduler with priority 99, constrain the 
  dynamic memory allocation to 10MB during execution with log level 3 (csv output
  on terminal) and take the input images from the current folder.
- `# disparity -p 1 -d 0.1 -t 2 -c 0 -f 99 -m 10M -l 3 -b .`: Run the benchmark 
  with 1 second period, 0.1 seconds deadline, execute only two jobs, pin the
  process on core 0, use the FIFO scheduler with priority 99, constrain the 
  dynamic memory allocation to 1MB during execution with log level 3 (csv output
  on terminal) and take the input images from the current folder.
- `# disparity -p 1 -d 0.3 -t 2 -c 0 -f 99 -m 10M -o output.csv -l 2 -b .`: Run the benchmark 
  with 1 second period, 0.3 seconds deadline, execute only two jobs, pin the
  process on core 0, use the FIFO scheduler with priority 99, constrain the 
  dynamic memory allocation to 10MB during execution, with log level 2 (csv output) 
  on the file called `output.csv` and take the input images from the current folder.
- `# disparity -p 1 -d 0.3 -c 0 -f 99 -m 10M -o output.csv -b .`: Run the benchmark 
  with 1 second period, 0.3 seconds deadline, keep executin jobs untile a `SIGINT` is received, pin the
  process on core 0, use the FIFO scheduler with priority 99, constrain the 
  dynamic memory allocation to 10MB during execution, with log level 3 (csv output on terminal) 
  on the file called `output.csv` (ignored in this case) and take the input images from the current folder.

@image html demo-bmark.gif "Animated demo executing the SD-VBS disparity benchmark"  width=50%

## Utility scripts

Most of the utility scripts are designed to be modular and feature some common CLI options.
All the script require the user to have already compiled the benchmarks that will be used in the test.

The list of available scripts and their specific documentation is available inside the [Utils](@ref #utils) module.
