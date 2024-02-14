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



/***
 * 
 *  Modified from detect(void); (src/plat/linux/detect.c, master branch)
 * 
 * 
*/
void detect_distro_linux(void){
   /* if distro_str was NOT set by the -D flag */
  if (STREQ(distro_str, "Unknown")) {
    FILE *distro_file;

    char distro_name_str[MAX_STRLEN];

    if (FILE_EXISTS("/system/bin/getprop")) {
      safe_strncpy(distro_str, "Android", MAX_STRLEN);
      safe_strncpy(host_color, TLGN, MAX_STRLEN);
    } else {
      bool detected = false;

      /* Bad solution, as /etc/issue contains junk on some distros */
      distro_file = fopen("/etc/issue", "r");

      if (distro_file != NULL) {
        /* get the first 4 chars, that's all we need */
        fscanf(distro_file, "%4s", distro_name_str);
        fclose(distro_file);

        if (STREQ(distro_name_str, "Kali")) {
          safe_strncpy(distro_str, "Kali Linux", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TLBL, MAX_STRLEN);
        } else if (STREQ(distro_name_str, "Back")) {
          safe_strncpy(distro_str, "Backtrack Linux", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TLRD, MAX_STRLEN);
        } else if (STREQ(distro_name_str, "Crun")) {
          safe_strncpy(distro_str, "CrunchBang", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TDGY, MAX_STRLEN);
        } else if (STREQ(distro_name_str, "LMDE")) {
          safe_strncpy(distro_str, "LMDE", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TLGN, MAX_STRLEN);
        } else if (STREQ(distro_name_str, "Debi") || STREQ(distro_name_str, "Rasp")) {
          safe_strncpy(distro_str, "Debian", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TLRD, MAX_STRLEN);
        } else if (STREQ(distro_name_str, "neon")) {
          safe_strncpy(distro_str, "KDE neon", MAX_STRLEN);
          detected = true;
          safe_strncpy(host_color, TLRD, MAX_STRLEN);
        }
      }
      
      if (!detected) {
        if (FILE_EXISTS("/etc/redhat-release")) {
          safe_strncpy(distro_str, "Red Hat Linux", MAX_STRLEN);
          safe_strncpy(host_color, TLRD, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/fedora-release")) {
          safe_strncpy(distro_str, "Fedora", MAX_STRLEN);
          safe_strncpy(host_color, TLBL, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/SuSE-release")) {
          safe_strncpy(distro_str, "OpenSUSE", MAX_STRLEN);
          safe_strncpy(host_color, TLGN, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/arch-release")) {
          safe_strncpy(distro_str, "Arch Linux", MAX_STRLEN);
          safe_strncpy(host_color, TLCY, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/gentoo-release")) {
          safe_strncpy(distro_str, "Gentoo", MAX_STRLEN);
          safe_strncpy(host_color, TLPR, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/angstrom-version")) {
          safe_strncpy(distro_str, "Angstrom", MAX_STRLEN);
          safe_strncpy(host_color, TNRM, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/manjaro-release")) {
          safe_strncpy(distro_str, "Manjaro", MAX_STRLEN);
          safe_strncpy(host_color, TLGN, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/slackware-release")) {
          safe_strncpy(distro_str, "Slackware", MAX_STRLEN);
          safe_strncpy(host_color, TLBL, MAX_STRLEN);
        } else if (FILE_EXISTS("/etc/lsb-release")) {
          distro_file = fopen("/etc/lsb-release", "r");
          fscanf(distro_file, "%s ", distro_name_str);
          fclose(distro_file);

          snprintf(distro_str, MAX_STRLEN, "%s", distro_name_str + 11);
          safe_strncpy(host_color, TLRD, MAX_STRLEN);

          if (STREQ(distro_str, "neon")) {
            safe_strncpy(distro_str, "KDE neon", MAX_STRLEN);
            detected = true;
            safe_strncpy(host_color, TLGN, MAX_STRLEN);
          return;}
        } else if (FILE_EXISTS("/etc/os-release")) {
          /*
            TODO: Parse NAME or PRETTY_NAME from os-release
            Until then, spit out an error message.
          */
          if (error)
            ERR_REPORT("Failed to detect a Linux distro (1).");
        } else {
          safe_strncpy(distro_str, "Linux", MAX_STRLEN);
          safe_strncpy(host_color, TLGY, MAX_STRLEN);

          if (error) {
            ERR_REPORT("Failed to detect a Linux distro (2).");
          }
        }
      }
    }
  }

  return;
}

void detect_cpu_linux(void) {
  FILE *cpu_file;
  char cpuinfo_buf[MAX_STRLEN];
  char *cpuinfo_line;
  int end;

  if ((cpu_file = fopen("/proc/cpuinfo", "r"))) {
    /* read past the first 4 lines (#5 is model name) */
    for (int i = 0; i < 5; i++) {
      if (!(fgets(cpuinfo_buf, MAX_STRLEN, cpu_file))) {
        ERR_REPORT("Fatal error while reading /proc/cpuinfo");
        return;
      }
    }

    /* fail to match a colon. this should never happen, but check anyways */
    if (!(cpuinfo_line = strchr(cpuinfo_buf, ':'))) {
      ERR_REPORT("Fatal error matching in /proc/cpuinfo");
      return;
    }

    cpuinfo_line += 2;
    end = strlen(cpuinfo_line);

    if (cpuinfo_line[end - 1] == '\n')
      cpuinfo_line[end - 1] = '\0';

    if (STREQ(cpuinfo_line, "ARMv6-compatible processor rev 7 (v6l)")) {
      safe_strncpy(cpu_str, "BCM2708 (Raspberry Pi)", MAX_STRLEN);
    } else {
      safe_strncpy(cpu_str, cpuinfo_line, MAX_STRLEN);
    }
  } else if (error) {
    ERR_REPORT("Failed to open /proc/cpuinfo. Ancient Linux kernel?");
  }

  return;
}
