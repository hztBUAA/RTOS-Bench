from os import listdir
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

data_folder = "../../results/x86_64/intel/i7/8550U/"
suites = [folder+"/" for folder in listdir(data_folder)]

for suite in suites:
    benchmarks = listdir(data_folder+suite);
    misses_per_clock = []
    inst_per_clock = []
    for bench in benchmarks:
        # read file
        data = pd.read_csv(data_folder+suite+bench)
        # compute metrics
        clock = np.mean(data["job_elapsed(clock_cycles)"].values)
        misses_per_clock += [np.divide(np.mean(data["job_l1_references"].values), clock)]
        inst_per_clock += [np.divide(np.mean(data["instructions_retired"].values), clock)]
    # plot
    plt.scatter(misses_per_clock, inst_per_clock, label=suite)
    for i, text in enumerate(benchmarks):
        plt.annotate(text.replace(".csv", ""), (misses_per_clock[i], inst_per_clock[i]), fontsize=7)

plt.xlabel("L1 references per clock")
plt.ylabel("Instruction per clock")
plt.legend()
plt.tight_layout()
plt.savefig("ipc_l1ref.pdf", bbox_inches='tight')
