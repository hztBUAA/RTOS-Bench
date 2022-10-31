Extending RT-Bench
==================

[TOC]

This page will guide the user through the procedure to add a new benchmark or benchmark set.
Another guide that the used should read is the [Documentation guide](4-Documentation_rules.markdown).

## Add a new benchmark set {#new-set}

To add a new benchmark set and integrate it with the other sets the following steps are needed:

(As an example, we will describe what to do if a new benchmark set, called `new set` is to be added)

1. The new benchmark set must be self contained in a repository. (example: `new_set`).
   The benchmark set repository has to be included in RT-Bench as a git submodule from the repo root.
2. The makefile in the repo root has to be updated with new targets that must be documented in the usage page:
   - `setup-new_set`: This target has to initialize the git submodule upon invocation.
     `setup-new_set` must also be included in as a dependency of the `setup` target.
     **NOTE**:The `setup-new_set` target will be used to generate the documentation, so is in this target there are dependecies to specific scripts or executable that do not impact on documentation generation they should be enclosed in a `ifndef DOCS_ONLY` [dependency-specific code] `endif`. See the `setup-image-filters` target for a working example. 
   - `compile-new_set`: This target has to compile all the benchmarks in the set.
   - `clean-new_set`: This target has to clean all the compilation and execution byproducts, including data, object files and executables generated.
     `clean-new_set` has also to be included as dependency of the `clean` target.
     
   The newly created target have to be added to the grouped targets, creating a new group if necessary
3. If the benchmark set uses a Makefile, the Makefile in `generator/Makefile` has to be included as early as possible, since it will define the necessary compilation and linking flags according to the features enabled.

    The `generator/Makefile` will define the `CFLAGS` and `LDFLAGS` variables that have to be used in the compilation command line. 
    Both `CFLAGS` and `LDFLAGS` can be extended, by using the `override` and `+=` operator.

    See `IsolBench/Makefile` for an example.

4. The benchmark set is to be described as a module in a .dox or .md file.

    All the details of the benchmark set, including submodule setup, compilation, benchmarks general usage have to be described in this file.
    It is also possible to indicate TODOs and bugs by using the doxygen `@todo` and `@bug` commands.
    
   To make sure that Doxygen will place the instruction in the correct location the file has to begin with the following snippet:
   
   ```
   @defgroup new_set New benchmark set name.
   @ingroup benchmarks
   @brief Brief description of the benchmark set.
   @details
   Detailed description of the benchmark set. If there are documents that describe the whole benchmark set they can be referenced here.
   @author Author
   @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
   SPDX-License-Identifier: [SPDX license expression]
   ```
**NOTE**: Doxygen will raise an error if `@defgroup` is used more than once with the same parameters!
   
   In case a .dox file is used, all the documentation has to be included in a Doxygen C-style comment:
   ```
   /**
    * [Snippet contents and documentation here]
    */
   ```
   
   In case a .md file is used the `utils/md2dox.sh` script should be used to safely convert the markdown file to a .dox file, specifying the filename as first argument.
   It is possible to enclose doxygen commands in HTML comments (i.e. `<!-- @bug -->`) for a cleaner rendering of the markdown file. The script will take care to uncomment them during the conversion.
   It is advised to add the resulting .dox file to the repository gitignore.
   This is necessary since currently in doxygen there is no way to avoid having an page for every markdown file. In addition, when defining a group the content of the group description will be removed from the current page. The combination of markdown files that specify groups would then litter the documentation with empty pages.

