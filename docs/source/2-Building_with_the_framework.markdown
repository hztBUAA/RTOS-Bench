# Building with the framework

[TOC]

Compiling a RT-Bench compilant benchmark (see [benchmark structure](3-Adding_benchmarks.markdown)) with the framework is relatively easy as with GCC only few optional and mandatoryflags are required or adviced.

The simplest compilation line necessary is as follows:

```{.sh}
gcc -O2 -Wall -g -Ipath/to/rt-bench/base -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap target.c path/to/rt-bench/base/*.c -o target
```

where:

- `-O2 -Wall -g` are _optional_ but recommended flags
- `-Ipath/to/rt-bench/base` is the path to the `base/` folder located in to the base folder located within your local rt-bench repository (_mandatory_)
- `-lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap` _must_ appear for the correct working of the RT-Bench core mechanics - `path/to/rt-bench/base/*.c` is the path to all the components located in the `base/` folder within your local rt-bench repository - `target` is the name of the benchmark undre consideration

## Optional RT-Bench specific options

In addition, RT-Bench supports dedicated flags that enable access to further features. These features are not part of the default set of features as they depend on the benchmark nature itself or on the platform on which the benchmarks will be deployed.

#### Performance counters and monitoring thread

This set of feature being specific to the core and plrform on which the benchmark will be deployed, two parameters must be added in other to enable them: the ISA and the core model. The table below lists of the flags to add and provide exmaples of compliant platform.

Additionally, there are equivalent Make variables that will enable the corresponding parameters when issuing a `make` command.

|    ISA    |     CORE     | Make Variable |   Platform    |
| :-------: | :----------: | :------------:| :-----------: |
| `-DAARCH64` | `-DCORTEX_A53` | `CORE=CORTEX_A53` | Xilinx ZCU102 |
