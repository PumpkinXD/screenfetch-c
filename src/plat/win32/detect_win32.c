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
#include <libc/nt/registry.h>


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
      case 10://TODO:add win11
/**        if (build>=22000)
        {//not quite reliable since win10 insider uses 21000
          safe_strncpy(distro_str, "Microsoft Windows 11", MAX_STRLEN);
          break;
        }*/
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
void detect_cpu_windows(void) {

    const int key_read=0x00020019;;
    int64_t hkey;
    uint32_t str_size=MAX_STRLEN;
    char16_t buffer[MAX_STRLEN];
    
    
    // RegOpenKey(kNtHkeyLocalMachine, u"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hkey);
    RegOpenKeyEx(kNtHkeyLocalMachine,u"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,key_read,&hkey);

    RegQueryValueEx(hkey,u"ProcessorNameString",0,NULL,(char*)buffer,&str_size);

    // //utf16le to utf8
    iconv_t cd = iconv_open("UTF-8", "UTF-16LE");  
    if (cd == (iconv_t)-1) {  
        perror("iconv_open");  
        exit(1);
    }  
    size_t utf8_size = str_size * 6 + 1; 
    size_t utf16_size=str_size;
    char *utf8_buffer = (char *)malloc(utf8_size);  
        if (utf8_buffer == NULL) {  
        perror("malloc");  
        iconv_close(cd);  
        exit(1);  
    }  
    char *inptr = (char *)buffer;  
    char *outptr = utf8_buffer;  
    memset(utf8_buffer, 0, utf8_size);
    if (iconv(cd, &inptr, &utf16_size, &outptr, &utf8_size) == (size_t)-1) {  
        perror("iconv");  
        free(utf8_buffer);  
        iconv_close(cd);  
        exit(1);  
    }  
    safe_strncpy(cpu_str,utf8_buffer,MAX_STRLEN);//or strlen(outbuf)+1?

    free(utf8_buffer);
    iconv_close(cd);
    RegCloseKey(hkey);
    return;
}