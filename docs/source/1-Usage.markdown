# Usage

[TOC]

## Initialization
This project is composed by several submodules, (roughly one per benchmark set) that need to be initialized if the corresponding benchmarks have to be used.

To initialize ALL the benchmarks submodules and the submodules needed to build the documentation, it is enough to issue a `make setup` command from the repo root folder.

Each submodule can be initialized independently by following its instructions, accessible from the [Benchmarks](@ref #benchmarks) section.

## Compilation

Benchmarks compilation steps are described in the
[Building with the framework](2-Building_with_the_framework.markdown).
In addition, each benchmark set has specific instruction in the relative module
page to build all the benchmarks of the corresponding set. Generally issuing
a `make` command in the benchmark folder should suffice.
Reading the documentation of the related benchmark set, available from
the module in the [Benchmarks page](@ref #benchmarks) will provide benchmark-specific instructions.

### Top-level Makefile

The project has a top level Makefile that can be used to quickly perform certain operations.

#### Custom flags

To enable certain RT-Bench features it is necessary to supply the compiler with additional flags.

To accomodate so in every makefile it is possible to add these flags in the `CFLAGS` variable before invoking the `make` command.
The example below will include support for JSON configuration files (discussed [here](@ref #benchmarks)) to the default make target:

```{.sh}
CFLAGS=-DJSON_SUPPORT make
```

A list of all the targets that can be specified can can be found [here](#compilation).

#### General targets

General targets perform operations on the whole project.

- `all`: a dummy targets that prevents the user to compile the full project.
- `docs`: Generate this documentation locally. Refer to [Documentation Rules](#docrules) for more precise instructions. Additional target are available in the makefile in `docs`.
- `clean`: Removes all non-source code files for the repo and all git submodules.
- `setup`: Initializes all the git submodules.

#### Setup targets

Setup targets are intended to initialize git submodules and prepare them for usage.
Since submodule might have dependencies in the setup phase, the `DOCS_ONLY` variable can be used to ignore these dependencies when initializing modules when generating documentation.

- `setup-docs`: Initializes the git submodules needed for building the documentation.
- `setup-tacle`: Initializes the git submodule for the [TACLeBench](@ref #rt-tacle-bench) suite.
- `setup-image-filters`: Initializes the git submodule for the [Image Filers](@ref #image-filters) benchmarks.

#### Compilation targets

Compilation targets are meant to compile all the benchmarks in a [benchmark set](@ref #benchmarks) with a single command. Refer to the [compilation instructions](#compilation) for dependencies.

- `compile-isolbench`: Compiles the [IsolBench](@ref #IsolBench) suite.
- `compile-vision`: Compiles the [SD-VBS](@ref #SD-VBS) suite.
- `compile-tacle`: Compiles the [TACLeBench](@ref #rt-tacle-bench) suite.
- `compile-image-filters`: Compiles the [Image Filers](@ref #image-filters) benchmarks.

#### Clean targets

Compilation targets are meant to remove most of the non-source code files.

- `clean-isolbench`: Cleans the [IsolBench](@ref #IsolBench) suite.
- `clean-vision`: Cleans the [SD-VBS](@ref #SD-VBS) suite.
- `clean-tacle`: Cleans the [TACLeBench](@ref #rt-tacle-bench) suite.
- `clean-image-filters`: Cleans the [Image Filers](@ref #image-filters) benchmarks.

#### Benchmark groups
Benchmark suite are also grouped to facilitate certain types of tests.
Each group will have a single setup, compilation and clean target.

Available groups are:
- `group-WCET`: WCET related benchmarks.
  Includes:
  - [TACLeBench](@ref #rt-tacle-bench)
- `group-interf`: Synthetic benchmarks used for system interference.
  Includes:
  - [IsolBench](@ref #IsolBench)
- `group-vision`: Computer vision benchmarks.
  Includes:
  - [SD-VBS](@ref #SD-VBS)

To use these grouped targets it is enough to prepend `setup-`, `clean-` or `compile-` before the group name.

## Benchmark executable

Depending on the benchmark it might be necessary to supply benchmark-specific
options. The [Benchmark page](@ref #benchmarks) will present the common options that RT-Bench exposes,
while for the benchmark-specific options and argument the user should refer to the specific benchmark set module.

### Benchmark execution example

To execute the benchmark is sufficient to give the executable the required arguments via CLI.
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

@author Mattia Nicolella
@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
