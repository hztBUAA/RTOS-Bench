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

Currently RT-Bench targets the following platforms:
- x86/x86_64
- ARM64

## Compiling RT-Bench

Compiling a RT-Bench compliant benchmark (see [benchmark structure](3-Extending_rt-bench.markdown)) with the framework is relatively easy as with GCC only few optional and mandatory flags are required or adviced.

The simplest compilation line necessary is as follows:

```{.sh}
gcc -O2 -Wall -g -Ipath/to/rt-bench/rt-bench_generator -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap target.c path/to/rt-bench/rt-bench_generator/*.c -o target
```

where:

- `-O2 -Wall -g` are _optional_ but recommended flags
- `-Ipath/to/rt-bench/rt-bench_generator` is the path to the `rt-bench_generator/` folder located within your local rt-bench repository (_mandatory_)
- `-lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap` _must_ appear for the correct working of the RT-Bench core mechanics 
- `path/to/rt-bench/rt-bench_generator/*.c` is the path to all the components located in the `rt-bench_generator/` folder within your local rt-bench repository 
- `target` is the name of the benchmark under consideration

## Optional RT-Bench specific options

In addition, RT-Bench supports dedicated flags that enable access to further features. These features are not part of the default set of features as they depend on the benchmark nature itself or on the platform on which the benchmarks will be deployed.

#### Performance counters and monitoring thread

This set of feature being specific to the core and platform on which the benchmark will be deployed, two parameters must be added in other to enable them: the ISA and the core model. The table below lists of the flags to add and provide examples of compliant platform.

Additionally, there are equivalent Make variables that will enable the corresponding parameters when issuing a `make` command.

|    ISA    |     CORE     | Make Variable |   Platform    |
| :-------: | :----------: | :------------:| :-----------: |
| `-DAARCH64` | `-DCORTEX_A53` | `CORE=CORTEX_A53` | Xilinx ZCU102 |
