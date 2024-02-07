/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>


#include <regex.h>
 


#include <sysinfo.h>
#include <utsname.h>
#include <runtime.h>
// #include <libc/calls/calls.h>
//#include "libc/nt/windows.h"


/* program includes */
#include "../detect.h"
#include "../arrays.h"
#include "../colors.h"
#include "../misc.h"
#include "../disp.h"
#include "../util.h"
#include "../error_flag.h"


// static struct utsname utsinst;

static void detect_distro_linux(void);
static void detect_distro_bsd(void);//*bsd
static void detect_distro_darwin(void);
static void detect_distro_windows(void);//pure windows,mingw, cygwin, msys, reactos or wine, etc.


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
    }else if (STREQ(kern_info.sysname,"Darwin"))
    {
      detect_distro_darwin();
    }
    
    
    

  }
  else
  {
    /* code */
  }
  

  // STREQ(distro_str, "Ubuntu")
//TODO:detect OS on runtime
}
void detect_host(void);
void detect_kernel(void) {
  struct utsname kern_info;

  if (!(uname(&kern_info))) {
    snprintf(kernel_str, MAX_STRLEN, "%s %s %s", kern_info.sysname, kern_info.release,
             kern_info.machine);
  } else if (error) {
    ERR_REPORT("Could not detect kernel information.");
    // safe_strncpy(kernel_str, "Unknown", MAX_STRLEN);
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
void detect_pkgs(void);
void detect_cpu(void);
void detect_gpu(void);
void detect_disk(void);
void detect_mem(void);
void detect_shell(void);
void detect_res(void);
void detect_de(void);
void detect_wm(void);
void detect_wm_theme(void);
void detect_gtk(void);