#!/usr/bin/python
import csv
import os
import subprocess
import sys
import time
from helpers.sharpsathelper import run_sharpsat_get_number

__author__ = 'thurley'

TIMEOUT = 200  # timeout in seconds


def read_filelist(filename):
    list = []
    with open(filename, "r") as countscsvfile:
        #countreader = csv.reader(countscsvfile)
        for line in countscsvfile:
            cnfname = os.path.expandvars(os.path.expanduser(line))
            list.append(cnfname.rstrip())
    return list

def take_time(binary, arg, timeout):
    try:
        start = time.time()
        number = run_sharpsat_get_number(binary, arg, timeout)
        time_taken = time.time() - start
    except RuntimeError as e:
        time_taken = 2*timeout
    except subprocess.CalledProcessError as E:
        erroroutput = "sharpSAT exited with exit code" + str(E.returncode) + "\n" + E.output
        time_taken = 2*timeout
    return time_taken


def compare_on_list(binary1, binary2, cnf_list):
    numfail = 0
    list = []
    for p in cnf_list:

        time1 = take_time(binary1, p, TIMEOUT)
        time2 = take_time(binary2, p, TIMEOUT)

        list.append([p, time1, time2])
    return list

def print_results(binary1, binary2, list):
    print('\t\t\t\t\t' + binary1 + '\t' + binary2)

    for p in list:

        stringtime1 = '--' if p[1] >= TIMEOUT else repr(round(p[1], 3))
        stringtime2 = '--' if p[2] >= TIMEOUT else repr(round(p[2], 3))

        print(p[0] + '\t' + stringtime1 + '\t' + stringtime2)


def bold(s, yes):
    if yes :
        return "<b>" + s + "</b>"
    return s

def store_results(filename, binary1, binary2, list):
    f = open(filename, 'w')

    num_successes1 = 0
    total_time1 = 0
    num_successes2 = 0
    total_time2 = 0
    for p in list:

        if p[1] < TIMEOUT:
            num_successes1 += 1
            total_time1 += p[1]

        if p[2] < TIMEOUT:
            num_successes2 += 1
            total_time2 += p[2]

    f.write("<!DOCTYPE html><html><header></header>\n<body>")
    f.write("<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" style=\"font-style:italic;width:854px\">")
    f.write("<thead><tr>")
    f.write("<td>Name</td><td>" + binary1 + "</td><td>" + binary2 + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    f.write("<td>Solved</td><td>" + repr(num_successes1) + "/" + repr(len(list)) + "</td><td>" + repr(num_successes2) + "/" + repr(len(list)) + "</td>")
    f.write("</tr>")
    f.write("<tr>")
    onesmaller = total_time1 < total_time2
    f.write("<td>Time </td><td>" + bold(repr(round(total_time1, 3)), onesmaller)+ "</td><td>" + bold(repr(round(total_time2, 3)), not onesmaller) + "</td>")
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


PATH_TO_SHARPSAT_BINARY1 = os.path.expandvars(os.path.expanduser(sys.argv[1]))
PATH_TO_SHARPSAT_BINARY2 = os.path.expandvars(os.path.expanduser(sys.argv[2]))

cnflist = read_filelist(os.path.expandvars(os.path.expanduser(sys.argv[3])))

compare_data = compare_on_list(PATH_TO_SHARPSAT_BINARY1, PATH_TO_SHARPSAT_BINARY2, cnflist)

print_results(PATH_TO_SHARPSAT_BINARY1, PATH_TO_SHARPSAT_BINARY2, compare_data)

store_results("results.html", PATH_TO_SHARPSAT_BINARY1, PATH_TO_SHARPSAT_BINARY2, compare_data)