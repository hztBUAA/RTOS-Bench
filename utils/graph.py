#! /bin/python
"""!
@file graph.py
@brief Draw graphs with matplotlib from csv files containing data
@details
This script contains a collection of methods to draw graphs starting from data in csv files.
@author Mattia Nicolella
"""
import matplotlib.pyplot as plt
from cycler import cycler

## Linestyles for plot, refer to: https://matplotlib.org/stable/gallery/lines_bars_and_markers/linestyles.html
linestyle_tuple = [
    (0,()), #solid
    (0, (1, 1)), #dotted
    (0, (5, 5)), #dashed
    (0, (3, 5, 1, 5)), #dashdotted
    (0, (3, 5, 1, 5, 1, 5)), #dashdotdotted
    (0, (1, 10)), #loosely dotted
    (0, (5, 10)), #loosely dashed
    (0, (3, 10, 1, 10)), #loosely dashdotted
    (0, (3, 10, 1, 10, 1, 10)), #loosely dashdotdotted
    (0, (1, 1)), # densely dotted
    (0, (5, 1)), # densely dashed
    (0, (3, 1, 1, 1)), #densely dashdotted
    (0, (3, 1, 1, 1, 1, 1)) #densely dashdotdotted
]

## All available markers, refer to: https://matplotlib.org/stable/api/markers_api.html#module-matplotlib.markers
markers=[".","o","v","^","<",">","1","2","3","4","8","s","p","P","*","h","H","+","x","X","D","d","|","_"]

def plot(x,y,xlabel,ylabel,title,line_labels=None,fname):
    lines=len(x)
    #we setup a cycler, which will change line style, markers and color automatically for each line
    cc=cycler(color=plt.cm.tab20(np.linspace(0,1,lines)))
    if lines <= len(linestyle_tuple):
        cc+=cycler(linestyle=linestyle_tuple[:lines])
    if lines<= len(markers):
        cc+=cycler(marker=markers[:lines])
    #get the axes
    ax = plt.gca()
    # we set the cycler
    ax.set_prop_cycle(cc)
    # we create the line collection
    for i in range(0,len(x)):
        try:
            label=line_labels[i]
        except Exception:
            label=None
        plt.plot(x[i],y[i],label=label)
    # we set plot properties
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.autoscale()
    plt.legend()
    ax.grid(True)
    plt.savefig(fname,format="png")
    plt.savefig(fname,format="svg")

plot([[1,2,3],[4,5,6],[7,8,9]],[[10,11,12],[13,14,15],[16,17,18]],'xlabel','ylabel','title',['1','2','3'])

## @TODO: make an execute function that will be called by other modules to draw graphs

if __name__ == "__main__":
    ## @TODO: parse command line arguments (for graph name, type, properties and csv location), then parse csv, draw graph