This step will ensure that all be benchmarks are grouped together and a new link will appear in the [Available Benchmarks](@ref #benchmarks) page.
Furthermore, documentation specific only to the benchmark set can be placed inside the detailed description in Markdown syntax. 


It also possible and encouraged to create subsets if necessary, documented using the same procedure.

Folders can be excluded from the documentation by editing the `EXCLUDE` tag in the RT-Bench Doxyfile (`docs/conf/Doxyfile`)

### Examples

The [SD-VBS](@ref #SD-VBS) module source file (`docs/source/modules/SD-VBS/SD-VBS.dox`) is a working .dox example.
The [TACLeBench](@ref #rt-tacle-bench) module source file (`rt-tacle-bench/README.md`) is a working markdown example that is converted to .dox for documentation purposes, while doubling as the submodule repository README.

Refer to the `setup-tacle`, `compile-tacle`, `clean-tacle` targets in the top-level Makefile as examples on how to initialize, build and clean the submodule.

**NOTE**: For markdown files, the extension has to be `.md`, so that they will not be included in doxygen documentation as pages, but can be convert with the aforementioned script.
This example will exclude all file in the RT-Bench documentation config folder and in the TACLeBench `parallel` subfolder from being documented:

```
EXCLUDE = ../config \
          ../../rt-tacle-bench/bench/parallel
```

**NOTE**: When editing the Doxyfile, paths are relative to the Doxyfile location!
### Updating a benchmark set submodule

Changes in an RT-Bench submodule are not detected automatically to prevent the submodule breaking when breaking changes are introduced and to make te commit history of RT-Bench always deployable.
Whenever a submodule is updated and it is judeged compatible to the current version of RT-Bench it is necessary to update the submodule reference SHA-1.
A quick way to do this is by adding the submodule folder to the repo, committing and pushing the changes.    

## Add a new benchmark in an existing set {#new-bmark}

When adding and integrating new benchmark in an existing benchmark set the following steps are needed:

(As an example we will describe what to do when a new benchmark called `new benchmark` is added to the benchmark set called `new set`)

1. It is recommended to create a folder with the benchmark name that will contain all the benchmark-exclusive files (example: `new_set/new_benchmark`), but there are no defined rules on how the benchmark set folder must be organized, it is sufficient to explain how to maintain, compile and execute the benchmarks in the set documentation.
2. It is recommended to create a set that will group all the files that compose the benchmark.
   The set is defined as follows and this snippet can be located in standalone .dox file or in any of the benchmark files:
   ```{.c}
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
    * @author Author
    * @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
    * SPDX-License-Identifier: [SPDX license expression]
    */
   ```
**NOTE**: Doxygen will raise an error if `@defgroup` is used more than once with the same parameters!
3. Each source file and header must have a documentation header with a reference to the benchmark module, a brief description of the file contents and optionally a detailed description of the file contents (example: `new_set/new_benchmark/benchmark_file.c`,`new_set/new_benchmark/benchmark_header.h`).
    The example for `new_set/new_benchmark/benchmark_file.c`, `new_set/new_benchmark/benchmark_header.h` is the same:
   ```{.c}
   /**
    * @file benchmark_file.c
    * @ingroup new_benchmark
    * @brief Benchmark brief description
    * @details
    * Benchmark detailed description.
    * @author Author
    * @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
    * SPDX-License-Identifier: [SPDX license expression]
    */
    ```
    It is recommended to document TODOs and bugs can be documents anywhere inside a doxygen comment using the `@todo` and `@bug` commands.
3. Files have to be documented according to the [Documentation Rules](#docrules). An exception can be made for symbols and function that were not written as a result of the adaptation.
4. The benchmark files _must_ export three functions:
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

  `parameters_num` and `parameters` are initialized by the [RT-Bench Generator](@ref #generator) module with the contents of the `-b` options and can be used like `argc` and `argv`.
  Global variables can be used to maintain data between different calls of these three functions.

5. The benchmark files must import the following libraries (provided by the [RT-Bench Generator](@ref #generator)):
  - The logging library provieded by RT-Bench.
  ```{.c}
  #include "logging.h"
  ```
  - The header that defines the exported functions along with other macros and dependencies.
  ```{.c}
    #include "periodic_benchmark.h"
  ```

6. _Optionally_, the benchmark can export functions for the extending the report interface. For this, only two functions are necessary:
	-
    ```{.c}
    const char* benchmark_log_header()
    ```
  	Which returns a constant string to extend the csv header (e.g., ",bandwidth(MB/S)" for isolbench/bandwidth)
	-
    ```{.c}
    float benchmark_log_data()
    ```
	  Which returns the benchmark-specific measurement.

  Note that, as indicated in [the building guidelines](2-Building_with_the_framework.markdown), the `-DEXTENDED_REPORT` compilation flag _must_ be used for these functions to be called.

Refer to the [disparity](@ref #disparity) benchmark documentation and source code for a working example.


## Add scripts and utilities

It is required to add the utility in the utils set, by creating a subset that will describe the utility as in the example below:

```{.dox}
/**
* @defgroup new_script New script
* @ingroup utils
* @brief Brief description of the utility script.
* @details
* Detailed description of script, including possible quirks and instruction on how to use it. If there are documents that describe the script set they can be referenced here.
* @author Author
* @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
* SPDX-License-Identifier: [SPDX license expression]
*/
```
This will allow the script to be included in the [Utils](@ref #utils) page.
The snippet can be located in a standalone .dox file, in `rt-bench/docs/source/modules/utils/new_script`. Or directly in one of the utility files.

**NOTE**: Doxygen will raise an error if `@defgroup` is used more than once with the same parameters!
 
Script files must be located in the `utils` folder under the project root, it is up to the user to create a subfolder to group all the related script files together.
Files have to be documented according to the [Documentation Rules](#docrules).
Since doxygen is able to an extent to pickup documentation for different languages it is recommended to checkout the [doxygen manual](https://doxygen.nl/manual/starting.html#step0) to understand how to write comments for languages different than C.

Refer to `WSS.py` as an example of a script documented with doxygen comments.

### Add a new module in the main repo

To add a new module in RT-Bench the following steps are needed:

(As an example, we will describe what to do if a new module, called `new module` is to be added to this repo)

1. The new module must be self contained in a single folder (example: `new_module`).
2. In `docs/source/modules` a new folder with the name/acronym of the module must be created. This folder will contain the documentation for the modules that compose the benchmark set (example: `docs/source/modules/new_module`).
3. The benchmark set is to be described as a module in a .dox file under the modules folder (`docs/source/modules/new_set/new_module.dox`).
   Example:
   ```{.dox}
   /**
    * @defgroup new_module New Module
    * @ingroup parent_module_if_present
    * @brief Brief description of the module.
    * @details
    * Detailed description of the module. If there are documents that describe the whole benchmark set they can be referenced here.
    * @author Author
    * @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
    * SPDX-License-Identifier: [SPDX license expression]
    */
   ```

Refer to the [SD-VBS](@ref #SD-VBS) module for a working example that has submodules.

It also possible and encouraged to create subsets if necessary, using the same procedure.

Files that compose the module are to be documented as follows:
1. The module files must be contained inside the relative benchmark set folder (example: `new_module`).
There are no defined rules on how the module folder must be organized, it is sufficient to explain how to maintain, compile and execute the module.

2. In case of a submodule, create a .dox file with the submodule name in the module documentation folder which will describe what the submodule does (example: `docs/source/new_module/new_submodule.dox`). For instance:

   ```{.dox}
   /**
    * @defgroup new_submodule
    * @ingroup new_module
    * @brief submodule brief description.
    * @details
    * Submodule detailed description
    * @author Author
    * @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
    * SPDX-License-Identifier: [SPDX license expression]
    */
   ```

3. Each source file and header must have a documentation header with a reference to the benchmark module, a brief description of the file contents and optionally a detailed description of the file contents (example: `new_set/file.c`,`new_set/header.h`).
    The example for `new_set/file.c`, `new_set/header.h` is the same:
   ```{.c}
   /**
    * @file file.c
    * @ingroup new_module
    * @brief Brief description
    * @details
    * Detailed description.
    * @author Author
    * @copyright [year file created] - [last year file modified], [file author] <[author email]> and the [project name] contributors
    * SPDX-License-Identifier: [SPDX license expression]
    */
    ```

Files have to be documented according to the [Documentation Rules](#docrules).

Setup, compilation and cleaning scripts for the new module should be included in the top-level makefile if needed.
Any new target should be documented in the usage page.

@author Mattia Nicolella
@copyright (C) 2021 - 2022, Denis Hoornaert <denis.hoornaert@tum.de>, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
