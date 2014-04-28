#!/usr/bin/python
import csv
import os
import subprocess
import sys
import time
from helpers.inputhelper import read_filelist
from helpers.sharpsathelper import run_sharpsat_get_number, take_time, run_all_on_list

__author__ = 'thurley'

TIMEOUT = 100  # timeout in seconds

def time_to_string(time, timeout):
    return '--' if time >= timeout else repr(round(time, 3))

def bold(s, yes):
    if yes :
        return "<b>" + s + "</b>"
    return s

def store_results(filename, binary1, binary2, list):
    f = open(filename, 'w')

    num_instances = len(list)
    num_successes1 = 0
    total_time1 = 0
    sync_total_time1 = 0
    num_successes2 = 0
    total_time2 = 0
    sync_total_time2 = 0
    for p in list:

        if p[1] < TIMEOUT:
            num_successes1 += 1
            total_time1 += p[1]
            if p[2] < TIMEOUT:
                sync_total_time1 += p[1]
                sync_total_time2 += p[2]

        if p[2] < TIMEOUT:
            num_successes2 += 1
            total_time2 += p[2]


    total_penalty_time1 = TIMEOUT*(num_instances - num_successes1)
    total_penalty_time2 = TIMEOUT*(num_instances - num_successes2)


    f.write("<!DOCTYPE html><html><header></header>\n<body>")
    f.write("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\" style=\"font-family:arial;width:854px\">\n")
    f.write("<thead><tr>")
    f.write("<td>Name</td><td>" + binary1 + "</td><td>" + binary2 + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Solved</td><td>" + repr(num_successes1) + "/" + repr(len(list)) + "</td><td>" + repr(num_successes2) + "/" + repr(len(list)) + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    onesmaller = total_time1 < total_time2

    f.write("<td>Time </td><td>" + bold(repr(round(total_time1, 3)), onesmaller)+ "</td><td>" + bold(repr(round(total_time2, 3)), not onesmaller) + "</td>")
    onesmaller = sync_total_time1 < sync_total_time2
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Time (only where both succeeded) </td><td>" + bold(repr(round(sync_total_time1, 3)), onesmaller)+ "</td><td>" + bold(repr(round(sync_total_time2, 3)), not onesmaller) + "</td>")
    onesmaller = total_time1 + total_penalty_time1 < total_time2 + total_penalty_time2
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Time (with TIMEOUT penalties) </td><td>" + bold(repr(round(total_time1+total_penalty_time1, 3)), onesmaller)+ "</td><td>" + bold(repr(round(total_time2+total_penalty_time2, 3)), not onesmaller) + "</td>")
    f.write("</tr></thead>")
    f.write("<tbody>")
    for p in list:

        stringtime1 = '--' if p[1] >= TIMEOUT else repr(round(p[1], 3))
        stringtime2 = '--' if p[2] >= TIMEOUT else repr(round(p[2], 3))
        f.write("<tr>")
        onesmaller = p[1] < p[2]
        f.write("<td>" + p[0] + "</td><td>" + bold(stringtime1, onesmaller) + "</td><td>"
                + bold(stringtime2, not onesmaller) + "</td>")
        f.write("</tr>")

    f.write("</tbody>")
    f.write("</table>")
    f.write("</body>\n</html>")


if len(sys.argv) < 3:
    print("Usage:")
    print("\t /comparetimes.py LIST_FILE BINARY1 BINARY2")
    print("\t /comparetimes.py DIRECTORY BINARY1 BINARY2")
    exit(0)


PATH_TO_SHARPSAT_BINARY1 = os.path.expandvars(os.path.expanduser(sys.argv[2]))
PATH_TO_SHARPSAT_BINARY2 = os.path.expandvars(os.path.expanduser(sys.argv[3]))

cnflist = read_filelist(os.path.expandvars(os.path.expanduser(sys.argv[1])))

compare_data = run_all_on_list(TIMEOUT, cnflist, [PATH_TO_SHARPSAT_BINARY1, PATH_TO_SHARPSAT_BINARY2])

store_results("compare_results.html", PATH_TO_SHARPSAT_BINARY1, PATH_TO_SHARPSAT_BINARY2, compare_data)