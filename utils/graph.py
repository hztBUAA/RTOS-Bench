#! /bin/python
"""!
@file graph.py
@brief Draw graphs with matplotlib from csv files containing data
@details
This script contains a collection of methods to draw graphs starting from data in csv files.
@author Mattia Nicolella
"""
import os
import numpy as np
from matplotlib import cm
from matplotlib.figure import Figure
from cycler import cycler

##The cycler used for dots and lines.
CYCLER_DOTS = None

##The cycler used for boxes and bars.
CYCLER_BOX = None

## The figure which will contain the plot.
FIGURE = None

linestyle_tuple = [
    (0, ()),  ##< solid
    (0, (1, 1)),  ##< dotted
    (0, (5, 5)),  ##< dashed
    (0, (3, 5, 1, 5)),  ##< dashdotted
    (0, (3, 5, 1, 5, 1, 5)),  ##< dashdotdotted
    (0, (1, 10)),  ##< loosely dotted
    (0, (5, 10)),  ##< loosely dashed
    (0, (3, 10, 1, 10)),  ##< loosely dashdotted
    (0, (3, 10, 1, 10, 1, 10)),  ##< loosely dashdotdotted
    (0, (1, 1)),  ##< densely dotted
    (0, (5, 1)),  ##< densely dashed
    (0, (3, 1, 1, 1)),  ##< densely dashdotted
    (0, (3, 1, 1, 1, 1, 1)),  ##< densely dashdotdotted
]
hatches = ["/", "\\", "|", "-", "+", "x", "o", "O", ".", "*"]
markers = [
    ".",
    "o",
    "v",
    "^",
    "<",
    ">",
    "1",
    "2",
    "3",
    "4",
    "8",
    "s",
    "p",
    "P",
    "*",
    "h",
    "H",
    "+",
    "x",
    "X",
    "D",
    "d",
    "|",
    "_",
]


def export_graph(graph, fname, output="./", prefix="", postfix=""):
    """! @brief save the current plot in png and svg format, also reset the global cycler
    @param[in] graph The plot to export.
    @param[in] fname The name of the output file.
    @param[in] output The output path.
    @param[in] prefix The prefix to preprend to the file name.
    @param[in] postfix The postfix to append to the file name.
    """
    output_path = os.path.join(output, prefix + fname + postfix)
    global FIGURE
    FIGURE.savefig(output_path + ".png", format="png")
    FIGURE.savefig(output_path + ".svg", format="svg")
    global CYCLER
    CYCLER = None
    FIGURE = None


##@TODO: Make possible to print a single group of data.


def set_graph_properties(
    graph,
    xlabel=None,
    ylabel=None,
    title=None,
    legend=None,
    grid=True,
    grid_axes="both",
):
    """!
    @brief Set common graph properties.
    @param[in] graph The graph which properties need to be adjusted.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] legend The legend that must be shown.
    @oaram[in] grid Whether to draw a grid.
    @param[in] grid_axes The axes that should be used for the grid (x,y or both)
    @details
    In addition the graph will also be scaled and a grid will be shown.
    """
    graph.autoscale()
    if title is not None:
        graph.set_title(title)
    if xlabel is not None:
        graph.set_xlabel(xlabel)
    if ylabel is not None:
        graph.set_ylabel(ylabel)
    if legend is not None:
        graph.legend(legend)
    graph.grid(grid, axis=grid_axes)


def init_cycler_for_boxes(groups):
    """!
    @brief Initializes the cycler with common properties of boxes and bars.
    @param[in] groups The number of data groups to plot.
    """
    global CYCLER_BOX
    CYCLER_BOX = cycler(color=cm.tab20(np.linspace(0, 1, groups)))
    if groups <= len(hatches):
        CYCLER_BOX += cycler(hatch=hatches[:groups])


def init_cycler_for_dots(groups):
    """!
    @brief Initializes the cycler with common properties of dots and lines.
    @param[in] groups The number of data groups to plot.
    """
    # we setup a cycler, which will change line style,
    # markers and color automatically
    global CYCLER_DOTS
    CYCLER_DOTS = cycler(color=cm.tab20(np.linspace(0, 1, groups)))
    if groups <= len(markers):
        CYCLER_DOTS += cycler(marker=markers[:groups])
    if groups <= len(linestyle_tuple):
        CYCLER_DOTS += cycler(linestyle=linestyle_tuple[:groups])


