#! /bin/python
"""!
@file graph.py
@ingroup graph
@brief Utils to draw graphs with matplotlib.
@details
This script contains a collection of methods to draw graphs.
@author Mattia Nicolella

@copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
SPDX-License-Identifier: MIT
"""
import argparse
import csv
import os

import numpy as np
from cycler import cycler
from matplotlib import cm
from matplotlib.pyplot import figure

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
    """! @brief save the current plot in png, pdf and svg format, also reset the global cycler.

    @param[in] graph The plot to export.
    @param[in] fname The name of the output file.
    @param[in] output The output path.
    @param[in] prefix The prefix to preprend to the file name.
    @param[in] postfix The postfix to append to the file name.
    """
    output_path = os.path.join(output, prefix + fname + postfix)
    global FIGURE
    FIGURE.savefig(output_path + ".png", format="png", bbox_inches="tight", dpi=300)
    FIGURE.savefig(output_path + ".svg", format="svg", bbox_inches="tight", dpi=300)
    FIGURE.savefig(output_path + ".pdf", format="pdf", bbox_inches="tight", dpi=300)


def teardown():
    """! @brief Destroy all the global graph variables.

    @details Resets `FIGURE` to `None` and `CYCLER_LAST_INDEX` to `0`
    """
    global FIGURE
    global CYCLER_LAST_INDEX
    FIGURE = None
    CYCLER_LAST_INDEX = 0


def set_figsize(width, height):
    """! @brief Sets the size of the figure.

    @param[in] width Figure width.
    @param[in] height Figure height.
    """
    global FIGURE
    FIGURE = figure(figsize=(width, height), tight_layout=True)


def set_graph_properties(
    graph,
    xlabel=None,
    ylabel=None,
    title=None,
    legend=None,
    grid=True,
    grid_axes="both",
):
    """! @brief Set common graph properties.

    @param[in] graph The graph which properties need to be adjusted.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] legend The legend that must be shown.
    @param[in] grid Whether to draw a grid.
    @param[in] grid_axes The axes that should be used for the grid (x,y or both)
    @details
    In addition the graph will also be scaled and a grid will be shown.
    """
    # graph.autoscale()
    if title is not None:
        graph.set_title(title)
    if xlabel is not None:
        graph.set_xlabel(xlabel)
    if ylabel is not None:
        graph.set_ylabel(ylabel)
    if legend is not None:
        graph.legend(
            # loc="upper right",
            # bbox_to_anchor=(0.125, 1, 0.775, 0),
            # mode="expand",
        )
    graph.grid(grid, axis=grid_axes)


def adjust_x_ticks_labels(graph, labels):
    """! @brief If the labels for the x ticks are too long rotates them.
    @param[in] graph The graph on which labels should be checked.
    @param[in] labels The list of labels to adjust.
    @details
    `\n` is taken into account when computing the label length.
    """
    lbl_len_list = list(map(len, labels))
    lbl_max_idx = lbl_len_list.index(max(lbl_len_list))
    lbl_max_len = lbl_len_list[lbl_max_idx] / (labels[lbl_max_idx].count("\n") + 1)
    if len(labels) > 4 and lbl_max_len > 10:
        graph.tick_params(axis="x", rotation=22)


