SLEEP_TIME = 0.2


def check_output(args):
    import os
    import time
    import uuid
    from subprocess import CalledProcessError
    iden = uuid.uuid4().hex
    cmd_path = "/var/run/req_run/{iden}.cmd"
    with open(os.open(cmd_path, os.O_CREAT | os.O_WRONLY, 0o755), "w") as cmd:
        cmd.write("#!/bin/bash\n")
        cmd.write(" ".join(args))
    with open("/var/run/req_run/reqs", "a") as req_run:
        req_run.write(iden + "\n")
    while not os.path.exists(f"/var/run/req_run/{iden}.code"):
        time.sleep(SLEEP_TIME)
    exit_code_str = ""
    while 1:
        with open(f"/var/run/req_run/{iden}.code") as code_f:
            exit_code_str = code_f.read().strip()
        if exit_code_str:
            break
        time.sleep(SLEEP_TIME)
    exit_code = int(exit_code_str)
    with open(f"/var/run/req_run/{iden}.stdout", "rb") as stdout_f:
        stdout = stdout_f.read()
    with open(f"/var/run/req_run/{iden}.stderr", "rb") as stderr_f:
        stderr = stderr_f.read()
    if exit_code:
        raise CalledProcessError(exit_code, args, stdout, stderr)
    return stdout
