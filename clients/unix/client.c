//#include "libc/alg/alg.h"
//#include "libc/rand/rand.h"
//#include "libc/isystem/stdint.h"
//#include "libc/calls/calls.h"
//#include "libc/stdio/stdio.h"
//#include "net/http/escape.h"
#include "base64.h"

// Currently free is not called anywhere because allocations are small and
// the process is short-lived. (This process is a bump allocator(!))

// Options:
char* PREFIX_ENV_VAR_NAME = "SINGREQRUN2_PREFIX";
char* DEFAULT_PREFIX = "/var/run/req_run/";

// Constants:
char* SCRIPT_SHEBAG = "#!/bin/bash\n";
unsigned int SLEEP_TIMES[] = {10000, 10000, 30000, 50000, 100000, 20000, 50000};
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

int main(int argc, char *argv[]) {
  // 0. Read configuration variables
  char* prefix = getenv(PREFIX_ENV_VAR_NAME);
  if (prefix == NULL || prefix[0] == '\0') {
    prefix = DEFAULT_PREFIX;
  }

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
  FILE* req_run_f = fopen(reqs_path, "a");
  if (req_run_f < 0) {
    return abort_write_error(xstrcat("could not open ", reqs_path));
  }
  fputs(iden, req_run_f);
  fputc('\n', req_run_f);
  fclose(req_run_f);

  // 4. Wait for exit code file to appear
  char* code_path = xstrcat(prefix_iden, ".code");
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
  int retcode = file_to_file(xstrcat(prefix_iden, ".stdout"), 1);
  if (retcode != 0) return retcode;
  retcode = file_to_file(xstrcat(prefix_iden, ".stderr"), 2);
  if (retcode != 0) return retcode;
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
