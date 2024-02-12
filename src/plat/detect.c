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

#include <regex.h>
 

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dlfcn.h> /* for loading native functions */
#include <libc/nt/windows.h>



/* program includes */
#include "../detect.h"
#include "../arrays.h"
#include "../colors.h"
#include "../misc.h"
#include "../disp.h"
#include "../util.h"
#include "../error_flag.h"



static void detect_distro_linux(void);
static void detect_distro_bsd(void){};//*bsd
static void detect_distro_darwin(void){};
static void detect_distro_windows(void);



void detect_distro(void){
  struct utsname kern_info;

  if (!(uname(&kern_info))) {
    if (STREQ(kern_info.sysname,"Linux"))
    {
      detect_distro_linux();
      return;
    }else if (STREQ(kern_info.sysname,"Windows"))
    {
      detect_distro_windows();
      return;
    }else if (STREQ(kern_info.sysname,"Darwin"))
    {
      detect_distro_darwin();
      return;
    }
    
    
  }
  else
  {
    /*show errors?*/
  }
}
void detect_host(void){
  char *given_user = "Unknown";
  char given_host[MAX_STRLEN] = "Unknown";
  struct utsname host_info;
  uname(&host_info);


  if(STREQ(host_info.sysname,"Windows")){

  }else{
  /* posix */
  given_user = getlogin();
  safe_strncpy(given_host, host_info.nodename, MAX_STRLEN);

  snprintf(host_str, MAX_STRLEN, "%s%s%s%s@%s%s%s%s", host_color, given_user, TNRM, TWHT, TNRM,
           host_color, given_host, TNRM);

  return;
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
void detect_cpu(void){};
void detect_gpu(void){};
void detect_disk(void){};
void detect_mem(void){};
void detect_shell(void){};
void detect_res(void){};
void detect_de(void){};
void detect_wm(void){};
void detect_wm_theme(void){};
void detect_gtk(void){};



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
            safe_strncpy(host_color, TLRD, MAX_STRLEN);
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


/***
 * 
 *  Modified from detect(void); (src/plat/win32/detect.c, master branch)
 * 
 * 
*/
void detect_distro_windows(void){
  void *ntdll=cosmo_dlopen("ntdll.dll", RTLD_LAZY);
  if(!ntdll){
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load ntdll.dll.");
    exit(1);
  }

  typedef struct _OSVERSIONINFOW {
    uint32_t dwOSVersionInfoSize;//ULONG
    uint32_t dwMajorVersion;//ULONG
    uint32_t dwMinorVersion;//ULONG
    uint32_t dwBuildNumber;//ULONG
    uint32_t dwPlatformId;//ULONG
    char16_t szCSDVersion[128];//win32 ver wchar_t
  } OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
  typedef int32_t(* __attribute__((__ms_abi__))fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
  fnRtlGetVersion pRtlGetVersion=cosmo_dlsym(ntdll,"RtlGetVersion");

  if(!pRtlGetVersion){
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load RtlGetVersion.");
    exit(1);
  }

  RTL_OSVERSIONINFOW VersionInformation = { 0 };
  VersionInformation.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
  int32_t ntStatus = pRtlGetVersion(&VersionInformation);
  if (ntStatus)
  {
    exit(1);
  }
      uint32_t major = VersionInformation.dwMajorVersion;
      uint32_t minor = VersionInformation.dwMinorVersion;
      uint32_t build = VersionInformation.dwBuildNumber;

      if (STREQ(distro_str, "Unknown")) {
      switch (major) {
      case 10:
        safe_strncpy(distro_str, "Microsoft Windows 10", MAX_STRLEN);
        break;
      case 6:
        switch (minor) {
        case 3:
          safe_strncpy(distro_str, "Microsoft Windows 8.1", MAX_STRLEN);
          break;
        case 2:
          safe_strncpy(distro_str, "Microsoft Windows 8", MAX_STRLEN);
          break;
        case 1:
          safe_strncpy(distro_str, "Microsoft Windows 7", MAX_STRLEN);
          break;
        case 0:
          safe_strncpy(distro_str, "Microsoft Windows Vista", MAX_STRLEN);
          break;
        }
        break;
      case 5:
        switch (minor) {
        case 1:
          safe_strncpy(distro_str, "Microsoft Windows XP", MAX_STRLEN);
          break;
        case 0:
          safe_strncpy(distro_str, "Microsoft Windows 2000", MAX_STRLEN);
          break;
        }
        break;
      default:
        safe_strncpy(distro_str, "Microsoft Windows", MAX_STRLEN);
        break;
      }
    }

    // TODO: This should really go somewhere else.
    if (major == 10 || (major == 6 && (minor == 3 || minor == 2))) {
      safe_strncpy(host_color, TLBL, MAX_STRLEN);
    } else {
      safe_strncpy(host_color, TRED, MAX_STRLEN);
    }

    cosmo_dlclose(ntdll);

    return;


}