//#include "libc/alg/alg.h"
//#include "libc/rand/rand.h"
//#include "libc/isystem/stdint.h"
//#include "libc/calls/calls.h"
//#include "libc/stdio/stdio.h"
//#include "net/http/escape.h"
#include "base64.h"

// Currently free is not called anywhere because allocations are small and
// the process is short-lived. (This process is a bump allocator(!))

#define MAX_IO_BYTES 1048576

// Options:
char* PREFIX_ENV_VAR_NAME = "SINGREQRUN2_PREFIX";
char* SYNC_ENV_VAR_NAME = "SINGREQRUN2_SYNC_RPC";
char* DEFAULT_PREFIX = "/var/run/req_run/";

// Constants:
char* SCRIPT_SHEBAG = "#!/bin/bash\n";
unsigned int SLEEP_TIMES[] = {10000, 10000, 30000, 50000, 100000, 200000, 500000};
size_t NUM_SLEEP_TIMES = 7;

void get_msgid(char *dest) {
  static char randbuf[16];
  long start = rand64();
  long end = rand64();
  memcpy(randbuf, &start, 8);
  memcpy(&randbuf[8], &end, 8);
  int ret = base64_encode(dest, 23, randbuf, 16);
}

int abort_write_error(char *detail) {
  fprintf(stderr, "Error writing to req_run: %s\n", detail);
  return -125;
}

int file_to_file(char* in_path, int out_fd) {
  int in_fd = open(in_path, O_RDONLY);
  if (in_fd < 0) {
    return abort_write_error(xstrcat("could not open ", in_path));
  }
  while (copyfd(in_fd, NULL, out_fd, NULL, 1024 * 64, 0) > 0);
  close(in_fd);
  return 0;
}

int pipe_to_pipe(int in_fd, int out_fd, bool *activity, bool *live) {
  while (1) {
    int ret = splice(in_fd, NULL, out_fd, NULL, MAX_IO_BYTES, SPLICE_F_NONBLOCK);
    if (ret == 0) {
      *live = false;
      return 0;
    } else if (ret == -1) {
      if (errno == EAGAIN) {
        return 0;
      } else {
        fprintf(stderr, "Error splicing from %d to %d, errno: %d\n", in_fd, out_fd, errno);
        return -124;
      }
    } else {
      *activity = true;
    }
  }
}

void nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int guard(int ret, char * err) {
  if (ret == -1) {
    perror(err);
    exit(1);
  }
  return ret;
}

