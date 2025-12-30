import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys

if (__name__ == "__main__"):
    # read file
    data = pd.read_csv(sys.argv[1])
    data = data[data["samples"] == 50]
    # extract column
    l2_references = np.divide(data["l2_references"], data["samples"])
    l2_references = np.divide(l2_references, l2_references.iloc[-1]).values
    l2_refills = np.divide(data["l2_refills"], data["samples"])
    l2_refills = np.divide(l2_refills, l2_refills.iloc[-1]).values
    l1_references = np.divide(data["l1_references"], data["samples"])
    l1_references = np.divide(l1_references, l1_references.iloc[-1]).values
    l1_refills = np.divide(data["l1_refills"], data["samples"])
    l1_refills = np.divide(l1_refills, l1_refills.iloc[-1]).values
    inst_retired = np.divide(data["inst_retired"], data["samples"])
    inst_retired = np.divide(inst_retired, inst_retired.iloc[-1]).values
    # plot
    x = np.arange(len(inst_retired))
    plt.plot(x, l1_references, label="L1 References")
    plt.plot(x, l1_refills, label="L1 Refills")
    plt.plot(x, l2_references, label="LLC References")
    plt.plot(x, l2_refills, label="LLC Refills")
    plt.xlabel("Time (10ms)")
    plt.plot(x, inst_retired, label="Inst. Retired")
    plt.ylabel("Normalized cumulative distribution")
    plt.legend()
    plt.savefig(sys.argv[2]+".pdf", bbox_inches='tight')
