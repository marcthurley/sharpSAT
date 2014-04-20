#!/usr/bin/python
import csv
import subprocess
import sys
from pycolorterm import pycolorterm
from processcall import run_with_timeout

SECONDS = 10

__author__ = 'thurley'
#
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def printPassed(s):
    print(bcolors.OKGREEN + "PASSED " + bcolors.ENDC + s)
def printWarn(s):
    print(bcolors.WARNING + "WARN " + bcolors.ENDC + s)
def printFAIL(s):
    print(bcolors.FAIL + "FAIL " + bcolors.ENDC + s)


PATH_TO_SHARPSAT_BINARY = sys.argv[1]

#CSV file containing file, modelcount
CHECKFILE = sys.argv[2]

print(sys.argv[1], CHECKFILE)

def read_modelcounts(filename):
    list = []
    with open(filename, "r") as countscsvfile:
        countreader = csv.reader(countscsvfile)
        for row in countreader:
            list.append([row[0].rstrip(), row[1].strip()])
    return list




def run_sharpsat_get_number(binary, arg):
    try:
        print("Running " + binary + " " + arg)
        output = run_with_timeout(SECONDS, [binary, arg], stderr=subprocess.STDOUT)
        #print(output)
    except OSError as E:
        print("OS Error")
        print(E.message)
        #print(output)
        return "0"
    i = output.find("# solutions")
    j = output.find("# END")
    return output[i:j].split("\n")[1]

def run_on_countlist(binary, countlist):
    numfail = 0
    for p in countlist:
        try:
            number = run_sharpsat_get_number(binary, p[0])
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