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

  size_t cmd_len = strlen(cmd) + strlen("command -v ")+1;
  char *cmd_line = malloc(cmd_len);
  if (cmd_line == NULL) {
    perror("malloc");
    return -1;
  }
  snprintf(cmd_line, cmd_len, "command -v %s", cmd);

  fp = popen(cmd_line, "r");
//   printf("%s\n",cmd_line);
  if (fp == NULL) {
    perror("popen");
    free(cmd_line);
    return -1;
  }
  free(cmd_line);

  status = pclose(fp);
  return (status == 0) ? 1 : 0;
}
// int has_command2(const char *cmd){

// }