/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* ints and chars */
#include <stdint.h>
#include <inttypes.h>
#include <uchar.h>

/* gnu stuff */
#include <iconv.h>
#include <regex.h>

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dlfcn.h> /* for loading native functions */
#include <libc/nt/windows.h>
#include <libc/nt/winsock.h>

/* program includes */
#include "../detect.h"
#include "../arrays.h"
#include "../colors.h"
#include "../misc.h"
#include "../disp.h"
#include "../util.h"
#include "../error_flag.h"
#include "detect_plat.h"

void detect_distro(void) {
  struct utsname kern_info;

  if (!(uname(&kern_info))) {
    if (STREQ(kern_info.sysname, "Linux")) {
      detect_distro_linux();
      return;
    } else if (STREQ(kern_info.sysname, "Windows")) {
      detect_distro_windows();
      return;
    } else if (STREQ(kern_info.sysname, "Darwin")) {
      // detect_distro_darwin();
      return;
    }

  } else {
    /*show errors?*/
  }
}
void detect_host(void) {
  char *given_user = "Unknown";
  char given_host[MAX_STRLEN] = "Unknown";
  struct utsname host_info;

  if (!uname(&host_info)) {
    if (STREQ(host_info.sysname, "Windows")) {

      given_user = getlogin();
      gethostname(given_host, MAX_STRLEN);

      snprintf(host_str, MAX_STRLEN, "%s%s%s%s@%s%s%s%s", host_color, given_user, TNRM, TWHT, TNRM,
               host_color, given_host, TNRM);
      return;
    } else {
      /* posix */
      given_user = getlogin();
      safe_strncpy(given_host, host_info.nodename, MAX_STRLEN);

      snprintf(host_str, MAX_STRLEN, "%s%s%s%s@%s%s%s%s", host_color, given_user, TNRM, TWHT, TNRM,
               host_color, given_host, TNRM);

      return;
    }
  }
};
void detect_kernel(void) {
  struct utsname kern_info;

  if (!(uname(&kern_info))) {
    snprintf(kernel_str, MAX_STRLEN, "%s %s %s", kern_info.sysname, kern_info.release,
             kern_info.machine);
  } else if (error) {
    ERR_REPORT("Could not detect kernel information.");
  }

  return;
}
void detect_uptime(void) {
  unsigned int secs = 0;
  unsigned int mins = 0;
  unsigned int hrs = 0;
  unsigned int days = 0;
  struct sysinfo si_upt;

  if (!(sysinfo(&si_upt))) {
    split_uptime(si_upt.uptime, &secs, &mins, &hrs, &days);

    if (days > 0)
      snprintf(uptime_str, MAX_STRLEN, "%dd %dh %dm %ds", days, hrs, mins, secs);
    else
      snprintf(uptime_str, MAX_STRLEN, "%dh %dm %ds", hrs, mins, secs);
  } else {
    ERR_REPORT("Could not detect system uptime.");
  }

  return;
}
void detect_pkgs(void){};
void detect_cpu(void) {
  struct utsname kern_info;
  if (!(uname(&kern_info))) {
    if (!(uname(&kern_info))) {
      if (STREQ(kern_info.sysname, "Linux")) {
        detect_cpu_linux();
        return;
      } else if (STREQ(kern_info.sysname, "Windows")) {
        detect_cpu_windows();
        return;
      } else if (STREQ(kern_info.sysname, "Darwin")) {
        detect_cpu_darwin();
        return;
      }
    }
  }
};
void detect_gpu(void) {
  struct utsname kern_info;
  if (!(uname(&kern_info))) {
    if (!(uname(&kern_info))) {
      if (STREQ(kern_info.sysname, "Linux")) {
        detect_gpu_linux();
        return;
      } else if (STREQ(kern_info.sysname, "Windows")) {
        detect_gpu_windows();
        return;
      } else if (STREQ(kern_info.sysname, "Darwin")) {
        detect_gpu_darwin();
        return;
      }
    }
  }
};
void detect_disk(void){};
void detect_mem(void) {
  struct sysinfo si;
  if (!sysinfo(&si)) {
    uint64_t total_mem_mib = si.totalram * si.mem_unit / (1024 * 1024);
    uint64_t free_mem_mib = si.freeram * si.mem_unit / (1024 * 1024);
    uint64_t used_mem_mib = total_mem_mib - free_mem_mib;
    uint64_t mem_pct = total_mem_mib > 0 ? (used_mem_mib * 100 / total_mem_mib) : 0;
    snprintf(mem_str, MAX_STRLEN, "%" PRId64 "%s / %" PRId64 "%s (%" PRId64 "%%)", used_mem_mib,
             "MiB", total_mem_mib, "MiB", mem_pct);
  }
};
void detect_shell(void){};
void detect_res(void){};
void detect_de(void){};
void detect_wm(void){};
void detect_wm_theme(void){};
void detect_gtk(void){};