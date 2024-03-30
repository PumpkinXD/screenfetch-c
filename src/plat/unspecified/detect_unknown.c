/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* ints and chars */
#include <stdint.h>
#include <uchar.h>

#include <iconv.h>
#include <regex.h>

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dlfcn.h> /* for loading native functions */

/* program includes */
#include "../../detect.h"
#include "../../arrays.h"
#include "../../colors.h"
#include "../../misc.h"
#include "../../disp.h"
#include "../../util.h"
#include "../../error_flag.h"
#include "../detect_plat.h"

int has_command(const char *cmd) {
  FILE *fp;
  int status;

  size_t cmd_len = strlen(cmd) + strlen("command -v > /dev/null 2>&1 ") + 1;
  char *cmd_line = malloc(cmd_len);
  if (cmd_line == NULL) {
    perror("malloc");
    return -1;
  }
  snprintf(cmd_line, cmd_len, "command -v %s > /dev/null 2>&1", cmd);

  fp = popen(cmd_line, "r");
  if (fp == NULL) {
    perror("popen");
    free(cmd_line);
    return -1;
  }
  free(cmd_line);

  status = pclose(fp);
  // printf("%d\n",status);
  return (status == 0) ? 1 : 0;
}
int has_command2(const char *cmd){
  FILE *fp;
  int status;

  size_t cmd_len = strlen(cmd) + strlen("type -p > /dev/null 2>&1 ") + 1;
  char *cmd_line = malloc(cmd_len);
  if (cmd_line == NULL) {
    perror("malloc");
    return -1;
  }
  snprintf(cmd_line, cmd_len, "type -p %s > /dev/null 2>&1", cmd);

  fp = popen(cmd_line, "r");
  if (fp == NULL) {
    perror("popen");
    free(cmd_line);
    return -1;
  }
  free(cmd_line);

  status = pclose(fp);
  return (status == 0) ? 1 : 0;
}

int has_command3(const char *cmd) {
  int status;
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    int fd = open("/dev/null", O_WRONLY);
    if (fd == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    char *argv[] = {"sh", "-c", cmd, NULL};
    execv("/bin/sh", argv);

    perror("execv");
    exit(EXIT_FAILURE);
  } else {
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
      return -1;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 1 : 0;
  }
}