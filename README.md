# singreqrun

This repository contains code to help code running within Singularity container
execute commands on the host. It contains two parts, a Bash *server component*
which runs on the host and a Python *client component* which runs in the
container. Pull requests with clients for other languages are welcome!

The main application is currently to allow Python code within a container to
run SLURM commands on the host. The idea is that repository is customised by
including it as a git submodule. Usually this customisation code will include
some setup that arranges so that the req_run.py is monkeypatched in place of
the subprocess module 

The whole setup is made to make minimal requirements upon the host system.
After all, why are we insisting upon containerisation, if we're just going to
go and install a bunch of things directory on the host? In particular, only the
following are required:

 * Bash
 * Git
 * Singularity

## Protocol

The protocol uses the file system for communication. It is kept simple and
deliberately restricts itself to using only named pipes and files, rather than
using unix domain sockets, which would add additional requirements (netcat).

The client by creating a unique message id and writing the command to run to
a file message on this message id. The client then writes the id, followed by
a newline, to a named pipe: `/var/run/req_run/reqs`. For example:
`"dc5507a904e34bc6967036898c8066ec\n"`. The server then writes its response to
files based upon the message id. The client knows the server has finished
writing its response once it has written the exit code.

The named pipe has multiple writers. [This is safe since writes of up to 512
bytes are guaranteed to be atomic by
POSIX](https://unix.stackexchange.com/questions/68146/what-are-guarantees-for-concurrent-writes-into-a-named-pipe).

A full summary of the files involved are given in the following table:

| Path | Type | Direction | Purpose |
|---|---|---|---|
| /var/run/req_run/reqs | Named pipe | Client -> Server | Initiating a new request/response session |
| /var/run/req_run/$ID.cmd | Executable file | Client -> Server | Specifying the command to be run |
| /var/run/req_run/$ID.stdout | File | Server -> Client | Returning the process's STDOUT the client |
| /var/run/req_run/$ID.stderr | File | Server -> Client | Returning the process's STDERR the client |
| /var/run/req_run/$ID.code | File | Server -> Client | Returning the exit code to the client |

## Integrating into your application

TODO

## As used by...

 * [singslurm2](https://github.com/frankier/singslurm2)
