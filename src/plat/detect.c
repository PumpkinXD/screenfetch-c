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
#include <libc/dce.h>
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

  if (IsLinux()) {
    detect_distro_linux();
    return;
  } else if (IsWindows()) {
    detect_distro_windows();
    return;
  } else if (IsXnuSilicon()) {
    /* code */
    return;
  } else if (IsXnu()) {
    detect_distro_darwin_x86workaround();
    return;
  } else if (IsBsd()) {
    /* code */
    return;
  } else {
    /* show errors? */
    /* code */
  }
}
void detect_host(void) {
  char *given_user = "Unknown";
  char given_host[MAX_STRLEN] = "Unknown";
  struct utsname host_info;

  if (!uname(&host_info)) {
    if (IsWindows()) {

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
  if (IsLinux()) {
    detect_cpu_linux();
    return;
  } else if (IsWindows()) {
    detect_cpu_windows();
    return;
  } else if (IsXnuSilicon()) {
    detect_cpu_darwin();
    return;
  } else if (IsXnu()) {
    detect_cpu_darwin_x86workaround();
    return;
  } else if (IsBsd()) {
    /* code */
    return;
  } else {
    /* show errors? */
    /* code */
  }
};
void detect_gpu(void) {
  if (IsLinux()) {
    detect_gpu_linux();
    return;
  } else if (IsWindows()) {
    detect_gpu_windows();
    return;
  } else if (IsXnuSilicon()) {
    detect_gpu_darwin();
    return;
  } else if (IsXnu()) {
    detect_gpu_darwin_x86workaround();
    return;
  } else if (IsBsd()) {
    /* code */
    return;
  } else {
    /* show errors? */
    /* code */
  } 
};
void detect_disk(void) {
  if (IsLinux()) {
    detect_disk_linux();
    return;
  } else if (IsWindows()) {
    detect_disk_windows();
    return;
  } else if (IsXnuSilicon()) {
    // detect_disk_darwin();
    return;
  } else if (IsXnu()) {
    // detect_disk_darwin_x86workaround();
    return;
  } else if (IsBsd()) {
    /* code */
    return;
  } else {
    /* show errors? */
    /* code */
  }
};
void detect_mem(void) {
  // kinda... inaccurate.. on macos(maybe including some linux/*nix distros???)
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
void detect_shell(void){
  if (!IsWindows())
  {
    FILE *shell_file;
    char *shell_name;
    char vers_str[MAX_STRLEN];

    if (!(shell_name = getenv("SHELL"))) {
      if (error)
        ERR_REPORT("Could not detect a shell - $SHELL not defined.");

      return;
    }

    if (STREQ(shell_name, "/bin/sh")) {
      safe_strncpy(shell_str, "POSIX sh", MAX_STRLEN);
    } else if (strstr(shell_name, "bash")) {
      shell_file = popen("bash --version | head -1", "r");
      fgets(vers_str, MAX_STRLEN, shell_file);
      snprintf(shell_str, MAX_STRLEN, "bash %.*s", 17, vers_str + 10);
      pclose(shell_file);
    } else if (strstr(shell_name, "zsh")) {
      shell_file = popen("zsh --version", "r");
      fgets(vers_str, MAX_STRLEN, shell_file);
      snprintf(shell_str, MAX_STRLEN, "zsh %.*s", 5, vers_str + 4);
      pclose(shell_file);
    } else if (strstr(shell_name, "csh")) {
      shell_file = popen("csh --version | head -1", "r");
      fgets(vers_str, MAX_STRLEN, shell_file);
      snprintf(shell_str, MAX_STRLEN, "csh %.*s", 7, vers_str + 5);
      pclose(shell_file);
    } else if (strstr(shell_name, "fish")) {
      shell_file = popen("fish --version", "r");
      fgets(vers_str, MAX_STRLEN, shell_file);
      snprintf(shell_str, MAX_STRLEN, "fish %.*s", 13, vers_str + 6);
      pclose(shell_file);
    } else if (strstr(shell_name, "dash") || strstr(shell_name, "ash") ||
               strstr(shell_name, "ksh")) {
      /* i don't have a version detection system for these, yet */
      safe_strncpy(shell_str, shell_name, MAX_STRLEN);
    }

    return;
  }else
  {
    detect_shell_windows();
  }
  
  
};
void detect_res(void) {
  printf("detect_res\n");
  if (IsLinux()) {
    // detect_res_linux();
    return;
  } else if (IsWindows()) {
    detect_res_windows();
    return;
  } else if (IsXnuSilicon()) {
    // detect_res_darwin();
    return;
  } else if (IsXnu()) {
    // detect_res_darwin_x86workaround();
    return;
  } else if (IsBsd()) {
    /* code */
    return;
  } else {
    /* show errors? */
    /* code */
  }
}
void detect_de(void) {
    if (IsLinux()) {
      detect_de_linux();
      return;
    } else if (IsWindows()) {
      // detect_de_windows();
      return;
    } else if (IsXnuSilicon()) {
      // detect_de_darwin();
      return;
    } else if (IsXnu()) {
      // detect_de_darwin_x86workaround();
      return;
    } else if (IsBsd()) {
      /* code */
      return;
    } else {
      /* show errors? */
      /* code */
    }
}
  void detect_wm(void){};
  void detect_wm_theme(void){};
  void detect_gtk(void){};