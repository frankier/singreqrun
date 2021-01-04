SLEEP_TIMES = [0.01, 0.01, 0.03, 0.05, 0.1, 0.2]


class Sleeper:
    def __init__(self):
        self.idx = 0

    def sleep(self):
        if self.idx >= len(SLEEP_TIMES):
            sleep_time = SLEEP_TIMES[-1]
        else:
            sleep_time = SLEEP_TIMES[self.idx]
            self.idx += 1
        time.sleep(sleep_time)


def check_output(args):
    import os
    import time
    import base64
    import random
    import shlex
    from subprocess import CalledProcessError
    rand_bytes = random.getrandbits(128).to_bytes(16, 'little')
    iden = base64.urlsafe_b64encode(rand_bytes).decode('utf-8')
    cmd_path = f"/var/run/req_run/{iden}.cmd"
    with open(os.open(cmd_path, os.O_CREAT | os.O_WRONLY, 0o755), "w") as cmd:
        cmd.write("#!/bin/bash\n")
        cmd.write(" ".join(shlex.quote(arg) for arg in args))
    with open("/var/run/req_run/reqs", "a") as req_run:
        req_run.write(iden + "\n")
    sleeper = Sleeper()
    while not os.path.exists(f"/var/run/req_run/{iden}.code"):
        sleeper.sleep()
    exit_code_str = ""
    sleeper = Sleeper()
    while 1:
        with open(f"/var/run/req_run/{iden}.code") as code_f:
            exit_code_str = code_f.read().strip()
        if exit_code_str:
            break
        sleeper.sleep()
    exit_code = int(exit_code_str)
    with open(f"/var/run/req_run/{iden}.stdout", "rb") as stdout_f:
        stdout = stdout_f.read()
    with open(f"/var/run/req_run/{iden}.stderr", "rb") as stderr_f:
        stderr = stderr_f.read()
    if exit_code:
        raise CalledProcessError(exit_code, args, stdout, stderr)
    return stdout
