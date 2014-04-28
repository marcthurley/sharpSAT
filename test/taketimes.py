#!/usr/bin/python
import os
import sys

from os import listdir
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
    for i in [2, len(sys.argv) - 1]:
        binaries.append(os.path.expandvars(os.path.expanduser(sys.argv[i])))
    return os.path.expandvars(os.path.expanduser(sys.argv[1])), binaries


if len(sys.argv) < 3:
    print("Usage: ")
    print("       taketime.py CNF_SOURCE BINARY1 [BINARY2 [BINARY3 ... ]]")


cnffile_source, binaries = extract_from_args()
cnflist = get_filelist(cnffile_source)

#for f in cnflist:
#    print(f)

compare_data = run_all_on_list(TIMEOUT, cnflist, binaries)

store_results("results.html", binaries[0], compare_data)