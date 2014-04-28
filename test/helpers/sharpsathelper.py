import subprocess
import time
from helpers.processcall import run_with_timeout

__author__ = 'thurley'



def run_all_on_list(timeout, cnf_list, binaries):
    list = []
    for p in cnf_list:
        instance_entry = [p]
        for binary in binaries:
            time = take_time(binary, p, timeout)
            instance_entry.append(time)
        list.append(instance_entry)
    return list



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

    stringtime = '--' if time_taken >= timeout else repr(round(time_taken, 3))

    print("Time " + '\t' + stringtime + "s")
    return time_taken