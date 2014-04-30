#!/usr/bin/python
import os
import sys

from os import listdir
from helpers.htmlresume import Htmltable, Htmlwriter
from helpers.inputhelper import read_filelist
from helpers.sharpsathelper import run_all_on_list

__author__ = 'thurley'

TIMEOUT = 100  # timeout in seconds

def bold(s, yes):
    if yes :
        return "<b>" + s + "</b>"
    return s

def store_results(filename, binary, list):
    f = open(filename, 'w')

    num_instances = len(list)
    num_successes = 0
    total_time = 0
    for p in list:

        if p[1] < TIMEOUT:
            num_successes += 1
            total_time += p[1]


    total_penalty_time = TIMEOUT*(num_instances - num_successes)

    f.write("<!DOCTYPE html><html><header></header>\n<body>")
    f.write("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\" style=\"font-family:arial;width:854px\">\n")
    f.write("<thead><tr>")
    f.write("<td>Name</td><td>" + binary + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Solved</td><td>" + repr(num_successes) + "/" + repr(len(list)) + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Time </td><td>" + bold(repr(round(total_time, 3)), False)+ "</td>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Time (with TIMEOUT penalties) </td><td>" + bold(repr(round(total_time+total_penalty_time, 3)), False)+ "</td>")
    f.write("</tr></thead>")
    f.write("<tbody>")
    for p in list:

        stringtime = '--' if p[1] >= TIMEOUT else repr(round(p[1], 3))
        f.write("<tr>")
        f.write("<td>" + p[0] + "</td><td>" + bold(stringtime, False) + "</td>")
        f.write("</tr>")

    f.write("</tbody>")
    f.write("</table>")
    f.write("</body>\n</html>")

def concat(a, list):
    l = [a]
    l.extend(list)
    return l

def store_results_as_html(filename, binaries, list):


    htmlwriter = Htmlwriter("test.html")

    htmltable = Htmltable()

    num_instances = len(list)
    all_successes = []
    all_total_time = []
    all_with_penalty_time = []

    opt_successes = 0
    opt_total_time = len(list) * TIMEOUT
    opt_with_penalty_time = len(list) * TIMEOUT
    for i in range(1, len(binaries)+1):
        num_successes = 0
        total_time = 0
        penalty_time = 0
        for p in list:
            if p[i] < TIMEOUT:
                num_successes += 1
                total_time += p[i]
            else:
                penalty_time += TIMEOUT

        if num_successes > opt_successes:
            opt_successes = num_successes
        if total_time < opt_total_time:
            opt_total_time = total_time
        if total_time + penalty_time < opt_with_penalty_time:
            opt_with_penalty_time = total_time + penalty_time

        all_successes.append(num_successes)
        all_total_time.append(total_time)
        all_with_penalty_time.append(total_time + penalty_time)

    str_all_successes = []
    str_all_total_time = []
    str_all_with_penalty_time = []

    for i in range(0, len(all_successes)):
        str_all_successes.append(["opt-head" if all_successes[i] >= opt_successes else "", repr(all_successes[i])])
        str_all_total_time.append(["opt-head" if all_total_time[i] <= opt_total_time else "", repr(round(all_total_time[i], 3))])
        str_all_with_penalty_time.append(["opt-head" if all_with_penalty_time[i] <= opt_with_penalty_time else "", repr(round(all_with_penalty_time[i], 3))])

    head = [["head", "Name"]]
    for b in binaries:
        head.append(["head", b])

    htmltable.add_header_w_cell_classes(head)
    htmltable.add_header_w_cell_classes(concat(["head", "Solved (out of " + repr(num_instances) + ")"], str_all_successes))
    htmltable.add_header_w_cell_classes(concat(["head", "Time"], str_all_total_time))
    htmltable.add_header_w_cell_classes(concat(["head", "Time (with penalties)"], str_all_with_penalty_time))

    for p in list:
        row_of_times = []
        opt = 1
        for i in range(1, len(binaries)+1):
            stringtime = '--' if p[i] >= TIMEOUT else repr(round(p[i], 3))
            if (p[i] < p[opt]):
                opt = i
            row_of_times.append(["", stringtime])

        for i in range(1, len(binaries)+1):
            if (p[i] == p[opt]):
                row_of_times[i-1][0] = "optimal"

        htmltable.add_row_w_cell_classes(concat(["", p[0]], row_of_times))

    htmlwriter.make_document("<style type=\"text/css\"> .optimal{ background: lightgreen; } .head { font-weight:bold; }  .opt-head{ background: lightgrey;}</style>", htmltable.make())

def get_filelist(cnffile_source):
    list = []
    if os.path.isdir(cnffile_source):
        print("Directory source found. Scanning for .cnf files ...")
        #os.chdir(cnffile_source)
        for (dpath, dnames, filenames) in os.walk(cnffile_source):
            #print(dpath)
            for f in filenames:
                #print(f + ": " + repr(os.path.isfile(dpath + f)))
                if os.path.isfile(dpath + "/" + f) and f.split(".")[len(f.split(".")) - 1] == "cnf":
                    list.append(dpath + "/" + f)
    else:
        print("A file with a list of cnf files found.")
        list = read_filelist(cnffile_source)
    print("DONE")
    return list


def extract_from_args():
    binaries = []
    for i in range(2, len(sys.argv)):
        binaries.append(os.path.expandvars(os.path.expanduser(sys.argv[i])))
    return os.path.expandvars(os.path.expanduser(sys.argv[1])), binaries

if len(sys.argv) < 3:
    print("Usage: ")
    print("       taketime.py CNF_SOURCE BINARY1 [BINARY2 [BINARY3 ... ]]")


cnffile_source, binaries = extract_from_args()
cnflist = get_filelist(cnffile_source)

for f in binaries:
    print(f)

compare_data = run_all_on_list(TIMEOUT, cnflist, binaries)

store_results("results.html", binaries[0], compare_data)
store_results_as_html("test.html", binaries, compare_data)