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

/* GNU libs*/
#include <iconv.h>
#include <regex.h>

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>


/* program includes */
#include "../../detect.h"
#include "../../arrays.h"
#include "../../colors.h"
#include "../../misc.h"
#include "../../disp.h"
#include "../../util.h"
#include "../../error_flag.h"
#include "../detect_plat.h"

void detect_cpu_darwin_x86workaround(void){
    FILE *fp;
    char buffer[MAX_STRLEN];
    fp=popen("/usr/sbin/sysctl -n machdep.cpu.brand_string", "r");
    if (!fp) {
      perror("popen");
      return 1;
    }
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      safe_strncpy(cpu_str, buffer, MAX_STRLEN);
    }
    pclose(fp);
}
void detect_gpu_darwin_x86workaround(void) { // GPU: https://stackoverflow.com/a/20115806 
                                             // returns "Unknown" on vmware/vitualbox/hackintosh(?)
  FILE *fp;
  char command[] =
      "system_profiler SPDisplaysDataType | awk -F': ' '/Chipset Model/ {print $2}'";
  char buffer[MAX_STRLEN] = {0};

  fp = popen(command, "r");
  if (fp == NULL) {
    perror("popen");
    return 1;
  }

  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    safe_strncpy(gpu_str, buffer, MAX_STRLEN);
  } else {
    safe_strncpy(gpu_str, "Unknown", MAX_STRLEN);
  }


  pclose(fp);
}

// res: system_profiler SPDisplaysDataType | awk -F': ' '/Resolution/ {print $2}'