int main(int argc, char *argv[]) {
  // -1. Check if we're just warming up the executable
  char* noop = getenv("SRR_CLIENT_NOOP");
  if (noop != NULL) {
    return 0;
  }

  // 0. Read configuration variables
  char* prefix = getenv(PREFIX_ENV_VAR_NAME);
  if (prefix == NULL || prefix[0] == '\0') {
    prefix = DEFAULT_PREFIX;
  }

  bool sync_rpc = getenv(SYNC_ENV_VAR_NAME) != NULL;

  // 1. Get filename
  // 128 / 6 = 22 base64 characters + 1 '\0'
  static char iden[24];
  get_msgid(iden);

  // 2. Write cmd file
  char* prefix_iden = xstrcat(prefix, iden);
  char* cmd_path = xstrcat(prefix_iden, ".cmd");
  // currently system call because it's easier
  int cmd_fd = creat(cmd_path, 0755); 
  if (cmd_fd < 0) {
    return abort_write_error(xstrcat("could not create ", cmd_path));
  }
  write(cmd_fd, SCRIPT_SHEBAG, strlen(SCRIPT_SHEBAG));
  char* cwd = getcwd(NULL, NULL);
  write(cmd_fd, "cd ", 3);
  write(cmd_fd, cwd, strlen(cwd));
  write(cmd_fd, " && ", 4);
  for (int i = 0; i < argc; i++) {
    // Idea from Python shlex.quote(...):
    // > use single quotes, and put single quotes into double quotes
    // > the string $'b is then quoted as '$'"'"'b'
    write(cmd_fd, "\'", 1);
    char* quoted = replacestr(argv[i], "'", "'\"'\"'");
    write(cmd_fd, quoted, strlen(quoted));
    write(cmd_fd, "\' ", 2);
  }
  close(cmd_fd);

  // 3. Put request in req file
  char* reqs_path = xstrcat(prefix, "reqs");
  char* code_path = xstrcat(prefix_iden, ".code");
  char* stdin_path = xstrcat(prefix_iden, ".stdin");
  char* stdout_path = xstrcat(prefix_iden, ".stdout");
  char* stderr_path = xstrcat(prefix_iden, ".stderr");

  if (!sync_rpc) {
    guard(mkfifo(stdin_path, 0777), "Could not make stdin pipe");
    guard(mkfifo(stdout_path, 0777), "Could not make stdout pipe");
    guard(mkfifo(stderr_path, 0777), "Could not make stderr pipe");
  }

  FILE* req_run_f = fopen(reqs_path, "a");
  if (req_run_f < 0) {
    return abort_write_error(xstrcat("could not open ", reqs_path));
  }
  fputs(iden, req_run_f);
  fputc('\n', req_run_f);
  fclose(req_run_f);

  if (sync_rpc) {
    // 4. Wait for exit code file to appear
    unsigned int sleep_time_idx = 0;
    while (1) {
      usleep(SLEEP_TIMES[sleep_time_idx]);
      int accessible = access(code_path, F_OK);
      if (accessible == 0) {
        break;
      }
      if (sleep_time_idx < NUM_SLEEP_TIMES - 1) {
        sleep_time_idx++;
      }
    }

    // 5. Write output files to output streams
    int retcode = file_to_file(stdout_path, 1);
    if (retcode != 0) return retcode;
    retcode = file_to_file(stderr_path, 2);
    if (retcode != 0) return retcode;
  } else {
    int sleep_time_idx = -1;
    nonblock(STDIN_FILENO);
    nonblock(STDOUT_FILENO);
    nonblock(STDERR_FILENO);
    int in_fd = open(stdin_path, O_WRONLY, O_NONBLOCK);
    if (in_fd == -1) {
      fprintf(stderr, "Error opening %s errno: %d\n", stdin_path, errno);
      return -125;
    }
    int out_fd = open(stdout_path, O_RDONLY, O_NONBLOCK);
    if (out_fd == -1) {
      fprintf(stderr, "Error opening %s errno: %d\n", stdout_path, errno);
      return -125;
    }
    int err_fd = open(stderr_path, O_RDONLY, O_NONBLOCK);
    if (err_fd == -1) {
      fprintf(stderr, "Error opening %s errno: %d\n", stderr_path, errno);
      return -125;
    }
    bool in_live = true;
    bool out_live = true;
    bool err_live = true;
    int retcode;

    while (1) {
      if (sleep_time_idx >= 0) {
        usleep(SLEEP_TIMES[sleep_time_idx]);
      }
      bool activity = false;
      if (in_live) {
        retcode = pipe_to_pipe(STDIN_FILENO, in_fd, &activity, &in_live);
        if (retcode != 0) return retcode;
      }
      if (out_live) {
        retcode = pipe_to_pipe(out_fd, STDOUT_FILENO, &activity, &out_live);
        if (retcode != 0) return retcode;
      }
      if (err_live) {
        retcode = pipe_to_pipe(err_fd, STDERR_FILENO, &activity, &err_live);
        if (retcode != 0) return retcode;
      }
      if (!out_live && !err_live) {
        int accessible = access(code_path, F_OK);
        if (accessible == 0) {
          close(in_fd);
          close(out_fd);
          close(err_fd);
          break;
        }
      }
      if (activity) {
        sleep_time_idx = -1;
      } else if (sleep_time_idx < NUM_SLEEP_TIMES - 1) {
        sleep_time_idx++;
      }
    }
  }
  // 6. Parse exit code file and return
  FILE* code_f = fopen(code_path, "r");
  if (code_f < 0) {
    return abort_write_error(xstrcat("could not open ", code_path));
  }
  static char exitcode[32];
  fgets(exitcode, 32, code_f);
  fclose(code_f);
  return atoi(exitcode);
}