def plot(x, y, xlabel=None, ylabel=None, title=None, line_label=None, graph=None):
    """!
    @brief Draws a line graph, output will be in png and svg.
    @param[in] x List of x values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] y List of y values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] line_label The label of the line, which will be displayed in a legend, if more than one line is being
    plotted.
    @param[in] graph An already existing Axes object, lines will be added here.
    @details
    Lines will automatically change color, marker and shape, to keep the graph as readable as possible.
    @returns The graph on which the lines have been plotted.
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    # we set the cycler
    if CYCLER_DOTS is None:
        init_cycler_for_dots(len(x))
        graph.set_prop_cycle(CYCLER_DOTS)
    # we create the line collection
    for i in range(0, len(x)):
        if line_label is not None:
            label = line_label[i]
        else:
            label = None
        graph.plot(x[i], y[i], label=label)
    # we set plot properties
    set_graph_properties(graph, xlabel, ylabel, title, line_label)
    return graph


def hist(
    data,
    bins=None,
    density=False,
    xlabel=None,
    ylabel=None,
    title=None,
    box_labels=None,
    graph=None,
):
    """!
    @brief Draws a histogram.
    @param[in] data The data to plot in the histogram.
    @param[in] bins The number of bins.
    @param[in] density If a probability density should be visualized, instead of just counting the occurencies.
    @param[in] xlabel The label for the x-axis.
    @param[in] ylabel The label for the y-axis.
    @param[in] title The title of the graph.
    @param[in] box_labels The labels of the bins.
    @param[in] graph An already existing Axes object, the histogram will be added here.
    @TODO: fix hatches
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    # we set the cycler
    if CYCLER_BOX is None:
        init_cycler_for_boxes(len(bins))
        graph.set_prop_cycle(CYCLER_BOX)
    set_graph_properties(graph, xlabel, ylabel, title, box_labels, True, "y")
    graph.hist(data, bins, label=box_labels, density=density)
    return graph


def bar(data, xlabel=None, ylabel=None, title=None, bar_label=None, graph=None):
    """!
    @brief Draws a bar graph.
    @param[in] data List of values to be plotted is more than one group of bars
    is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the bars in the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] bar_label The label of each bar group, which will be displayed in a
    legend, one per groups of bars.
    @param[in] graph An already existing Axes object, bars will be added here.
    @details
    Bar groups will automatically change color, marker and shape, to keep the
    graph as readable as possible.
    @returns The graph on which the bars have been plotted.
    @TODO: fix hatches
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    # we set the cycler
    if CYCLER_BOX is None:
        init_cycler_for_boxes(len(data))
        graph.set_prop_cycle(CYCLER_BOX)
    # we create the bar chart
    latest_data = np.zeros(len(data[0]))
    for i in range(len(data)):
        graph.bar(range(len(data[i])), data[i], tick_label=xlabel, bottom=latest_data)
        latest_data += np.asarray(data[i])
    # we set plot properties
    set_graph_properties(graph, None, ylabel, title, bar_label, True, "y")
    return graph


def scatter(x, y, xlabel=None, ylabel=None, title=None, group_label=None, graph=None):
    """!
    @brief Draws a scatter  graph, output will be in png and svg.
    @param[in] x List of x values to be plotted, if more than one group of points is being plotted, then this is a list of lists.
    @param[in] y List of y values to be plotted, if more than one group of points is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] group_label The label of the group of points, which will be displayed in a legend, if more than one group is being
    plotted.
    @param[in] graph An already existing Axes object, lines will be added here.
    @details
    Groups of points will automatically change color, marker and shape, to keep the graph as readable as possible.
    @returns The graph on which the lines have been plotted.
    @TODO fix markers
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    # we set the cycler
    if CYCLER_DOTS is None:
        init_cycler_for_dots(len(x))
        graph.set_prop_cycle(CYCLER_DOTS)
    # we create the line collection
    for i in range(0, len(x)):
        graph.scatter(x[i], y[i])
    # we set plot properties
    set_graph_properties(graph, xlabel, ylabel, title, group_label)
    return graph


graph = bar(
    [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
    [1, 2, 3, 4],
    "ylabel",
    "title",
    ["1", "2", "3", "4"],
)

export_graph(graph, "test")


# if __name__ == "__main__":
## @TODO: parse command line arguments (for graph name, type, properties and csv location), then parse csv, draw graph
