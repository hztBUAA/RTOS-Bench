Extending RT-Bench
==================

[TOC]

This page will guide the user through the procedure to add a new benchmark or benchmark set.
Another guide that the used should read is the [Documentation guide](4-Documentation_rules.markdown).

## Adding and documenting a new benchmark set {#new-set}

To add a new benchmark set and integrate it with the other sets the following steps are needed:

(As an example, we will describe what to do if a new benchmark set, called `new set` is to be added to this repo)

1. The new benchmark set must be self contained in a single folder in the repo root (example: `new_set`).
2. In `docs/source/modules` a new folder with the name/acronym of the set must be created. This folder will contain the documentation for the modules that compose the benchmark set (example: `docs/source/modules/new_set`).
3. The benchmark set is to be described as a module in a .dox file under the modules folder (`docs/source/modules/new_set/new_set.dox`).
    
   Example:

   ```{.dox}
   /**
    * @defgroup new_set
    * @ingroup benchmarks
    * @brief Brief description of the benchmark set.
    * @details
    * Detailed description of the benchmark set. If there are documents that describe the whole benchmark set they can be referenced here.
    */
   ```
This step will ensure that all be benchmarks are grouped together and a new link will appear in the [Available Benchmarks](@ref #benchmarks) page.
Furthermore, documentation specific only to the benchmark set can be placed inside the detailed description in Markdown syntax. 

Refer to the [SD-VBS](@ref #SD-VBS) module for a working example.

It also possible and encouraged to create subsets if necessary, using the same procedure.

The next section will cover how to add benchmarks in an existing set.

## Adding and documenting a new benchmark in an existing set {#new-bmark}

When adding and integrating new benchmark in an existing benchmark set the following steps are needed:

(As an example we will describe what to do when a new benchmark called `new benchmark` is added to the benchmark set called `new set`)

1. The benchmark files must be contained inside the relative benchmark set folder (example: `new_set`).
It is also recommended to create a folder with the benchmark name that will contain all the benchmark-exclusive files (example: `new_set/new_benchmark`), but there are no defined rules on how the benchmark set folder must be organized, it is sufficient to explain how to maintain, compile and execute the benchmarks module (step 2) or in the set documentation.
2. Create a .dox file with the benchmark name in the benchmark set documentation folder which will describe what the benchmark does (example: `docs/source/new_set/new_benchmark.dox`). For instance:
   ```{.dox}
   /**
    * @defgroup new_benchmark
    * @ingroup new_set
    * @brief benchmark brief description.
    *
    * Benchmark detailed description which includes:
    * - benchmark files location
    * - what the benchmark does (a reference/link to another document is sufficient)
    * - how to compile the benchmark (a reference/link to another document is sufficient)
    * - how to execute the benchmark (a reference/link to another document is sufficient)
    */
   ```
3. Each source file and header must have a documentation header with a reference to the benchmark module, a brief description of the file contents and optionally a detailed description of the file contents (example: `new_set/new_benchmark/benchmark_file.c`,`new_set/new_benchmark/benchmark_header.h`).
    The example for `new_set/new_benchmark/benchmark_file.c`, `new_set/new_benchmark/benchmark_header.h` is the same:
   ```{.c}
   /**
    * @file benchmark_file.c
    * @ingroup new_benchmark
    * @brief Benchmark brief description
    * @details
    * Benchmark detailed description.
    */
    ```
4. The benchmark files must export (and document as described in the next section) three functions:
	-
    ```{.c}
    int benchmark_init(int parameters_num, void **parameters)
    ```
	Will initialize the benchmark using the supplied parameters. This initialization is run only once so it needs to prepare the benchmark for periodic execution (eg. reading data from file, allocation memory, preparing data structures,...) Data written by this function must be treated a read-only, while memory allocated can be freely used, but should be reset after execution.
	-
    ```{.c}
    void benchmark_execution(int parameters_num, void **parameters)
    ```
	Will execute the benchmark as if it was launched for the first time. It must treat data from the `benchmark_init` function as read-only and reset any used memory location to its initial value after the benchmark has completed, to ensure that periodic executions will have always the same environment and hence the same result.
	-
    ```{.c}
    void benchmark_teardown(int parameters_num, void **parameters)
    ```
	Will revert all the operations done by `benchmark_init` and free allocated memory, to ensure a clean termination of the program. This function is executed only when the program is terminating.

`parameters_num` and `parameters` are initialized by the [RT-Bench Generator](@ref #rt-bench_generator) module with the contents of the `-b` options and can be used like `argc` and `argv`.
Global variables can be used to maintain data between different calls of these three functions.

Refer to the [disparity](@ref #disparity) benchmark documentation and source code for a working example.

## Adding and documenting scripts and utilities

The procedure to add new script to the [Utils](@ref #utils) layer is almost the same as for adding benchmark.
It is required to create a .dox file (as for [new benchmark sets](#new-set)) for each script, which will hold the instructions for the script/utility and indicate `utils` as the parent set.
The .dox file location must be in `rt-bench/docs/source/modules/utils/new_script`.

Example:

```{.dox}
/**
* @defgroup new_script
* @ingroup utils
* @brief Brief description of the utility script.
* @details
* Detailed description of script, including possible quirks and instruction on how to use it. If there are documents that describe the script set they can be referenced here.
*/
```

This will allow the script to be included in the [Utils](@ref #utils) page.
 
Script files must be located in the `utils` folder under the project root, it is up to the user to create a subfolder to group all the related script files together.
The files have to be documented as for [benchmark files](#new-bmark).
Since doxygen is able to an extent to pickup documentation for different languages it is recommended to checkout the [doxygen manual](https://doxygen.nl/manual/starting.html#step0) to understand how to write comments for languages different than C.

Refer to `base.py` as an example of a script documented with doxygen comments.
