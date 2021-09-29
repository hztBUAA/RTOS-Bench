#! /bin/python
"""!
@file graph.py
@brief Draw graphs with matplotlib from csv files containing data
@details
This script contains a collection of methods to draw graphs starting from data in csv files.
@author Mattia Nicolella
"""
import numpy as np
import os
from matplotlib.figure import Figure
from cycler import cycler

## Linestyles for plot, refer to: https://matplotlib.org/stable/gallery/lines_bars_and_markers/linestyles.html
linestyle_tuple = [
    (0,()), ##< solid
    (0, (1, 1)), ##< dotted
    (0, (5, 5)), ##< dashed
    (0, (3, 5, 1, 5)), ##< dashdotted
    (0, (3, 5, 1, 5, 1, 5)), ##< dashdotdotted
    (0, (1, 10)), ##< loosely dotted
    (0, (5, 10)), ##< loosely dashed
    (0, (3, 10, 1, 10)), ##< loosely dashdotted
    (0, (3, 10, 1, 10, 1, 10)), ##< loosely dashdotdotted
    (0, (1, 1)), ##< densely dotted
    (0, (5, 1)), ##< densely dashed
    (0, (3, 1, 1, 1)), ##< densely dashdotted
    (0, (3, 1, 1, 1, 1, 1)) ##< densely dashdotdotted
]

## All available markers, refer to: https://matplotlib.org/stable/api/markers_api.html#module-matplotlib.markers
markers=[".","o","v","^","<",">","1","2","3","4","8","s","p","P","*","h","H","+","x","X","D","d","|","_"]

## The cycler that is used by all the graphs
graph_cycler=None

def export_graph(graph,fname,output='./',prefix='',postfix=''):
    """! @brief save the current plot in png and svg format, also reset the global cycler
    @param[in] graph The plot to export.
    @param[in] fname The name of the output file.
    @param[in] output The output path.
    @param[in] prefix The prefix to preprend to the file name.
    @param[in] postfix The postfix to append to the file name.
    """
    output_path=os.path.join(output,prefix+fname+postfix)
    plt.savefig(output_path+".png",format="png")
    plt.savefig(output_path+".svg",format="svg")
    graph_cycler=None

##@TODO: make possible to chain different plotting functions and create a composed graph (either by stacking plots of by
##using subplots.

def plot(x,y,xlabel=None,ylabel=None,title=None,line_label=None,graph=None):
    """!
    @brief Draws line graph, output will be in png and svg.
    @param[in] x List of x values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] y List of y values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] line_label The label of the line, which will be displayed in a legend, if more than one line is being
    plotted.
    @param[in] graph An already existing figure on which something has already been plotted, lines will be added here.
    @details
    The line graph can contain more than one line and it will be saved in png and svg format.
    Lines will automatically change color, marker and shape, to keep the graph as readable as possible.
    @returns The graph on which the lines have been plotted.
    """
    #get the axes
    if graph==None:
        graph=Figure()
    ax=graph.gca()
    # we set the cycler
    if graph_cycler==None
    #we setup a cycler, which will change line style, markers and color automatically for each line
        graph_cycler=cycler(color=plt.cm.tab20(np.linspace(0,1,lines)))
        if lines<= len(markers):
            graph_cycler+=cycler(marker=markers[:lines])
        ax.set_prop_cycle(cc)
    lines=len(x)
    if lines <= len(linestyle_tuple):
        graph_cycler+=cycler(linestyle=linestyle_tuple[:lines])
    # we create the line collection
    for i in range(0,len(x)):
        if line_label!=None:
            label=line_label[i]
        else:
            label=None
        graph.plot(x[i],y[i],label=label)
    # we set plot properties
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.autoscale()
    if line_label!=None:
        graph.legend()
    ax.grid(True)
    return graph

plot([[1,2,3],[4,5,6],[7,8,9]],[[10,11,12],[13,14,15],[16,17,18]],'test_graph','../','a ',
     'in','xlabel','ylabel','title',['1','2','3','h1','h2'],[2,5,8],[11,14,17])

##@TODO: make also a histogram for WCET-like things


##@TODO: make also a scatterplot

#if __name__ == "__main__":
## @TODO: parse command line arguments (for graph name, type, properties and csv location), then parse csv, draw graph
