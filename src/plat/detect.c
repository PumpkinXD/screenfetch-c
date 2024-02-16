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
      // detect_distro_darwin();
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

    // given_user = malloc(sizeof(char) * MAX_STRLEN);
    // if (!given_user) {
    //   ERR_REPORT("Memory allocation failed in detect_host.");
    //   exit(1);
    // }


    // void *Advapi32_dll=cosmo_dlopen("Advapi32.dll", RTLD_LAZY);
    // if (!Advapi32_dll)
    // {
    //   ERR_REPORT("Failed to load Advapi32.dll in detect_host");
    //   exit(1);
    // }
    // typedef uint32_t(* __attribute__((__ms_abi__))fnGetUserNameW)(char16_t* lpBuffer, uint32_t* pcbBuffer);
    // typedef uint32_t(* __attribute__((__ms_abi__))fnGetUserNameA)(char* lpBuffer, uint32_t* pcbBuffer);
    // fnGetUserNameW pGetUserNameW=cosmo_dlsym(Advapi32_dll,"GetUserNameW");
    // fnGetUserNameA pGetUserNameA=cosmo_dlsym(Advapi32_dll,"GetUserNameA");
    // if (!pGetUserNameA)
    // {
    //   ERR_REPORT("Failed to load function \"GetUserNameA()\" in detect_host");
    //   exit(1);
    // }

    given_user=getlogin();
    gethostname(given_host, MAX_STRLEN);
    //iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
    // iconv(cd,&given_user_u16,&inbytesleft,outbuf,outbytesleft);//
    
    snprintf(host_str, MAX_STRLEN, "%s%s%s%s@%s%s%s%s", host_color, given_user, TNRM, TWHT, TNRM,
             host_color, given_host, TNRM);

    // free(given_user);
    //iconv_close(cd);
    // cosmo_dlclose(Advapi32_dll);
    return;
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
void detect_cpu(void){
  struct utsname kern_info;
  if (!(uname(&kern_info))) {
      if (!(uname(&kern_info))) {
    if (STREQ(kern_info.sysname,"Linux"))
    {
      detect_cpu_linux();
      return;
    }else if (STREQ(kern_info.sysname,"Windows"))
    {
      detect_cpu_windows();
      return;
    }else if (STREQ(kern_info.sysname,"Darwin"))
    {
      // detect_cpu_darwin();
      return;
    }
   }
  }
};
void detect_gpu(void){
  struct utsname kern_info;
  if (!(uname(&kern_info))) {
      if (!(uname(&kern_info))) {
    if (STREQ(kern_info.sysname,"Linux"))
    {
      detect_gpu_linux();
      return;
    }else if (STREQ(kern_info.sysname,"Windows"))
    {
      detect_gpu_windows();
      return;
    }else if (STREQ(kern_info.sysname,"Darwin"))
    {
      // detect_cpu_darwin();
      return;
    }
   }
  }
};
void detect_disk(void){};
void detect_mem(void){};
void detect_shell(void){};
void detect_res(void){};
void detect_de(void){};
void detect_wm(void){};
void detect_wm_theme(void){};
void detect_gtk(void){};