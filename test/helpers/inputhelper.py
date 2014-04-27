import os

__author__ = 'thurley'



def read_filelist(filename):
    list = []
    with open(filename, "r") as cnflistfile:
        for line in cnflistfile:
            cnfname = os.path.expandvars(os.path.expanduser(line))
            list.append(cnfname.rstrip())
    return list

