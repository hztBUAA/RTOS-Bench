# Building with the framework

[TOC]

This page will guide the user in building benchmarks with RT-Bench.

## Dependencies

In the current implementation the framework has some
dependencies the user has to be aware of:

- Glibc: Provides primitives used by the memory watcher and the argument parser.
- POSIX.4 real-time signals: used to execute the benchmark periodically and to gather stats.
- Linux scheduler syscalls: Used to change the scheduling policy.
- Linux Perf: Used to read performance counters (currently only on CORTEX A53)
- [JSON-C](https://github.com/json-c/json-c): Used to read and parse input JSON configuration files.

Currently RT-Bench targets the following platforms:
- x86/x86_64
- ARM64

#### Dependence installation
  The `json-c` dependence can be installed with the following command:
- Ubuntu/Debian:  
```{.sh}
sudo apt install libjson-c5 libjson-c-dev
```
- Arch Linux:  
```{.sh}
sudo pacman -S json-c
```

## Compiling RT-Bench

Compiling a RT-Bench compliant benchmark (see [benchmark structure](3-Extending_rt-bench.markdown)) with the framework is relatively easy as with GCC only few optional and mandatory flags are required or adviced.

The simplest compilation line necessary is as follows:

```{.sh}
gcc -O2 -Wall -g -Ipath/to/rt-bench/generator -lrt -lm -ljson-c -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap target.c path/to/rt-bench/generator/*.c -o target
```

where:

- `-O2 -Wall -g` are _optional_ but recommended flags
- `-Ipath/to/rt-bench/generator` is the path to the `generator/` folder located within your local rt-bench repository (_mandatory_)
- `-lrt -lm -ljson-c -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap` _must_ appear for the correct working of the RT-Bench core mechanics 
- `path/to/rt-bench/generator/*.c` is the path to all the components located in the `rt-bench_generator/` folder within your local rt-bench repository 
- `target` is the name of the benchmark under consideration

## Optional RT-Bench specific options

In addition, RT-Bench supports dedicated flags that enable access to further features. These features are not part of the default set of features as they depend on the benchmark nature itself or on the platform on which the benchmarks will be deployed.

#### Extended Reporting (Benchmark Specific Measurement Reporting)

Some benchmark classes (e.g., synthetic workloads) measure specific impact on the platform. RT-Bench offers the possibility to extend the existing `.csv` report interface to include the desired _benchmark-specific_ measurement. Providing the benchmarks follows the rules mentioned in [benchmark structure](3-Extending_rt-bench.markdown), extended reporting can be enabled by adding the `-DEXTENDED_REPORT` flag in the compilation command line.

#### Performance counters and monitoring thread

This set of feature being specific to the core and platform on which the benchmark will be deployed, two parameters must be added in other to enable them: the ISA and the core model. The table below lists of the flags to add and provide examples of compliant platform.

Additionally, there are equivalent Make variables that will enable the corresponding parameters when issuing a `make` command.

|    ISA    |     CORE     | Make Variable |   Platform    |
| :-------: | :----------: | :------------:| :-----------: |
| `-DAARCH64` | `-DCORTEX_A53` | `CORE=CORTEX_A53` | Xilinx ZCU102 |
