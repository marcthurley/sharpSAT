import subprocess
from helpers.processcall import run_with_timeout

__author__ = 'thurley'




def run_sharpsat_get_number(binary, arg, timeout):
    try:
        print("Running " + binary + " " + arg)
        output = run_with_timeout(timeout, [binary, arg], stderr=subprocess.STDOUT)
        #print(output)
    except OSError as E:
        print("OS Error")
        print(E.message)
        #print(output)
        return "0"
    i = output.find("# solutions")
    j = output.find("# END")
    try:
        return output[i:j].split("\n")[1]
    except IndexError as e:
        print(e)
        print(output)