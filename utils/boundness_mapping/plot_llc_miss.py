from os import listdir
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

labels_offset = {
    "sha.csv": (0.0, 0.005),
    "mpeg2.csv": (0.0, -0.005),
    "cjpeg_transupp.csv": (0.0, 0.02),
    "filterbank.csv": (0.0, -0.02),
    "gsm_dec.csv": (0.0, 0.005),
    "isqrt.csv": (0.0, 0.01),
    "gsm_enc.csv": (0.0, -0.005)
}

def placement(label, x, y, offsets=(0.00005, -0.01)):
    a, b = labels_offset[label] if (label in labels_offset.keys()) else (0, 0)
    return (x+offsets[0]+a, y+offsets[1]+b)

data_folder = "../../../measurements-database/aarch64/cortex_a53/rpi3b+/"
suites = ["tacle/", "vision/vga/", "image-filters/vga/"]

fig = plt.figure()
ax = fig.add_subplot(100, 100, (1, 10000))
ax_small = fig.add_subplot(100, 100, (35, 7500))

for suite in suites:
    benchmarks = [bench for bench in listdir(data_folder+suite) if (bench.endswith(".csv"))]
    misses_per_clock = []
    inst_per_clock = []
    for bench in benchmarks:
        # read file
        data = pd.read_csv(data_folder+suite+bench)
        # compute metrics
        clock = np.mean(data["cpu_clock_count"].values)
        misses_per_clock += [np.divide(np.mean(data["job_l2_misses"].values), clock)]
        inst_per_clock += [np.divide(np.mean(data["instructions_retired"].values), clock)]
    # plot
    ax.scatter(misses_per_clock, inst_per_clock, label=suite, s=7, zorder=8)
    # zoom-in
    ax_small.scatter(misses_per_clock, inst_per_clock, s=7, zorder=9)
    for i, text in enumerate(benchmarks):
        if ((0 <= misses_per_clock[i] < 0.0007) and (0.57 <= inst_per_clock[i] < 0.92)):
            ax_small.annotate(text.replace(".csv", ""), placement(text, misses_per_clock[i], inst_per_clock[i], (0.00001, -0.003)), fontsize=7)
        else:
            ax.annotate(text.replace(".csv", ""), placement(text, misses_per_clock[i], inst_per_clock[i]), fontsize=7)

ax.fill_between([-0.00002, 0.0007], [0.58, 0.58], [0.91, 0.91], color='lightgray')
ax_small.set_xlim([0, 0.0007])
ax_small.set_ylim([0.58, 0.92])
ax_small.ticklabel_format(axis='x', style='sci', scilimits=(0, 0))
ax_small.set_xticks([])
ax_small.set_yticks([])

ax.set_xlim([-0.0002, 0.0057])
ax.set_ylim([0.35, 1.8])

# plot zoom lines
ax.plot([-0.00002, 0.0018], [0.91, 1.8], color='lightgrey', linestyle="--", linewidth=1)
ax.plot([0.0007, 0.0057], [0.58, 0.71], color='lightgrey', linestyle="--", linewidth=1)

ax.set_xlabel("LLC misses per clock")
ax.set_ylabel("Instruction per clock")
ax.legend(loc='lower left', ncol=2)
plt.tight_layout()
plt.savefig("mapping.pdf", bbox_inches='tight')
