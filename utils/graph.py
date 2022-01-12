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

## The figure which will contain the plot.
FIGURE = None

## The last index used in to determine an item property (color,linstyle,hatch,marker).
CYCLER_LAST_INDEX = 0

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
hatches = [
    "/",
    "\\",
    "|",
    "-",
    "+",
    "x",
    ".",
    "//",
    "\\",
    "||",
    "--",
    "++",
    "xx",
    "oo",
    "OO",
    "..",
    "**",
    "O.",
    "o",
    "O",
    "*",
]
markers = [
    "v",
    "o",
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
    FIGURE.savefig(output_path + ".png", format="png", bbox_inches="tight")
    FIGURE.savefig(output_path + ".svg", format="svg", bbox_inches="tight")


def teardown():
    """!
    @brief Destroy all the global graph variables
    @details Resets `FIGURE` to `None` and `CYCLER_LAST_INDEX` to `0`
    """
    global FIGURE
    global CYCLER_LAST_INDEX
    FIGURE = None
    CYCLER_LAST_INDEX = 0


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
        FIGURE.legend(
            loc="lower center",
            bbox_to_anchor=(0.125, -0.075, 0.750, 0),
            mode="expand",
            ncol=4,
        )
    graph.grid(grid, axis=grid_axes)


def init_cycler(groups, graph_type):
    """!
    @brief Initializes the cycler with common properties of dots and lines.
    @param[in] groups The number of data groups to plot.
    @param[in] graph_type The name of the function that is producing the graph, as a string.
    """
    # we setup a cycler, which will change line style,
    # markers and color automatically
    global CYCLER_LAST_INDEX
    graph_cycler = cycler(
        color=cm.plasma(
            np.linspace(0.05, 0.85, (CYCLER_LAST_INDEX + groups))[CYCLER_LAST_INDEX:]
        )
    )
    if CYCLER_LAST_INDEX + groups <= len(markers) and graph_type in ["scatter", "plot"]:
        graph_cycler += cycler(
            marker=markers[CYCLER_LAST_INDEX : CYCLER_LAST_INDEX + groups]
        )
    if CYCLER_LAST_INDEX + groups <= len(linestyle_tuple) and graph_type in ["plot"]:
        graph_cycler += cycler(
            linestyle=linestyle_tuple[CYCLER_LAST_INDEX : CYCLER_LAST_INDEX + groups]
        )
    if CYCLER_LAST_INDEX + groups <= len(hatches) and graph_type in ["bar", "hist"]:
        graph_cycler += cycler(
            hatch=hatches[CYCLER_LAST_INDEX : CYCLER_LAST_INDEX + groups]
        )
    CYCLER_LAST_INDEX += groups
    return graph_cycler


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
    if type(x[0]) is list:
        xlist = x
        ylist = y
        groups = len(xlist)
    else:
        xlist = [x]
        ylist = [y]
        groups = len(xlist)

    # we set the cycler
    graph_cycler = init_cycler(groups, "plot")
    graph.set_prop_cycle(graph_cycler)
    # we create the line collection
    for i in range(0, groups):
        if line_label is not None:
            label = line_label[i]
        else:
            label = None
        graph.plot(xlist[i], ylist[i], label=label)
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
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    set_graph_properties(graph, xlabel, ylabel, title, box_labels, True, "y")
    graph_cycler = init_cycler(1, "hist")
    graph.set_prop_cycle(graph_cycler)
    hist = graph.hist(
        data,
        bins,
        label=box_labels,
        density=density,
        hatch=graph_cycler.by_key().get("hatch")[0],
    )
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
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    # we set the cycler
    if type(data[0]) is list:
        groups = len(data)
        latest_data = np.zeros(len(data[0]))
    else:
        groups = 1
        latest_data = np.zeros(len(data))

    graph_cycler = init_cycler(groups, "bar")
    graph.set_prop_cycle(graph_cycler)
    # we create the bar chart
    for i in range(groups):
        if bar_label is not None:
            label = bar_label[i]
        else:
            label = None
        if type(data[0]) is list:
            xvals = range(len(data[i]))
            yvals = data[i]
        else:
            xvals = range(len(data))
            yvals = data
        bar = graph.bar(
            xvals,
            yvals,
            tick_label=xlabel,
            bottom=latest_data,
            label=label,
            hatch=graph_cycler.by_key().get("hatch")[i],
        )
        graph.bar_label(
            bar,
            fmt="%.3g",
            label_type="center",
            bbox={"boxstyle": "circle", "color": "white"},
        )
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
    """
    # get the axes
    if graph is None:
        global FIGURE
        FIGURE = Figure()
        graph = FIGURE.gca()
    graph_cycler = init_cycler(len(x), "scatter")
    graph.set_prop_cycle(graph_cycler)
    # we create the line collection
    for i in range(0, len(x)):
        if group_label is not None:
            label = group_label[i]
        else:
            label = None
        graph.plot(x[i], y[i], linestyle="", label=label)
    # we set plot properties
    set_graph_properties(graph, xlabel, ylabel, title, group_label)
    return graph


def test():
    graph = plot(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [1, 2, 3, 4],
        "ylabel",
        "title",
        ["1", "2", "3", "4"],
    )
    export_graph(graph, "test-plot")

    graph = scatter(
        [[1, 2], [3, 4]],
        [[1, 2], [3, 4]],
        xlabel="xlabel",
        ylabel="ylabel",
        title="title",
        group_label=["group1", "group2"],
    )
    export_graph(graph, "test-scatter")

    graph = hist(
        [1, 2, 3, 4, 5],
        bins=3,
        density=False,
        xlabel="xlabel",
        ylabel="ylabel",
        title="title",
    )
    export_graph(graph, "test-hist")
    graph = bar(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [1, 2, 3, 4],
        "ylabel",
        "title",
        ["1", "2", "3", "4"],
    )
    export_graph(graph, "test-bar")


if __name__ == "__main__":
    test()
