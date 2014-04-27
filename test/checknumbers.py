#!/usr/bin/python
import csv
import os
import subprocess
import sys
from helpers.pycolortermhelp import bcolors
from helpers.sharpsathelper import run_sharpsat_get_number

SECONDS = 10

__author__ = 'thurley'
#

def printPassed(s):
    print(bcolors.OKGREEN + "PASSED " + bcolors.ENDC + s)
def printWarn(s):
    print(bcolors.WARNING + "WARN " + bcolors.ENDC + s)
def printFAIL(s):
    print(bcolors.FAIL + "FAIL " + bcolors.ENDC + s)

def read_modelcounts(filename):
    list = []
    with open(filename, "r") as countscsvfile:
        countreader = csv.reader(countscsvfile)
        for row in countreader:
            cnfname = os.path.expandvars(os.path.expanduser(row[0].rstrip()))
            list.append([cnfname, row[1].strip()])
    return list

def run_on_countlist(binary, countlist):
    numfail = 0
    for p in countlist:
        try:
            number = run_sharpsat_get_number(binary, p[0], SECONDS)
            if p[1] == number:
                printPassed(p[0] + "; " + p[1])
            else:
                numfail += 1
                printFAIL(p[0])
                print ("    expected # " + p[1])
                print ("    seen     # " + number)

        except subprocess.CalledProcessError as E:
            erroroutput = "sharpSAT exited with exit code" + str(E.returncode) + "\n" + E.output
    return numfail



PATH_TO_SHARPSAT_BINARY = os.path.expandvars(os.path.expanduser(sys.argv[1]))

#CSV file containing file, modelcount
CHECKFILE = os.path.expandvars(os.path.expanduser(sys.argv[2]))

print(sys.argv[1], CHECKFILE)

counts = read_modelcounts(CHECKFILE)
numfail = run_on_countlist(PATH_TO_SHARPSAT_BINARY, counts)

if numfail != 0:
    print (str(len(counts) - numfail) + " TESTS PASSED " + bcolors.FAIL + str(numfail) + " FAILED" + bcolors.ENDC)
else:
    print (bcolors.OKGREEN + "All " + str(len(counts)) + " PASSED " + bcolors.ENDC)
#for r in list:
#    print("aw " + r[0] + "; " + r[1])

#cmd = [sys.argv[1], sys.argv[2]]
#
#try:
#    output = run_with_timeout(10, cmd, stderr=subprocess.STDOUT)
#except subprocess.CalledProcessError as E:
#    erroroutput = "sharpSAT exited with exit code" + str(E.returncode) + "\n" + E.output
#
#if len(output) > 0:
#    i = output.find("# solutions")
#
#    j = output.find("# END")
#
#    number = output[i:j].split("\n")[1]
#    print output
#    print number