from os import listdir
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data_folder = "data/"
suites = ["tacle/", "vision/", "image-filters/"]

for suite in suites:
    benchmarks = listdir(data_folder+suite);
    misses_per_clock = []
    inst_per_clock = []
    for bench in benchmarks:
        # read file
        data = pd.read_csv(data_folder+suite+bench)
        # compute metrics
        clock = np.mean(data["job_elapsed(clock_cycles)"].values)
        misses_per_clock += [np.divide(np.mean(data["job_l1_misses"].values), clock)]
        inst_per_clock += [np.divide(np.mean(data["instructions_retired"].values), clock)]
    # plot
    plt.scatter(misses_per_clock, inst_per_clock, label=suite, s=3)

plt.xlabel("LLC misses per clock")
plt.ylabel("Instruction per clock")
plt.legend()
plt.tight_layout()
plt.savefig("mapping.pdf", bbox_inches='tight')
