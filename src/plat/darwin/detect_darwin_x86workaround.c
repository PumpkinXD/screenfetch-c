/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
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

// generated by microsoft copilot
void detect_distro_darwin_x86workaround() {

  FILE *fp;
  char codename[MAX_STRLEN]={0};
  char productver[MAX_STRLEN]={0};
  char buildver[MAX_STRLEN]={0};


  fp = popen("/usr/bin/sw_vers -productVersion", "r");
  if (fp == NULL) {
    printf("Failed to run command\n");
    return ;
  }
  fgets(productver, sizeof(productver) - 1, fp);
  pclose(fp);


  fp = popen("/usr/bin/sw_vers -buildVersion", "r");
  if (fp == NULL) {
    printf("Failed to run command\n");
    return ;
  }
  fgets(buildver, sizeof(buildver) - 1, fp);
  pclose(fp);

  const char *lookfor = "SOFTWARE LICENSE AGREEMENT FOR ";
  fp = fopen("/System/Library/CoreServices/Setup "
             "Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf",
             "r");
  if (fp != NULL) {
    char buf[MAX_STRLEN];
    for (int i = 0; i < 50 && fgets(buf, sizeof(buf), fp); ++i) {
      char *p = strstr(buf, lookfor);
      if (p) {
        strncpy(codename, p + strlen(lookfor), sizeof(codename) - 1);
        codename[sizeof(codename) - 1] = '\0'; 
        break;
      }
    }
    fclose(fp);
  }
  //remove \n 
  productver[strcspn(productver, "\n")]=0;
  buildver[strcspn(buildver, "\n")] = 0;
  unsigned codename_lf_pos=strcspn(codename,"\n");
  if(isalpha(codename[codename_lf_pos-1])){
    codename[codename_lf_pos]=0;
  }else codename[codename_lf_pos-1]=0;

  // distro="$codename $osx_version $osx_build"
  snprintf(distro_str, MAX_STRLEN, "%s %s (%s)", codename, productver, buildver);
  safe_strncpy(host_color, TLBL, MAX_STRLEN);
}

void detect_cpu_darwin_x86workaround(void) {
  FILE *fp;
  char buffer[MAX_STRLEN];
  fp = popen("/usr/sbin/sysctl -n machdep.cpu.brand_string", "r");
  if (!fp) {
    perror("popen");
    return;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    buffer[strcspn(buffer, "\n")] = 0;
    safe_strncpy(cpu_str, buffer, MAX_STRLEN);
  }
  pclose(fp);
}
void detect_gpu_darwin_x86workaround(void) { // GPU: https://stackoverflow.com/a/20115806
                                             // returns "Unknown" on vmware/vitualbox/hackintosh(?)
  FILE *fp;
  char command[] = "system_profiler SPDisplaysDataType | awk -F': ' '/Chipset Model/ {print $2}'";
  char buffer[MAX_STRLEN] = {0};

  fp = popen(command, "r");
  if (fp == NULL) {
    perror("popen");
    return;
  }

  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    buffer[strcspn(buffer, "\n")] = 0;
    safe_strncpy(gpu_str, buffer, MAX_STRLEN);
  } else {
    safe_strncpy(gpu_str, "Unknown", MAX_STRLEN);
  }

  pclose(fp);
}

// res: system_profiler SPDisplaysDataType | awk -F': ' '/Resolution/ {print $2}'