def init_cycler(groups, graph_type):
    """! @brief Initializes the cycler with common properties of dots and lines.

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


def plot(
    x,
    y,
    xlabel=None,
    ylabel=None,
    title=None,
    line_label=None,
    log_scale=False,
    line_markers=True,
    graph=None,
):
    """! @brief Draws a line graph, output will be in png and svg.

    @param[in] x List of x values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] y List of y values to be plotted, if more than one line is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] line_label The label of the line, which will be displayed in a legend, if more than one line is being
    plotted.
    @param[in] log_scale If the scale of the plot must be logarithmic.
    @param[in] line_markers If point markers have to be painted.
    @param[in] graph An already existing Axes object, lines will be added here.
    @details
    Lines will automatically change color, marker and shape, to keep the graph as readable as possible.
    @returns The graph on which the lines have been plotted.
    """
    # get the axes
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
        graph = FIGURE.gca()
    if hasattr(x[0], "__iter__"):
        xlist = x
        ylist = y
        groups = len(xlist)
    else:
        xlist = [x]
        ylist = [y]
        groups = len(xlist)

    # we set the cycler
    if line_markers is True:
        graph_cycler = init_cycler(groups, "plot")
    else:
        graph_cycler = init_cycler(groups, "plot-nomarkers")
    graph.set_prop_cycle(graph_cycler)
    # we create the line collection
    for i in range(0, groups):
        if line_label is not None:
            label = line_label[i]
        else:
            label = None
        graph.plot(
            xlist[i],
            ylist[i],
            label=label,
        )
    # we set plot properties
    if log_scale:
        graph.set_yscale("log")
        graph.set_xscale("log")
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
    log_scale=False,
    graph=None,
):
    """! @brief Draws a histogram.

    @param[in] data The data to plot in the histogram.
    @param[in] bins The number of bins.
    @param[in] density If a probability density should be visualized, instead of just counting the occurencies.
    @param[in] xlabel The label for the x-axis.
    @param[in] ylabel The label for the y-axis.
    @param[in] title The title of the graph.
    @param[in] box_labels The labels of the bins.
    @param[in] log_scale If the logarithmic scale must be used to plot data.
    @param[in] graph An already existing Axes object, the histogram will be added here.
    """
    # get the axes
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
        graph = FIGURE.gca()
    graph_cycler = init_cycler(1, "hist")
    graph.set_prop_cycle(graph_cycler)
    graph = graph.hist(
        data,
        bins,
        label=box_labels,
        density=density,
        hatch=graph_cycler.by_key().get("hatch")[0],
    )
    if log_scale:
        graph.set_yscale("log")
    set_graph_properties(graph, xlabel, ylabel, title, box_labels, True, "y")
    adjust_x_ticks_labels(graph, box_labels)
    return graph


def bar(
    data,
    xlabel,
    ylabel=None,
    title=None,
    bar_label=None,
    log_scale=False,
    stacked=False,
    graph=None,
):
    """! @brief Draws a bar graph.

    @param[in] data List of values to be plotted is more than one group of bars
    is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the bars in the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] bar_label The label of each bar group, which will be displayed in a
    legend, one per groups of bars.
    @param[in] log_scale If the logarithmic scale must be used to plot data.
    @param[in] stacked True to plot a stacked bar chart, false to plot a grouped bar char.
    @param[in] graph An already existing Axes object, bars will be added here.
    @details
    Bar groups will automatically change color, marker and shape, to keep the
    graph as readable as possible.
    @returns The graph on which the bars have been plotted.
    """
    # get the axes
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
        graph = FIGURE.gca()
    # we set the cycler
    if hasattr(data[0], "__iter__"):
        groups = len(data)
        latest_data = np.zeros(len(data[0]))
    else:
        groups = 1
        latest_data = np.zeros(len(data))
    if stacked is True:
        bar_width = 1
    else:
        bar_width = 0.8 / groups
    graph_cycler = init_cycler(groups, "bar")
    graph.set_prop_cycle(graph_cycler)
    # we create the bar chart
    for i in range(groups):
        if bar_label is not None:
            label = bar_label[i]
        else:
            label = None
        if hasattr(data[0], "__iter__"):
            xvals = np.arange(0, len(data[i]))
            yvals = data[i]
        else:
            xvals = np.arange(0, len(data))
            yvals = data
        bar_graph = graph.bar(
            xvals + (i * bar_width),
            yvals,
            tick_label=xlabel,
            bottom=latest_data,
            label=label,
            hatch=graph_cycler.by_key().get("hatch")[i],
            width=bar_width,
        )
        if stacked is True:
            latest_data += np.asarray(data[i])
        else:
            if groups > 1:
                graph.set_xticks(xvals + bar_width / groups)
                graph.set_xticklabels(xlabel)
        # graph.bar_label(
        # bar_graph,
        # fmt="%.3g",
        # label_type="edge",
        # bbox={"boxstyle": "circle", "color": "white"},
        # )
    # we set plot properties
    if log_scale:
        graph.set_yscale("log")
    set_graph_properties(graph, None, ylabel, title, bar_label, True, "y")
    adjust_x_ticks_labels(graph, xlabel)
    return graph


def scatter(
    x,
    y,
    xlabel=None,
    ylabel=None,
    title=None,
    group_label=None,
    log_scale=False,
    graph=None,
):
    """! @brief Draws a scatter  graph.

    @param[in] x List of x values to be plotted, if more than one group of points is being plotted, then this is a list of lists.
    @param[in] y List of y values to be plotted, if more than one group of points is being plotted, then this is a list of lists.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.
    @param[in] group_label The label of the group of points, which will be displayed in a legend, if more than one group is being
    plotted.
    @param[in] log_scale If the logarithmic scale must be used to plot data.
    @param[in] graph An already existing Axes object, lines will be added here.
    @details
    Groups of points will automatically change color, marker and shape, to keep the graph as readable as possible.
    @returns The graph on which the lines have been plotted.
    """
    # get the axes
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
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
    if log_scale:
        graph.set_xscale("log")
        graph.set_yscale("log")
    set_graph_properties(graph, xlabel, ylabel, title, group_label)
    return graph


def boxplot(
    data,
    labels=None,
    xlabel=None,
    ylabel=None,
    title=None,
    notch=False,
    log_scale=False,
    graph=None,
):
    """! @brief Draws a boxplot graph.

    @param[in] data The data that needs to be plotted, it can be a 2D array.
    If so, a boxplot per column, man will be plotted.
    @param[in] labels The labels for each dataset.
    @param[in] notch If a notch needs to be drawn.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.False
    @param[in] log_scale if the scale of the plot must be logarithmic.
    @param[in] graph An already existing Axes object, lines will be added here.
    """
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
        graph = FIGURE.gca()
    graph_cycler = init_cycler(len(data), "scatter")
    graph.set_prop_cycle(graph_cycler)
    graph.boxplot(data, whis=(0, 100), labels=labels, notch=notch)
    if log_scale:
        graph.set_yscale("log")
    set_graph_properties(graph, xlabel, ylabel, title)
    adjust_x_ticks_labels(graph, labels)
    return graph


def violinplot(
    data,
    labels=None,
    xlabel=None,
    ylabel=None,
    title=None,
    log_scale=False,
    graph=None,
):
    """! @brief Draws a violinplot graph.

    @param[in] data The data that needs to be plotted, it can be a 2D array.
    If so, a boxplot per columns, man will be plotted.
    @param[in] labels The labels for each dataset.
    @param[in] xlabel The label for the x axis.
    @param[in] ylabel The label for the y axis.
    @param[in] title The graph title.False
    @param[in] log_scale if the scale of the plot must be logarithmic.
    @param[in] graph An already existing Axes object, lines will be added here.
    """
    if graph is None:
        global FIGURE
        if FIGURE is None:
            FIGURE = figure(tight_layout=True)
        graph = FIGURE.gca()
    graph_cycler = init_cycler(len(data), "scatter")
    graph.set_prop_cycle(graph_cycler)
    graph.violinplot(data, showmeans=True, showextrema=True)
    if type(labels) == list:
        graph.set_xticks(range(0, len(labels) + 1))
        graph.set_xticklabels(labels=[""] + labels)
    if log_scale:
        graph.set_yscale("log")
    set_graph_properties(graph, xlabel, ylabel, title)
    if type(labels) == list:
        adjust_x_ticks_labels(graph, labels)
    return graph


def test():
    """! @brief A test function to check if graphs are generated correctly."""
    graph = plot(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [1, 2, 3, 4],
        "ylabel",
        "title",
        ["1", "2", "3", "4"],
        log_scale=True,
    )
    export_graph(graph, "test-plot")

    graph = scatter(
        [[1, 2], [3, 4]],
        [[1, 2], [3, 4]],
        xlabel="xlabel",
        ylabel="ylabel",
        title="title",
        group_label=["group1", "group2"],
        log_scale=True,
    )
    export_graph(graph, "test-scatter")

    graph = hist(
        [1, 2, 3, 4, 5],
        bins=3,
        density=False,
        xlabel="xlabel",
        ylabel="ylabel",
        title="title",
        log_scale=True,
    )
    export_graph(graph, "test-hist")
    graph = bar(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        [1, 2, 3, 4],
        "ylabel",
        "title",
        ["1", "2", "3", "4"],
        log_scale=True,
    )
    export_graph(graph, "test-bar")

    graph = boxplot(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        ["lbl1", "2", "3", "lbl4"],
        "xlabel",
        "ylabel",
        "title",
        log_scale=True,
    )
    export_graph(graph, "test-boxplot")
    graph = violinplot(
        [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16]],
        ["lbl1", "2", "3", "lbl4"],
        "xlabel",
        "ylabel",
        "title",
        log_scale=True,
    )
    export_graph(graph, "test-violinplot")


def parser_init():
    """! @brief Initialize an argument parser with a set of common arguments.

    @returns The initialized `ArgumentParser` object.
    """
    # set up the argument parser
    parser = argparse.ArgumentParser(
        description="A script to create graphs from csv files."
    )

    parser.add_argument(
        "-g",
        "--graph",
        metavar="graph",
        nargs="+",
        type=str,
        help="Which type of graph must be plotted. If repeated plots will stack.",
        choices=[
            "plot",
            "bar",
            "hist",
            "scatter",
            "boxplot",
            "violin",
            "test",
        ],
        required=True,
        dest="graph_type",
    )

    parser.add_argument(
        "-i",
        "--input",
        metavar="file1,file2,...",
        nargs="+",
        type=str,
        help="Path to the input file. Can be repeated.",
        required=False,
        dest="input_files",
    )

    parser.add_argument(
        "-o",
        "--output",
        metavar="path",
        type=str,
        help="The path where the graph will saved.",
        required=False,
        default="./",
        dest="output",
    )

    parser.add_argument(
        "-pre",
        "--prefix",
        metavar="prefix",
        type=str,
        help="A prefix to set on the graph files",
        required=False,
        default="",
        dest="prefix",
    )

    parser.add_argument(
        "-post",
        "--postfix",
        metavar="postfix",
        type=str,
        help="A postfix to set on the generated files",
        required=False,
        default="",
        dest="postfix",
    )

    parser.add_argument(
        "-xc",
        "--x-column",
        metavar="column1,column2,...",
        nargs="+",
        type=str,
        help="CSV column that contain the data to plot on the x axis. Can be repeated (one for each graph type).",
        required=True,
        dest="x_cols",
    )

    parser.add_argument(
        "-yc",
        "--y-column",
        metavar="column1,column2,...",
        nargs="+",
        type=str,
        help="CSV column that contain the data to plot on the y axis. Can be repeated (one for each graph type).",
        required=True,
        default=None,
        dest="y_cols",
    )

    parser.add_argument(
        "-lc",
        "--labels",
        metavar="label1,label2,...",
        nargs="+",
        type=str,
        help="Labels for each group of data.",
        required=False,
        default=None,
        dest="labels",
    )

    parser.add_argument(
        "-t",
        "--title",
        metavar="title",
        type=str,
        help="Graph title.",
        required=False,
        default=None,
        dest="title",
    )

    parser.add_argument(
        "-xlbl",
        "--xlabel",
        metavar="x-label",
        type=str,
        help="Graph label for the x axis.",
        required=False,
        default=None,
        dest="x_label",
    )

    parser.add_argument(
        "-ylbl",
        "--ylabel",
        metavar="y-label",
        type=str,
        help="Graph label for the y axis.",
        required=False,
        default=None,
        dest="y_label",
    )

    parser.add_argument(
        "-hb",
        "--hist-bins",
        metavar="bins-num",
        type=int,
        help="Number of bins for histogram graph",
        required=False,
        default=0,
        dest="bins",
    )
    parser.add_argument(
        "-hd",
        "--hist-density",
        type=bool,
        help="To make the the histogram consider the density of samples instead of their number.",
        required=False,
        default=False,
        choices=[True, False],
        dest="density",
    )
    return parser


def read_data(files, x_cols, y_cols=None):
    x_data = []
    if y_cols is not None:
        y_data = []
    else:
        y_data = None
    for tup in enumerate(files):
        file_str = tup[1]
        x_col = x_cols[tup[0]]
        y_col = y_cols[tup[0]]
        try:
            data_file = open(file_str)
        except Exception as e:
            print(f"file {file_str} not found: {e}")
            return None
        reader = csv.DictReader(data_file)
        # skip the header
        next(reader)
        x_tmp = []
        if y_col is not None:
            y_tmp = []
        else:
            y_tmp = None
        for row in reader:
            x_tmp.append(float(row[x_col]))
            if y_col is not None:
                y_tmp.append(float(row[y_col]))
        x_data.append(x_tmp)
        if y_col is not None:
            y_data.append(y_tmp)
    if len(files) == 1:
        x_data = x_tmp
        y_data = y_tmp
    return x_data, y_data


if __name__ == "__main__":
    parser = parser_init()
    args = parser.parse_args()
    if "test" in args.graph_type:
        test()
    else:
        path_is_file = os.path.isfile(args.output) or not os.path.exists(args.output)
        if path_is_file:
            output_path, output_file = os.path.split(args.output)
        else:
            output_path = args.output
            output_file = "graph"
        if not os.path.exists(output_path):
            print(f"{output_path} is not a valid path!")
            quit(-1)
        x_data, y_data = read_data(args.input_files, args.x_cols, args.y_cols)
        graph = None
        for graph_type in args.graph_type:
            if "plot" == graph_type:
                graph = plot(
                    x_data,
                    y_data,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    line_label=args.labels,
                    graph=graph,
                )
            elif "bar" == graph_type:
                graph = bar(
                    x_data,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    bar_label=args.label,
                    graph=graph,
                )
            elif "hist" == graph_type:
                graph = hist(
                    x_data,
                    bins=args.bins,
                    density=args.density,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    box_labels=args.labels,
                    graph=graph,
                )
            elif "scatter" == graph_type:
                graph = scatter(
                    x_data,
                    y_data,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    group_label=args.labels,
                    graph=graph,
                )
            elif "boxplot" == graph_type:
                graph = boxplot(
                    x_data,
                    labels=args.labels,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    graph=graph,
                )
            elif "violin" == graph_type:
                graph = violinplot(
                    x_data,
                    xlabel=args.x_label,
                    ylabel=args.y_label,
                    title=args.title,
                    graph=graph,
                )
        export_graph(
            graph,
            output_file,
            output=output_path,
            prefix=args.prefix,
            postfix=args.postfix,
        )
