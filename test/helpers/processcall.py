import os
import subprocess
import time
import signal

__author__ = 'thurley'


def wait_timeout(proc, seconds):
    """Wait for a process to finish, or raise exception after timeout"""
    start = time.time()
    end = start + seconds
    interval = 0.01

    while True:
        result = proc.poll()
        #print "waiting"
        if result is not None:
            return result
        if time.time() >= end:

            os.killpg(proc.pid, signal.SIGTERM)
            raise RuntimeError("Process timed out")
        time.sleep(interval)

def run_with_timeout(seconds, *popenargs, **kwargs):
    if 'stdout' in kwargs:
        raise ValueError('stdout argument not allowed, it will be overridden.')
    process = subprocess.Popen(stdout=subprocess.PIPE,
                        preexec_fn=os.setsid, *popenargs, **kwargs)
    retcode = wait_timeout(process, seconds)
    output, unused_err = process.communicate()

    if retcode:
        cmd = kwargs.get("args")
        if cmd is None:
            cmd = popenargs[0]
        raise subprocess.CalledProcessError(retcode, cmd, output=output)
    return output
