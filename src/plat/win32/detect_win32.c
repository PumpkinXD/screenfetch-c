/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* ints and chars */
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

/* GNU */
#include <iconv.h>
#include <regex.h>

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <dlfcn.h> /* for loading native functions */

/* cosmo libc WIN32*/
#include <libc/nt/windows.h>
#include <libc/nt/winsock.h>
#include <libc/nt/registry.h>
#include <libc/nt/runtime.h>
#include <libc/nt/process.h>
#include <libc/nt/struct/processentry32.h>
#include <libc/nt/struct/osversioninfo.h>
#include <libc/nt/systeminfo.h>

/* program includes */
#include "../../detect.h"
#include "../../arrays.h"
#include "../../colors.h"
#include "../../misc.h"
#include "../../disp.h"
#include "../../util.h"
#include "../../error_flag.h"
#include "../detect_plat.h"

typedef struct win32ProcessInfoSimp {
  uint32_t processID;
  uint32_t parentProcessID;
  char16_t processName[260];
} SimpWin32ProcessInfo;

static char *GetRegU8StrValue(int64_t predefined, const char16_t *pathkey,
                              const char16_t *key); // use free(1) to deallocate
static char *utf16_to_utf8(const char16_t *input);
static int isSpecialProcess(const char16_t *processName);
static SimpWin32ProcessInfo GetSimpWin32ProcessInfo(uint32_t pid);
static char *getParentShellProcessName();

/***
 *
 *  Modified from detect_distro(void); (src/plat/win32/detect.c, master branch)
 *
 *
 */
void detect_distro_windows(void) {

  // TODO:reactos, wine,mingw, etc.
  //@see https://reactos.org/forum/viewtopic.php?p=129190#p129190
  //@see https://www.winehq.org/pipermail/wine-devel/2008-September/069387.html
  //@see https://www.msys2.org/wiki/Porting/ (ENV?)

  void *ntdll = cosmo_dlopen("ntdll.dll", RTLD_LAZY);
  if (!ntdll) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load ntdll.dll.");
    exit(1);
  }

  typedef struct _OSVERSIONINFOW {
    uint32_t dwOSVersionInfoSize; // ULONG
    uint32_t dwMajorVersion;      // ULONG
    uint32_t dwMinorVersion;      // ULONG
    uint32_t dwBuildNumber;       // ULONG
    uint32_t dwPlatformId;        // ULONG
    char16_t szCSDVersion[128];   // win32 ver wchar_t
  } OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
  typedef struct NtOsVersionInfo OSVERSIONINFOWEX, *POSVERSIONINFOWEX;
  typedef int32_t (*__attribute__((__ms_abi__))
                   fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
  fnRtlGetVersion pRtlGetVersion = cosmo_dlsym(ntdll, "RtlGetVersion");

  if (!pRtlGetVersion) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load RtlGetVersion.");
    exit(1);
  }

  struct NtOsVersionInfo VersionInformation = {0};
  VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFOWEX);
  int32_t ntStatus = pRtlGetVersion((PRTL_OSVERSIONINFOW)&VersionInformation);
  if (ntStatus) {
    exit(1);
  }
  uint32_t major = VersionInformation.dwMajorVersion;
  uint32_t minor = VersionInformation.dwMinorVersion;
  uint32_t build = VersionInformation.dwBuildNumber;
  uint8_t product_type = VersionInformation.wProductType; /// VER_NT_WORKSTATION==0x0000001

  if (STREQ(distro_str, "Unknown")) {
    switch (major) {
    case 10:
      if (product_type != 1) {
        if (build > 17763) {
          safe_strncpy(distro_str, "Microsoft Windows Server 2022", MAX_STRLEN);
        } else if (build > 14393) {
          safe_strncpy(distro_str, "Microsoft Windows Server 2019", MAX_STRLEN);
        } else {
          safe_strncpy(distro_str, "Microsoft Windows Server 2016", MAX_STRLEN);
        }
      } else {
        if (build >= 22000) {
          safe_strncpy(distro_str, "Microsoft Windows 11", MAX_STRLEN);
        } else {
          safe_strncpy(distro_str, "Microsoft Windows 10", MAX_STRLEN);
        }
      }
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
        // unreachable??? (cosmo libc supports windows 8+)
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
void detect_pkgs_windows(void) {
  // winget (win10+), scoop,
}
void detect_cpu_windows(void) {

  const int key_read = 0x00020019;
  int64_t hkey;
  uint32_t str_size = MAX_STRLEN;
  char16_t buffer[MAX_STRLEN];

  // RegOpenKey(kNtHkeyLocalMachine, u"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hkey);
  RegOpenKeyEx(kNtHkeyLocalMachine, u"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
               key_read, &hkey);

  RegQueryValueEx(hkey, u"ProcessorNameString", 0, NULL, (char *)buffer, &str_size);

  // //utf16le to utf8
  iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd == (iconv_t)-1) {
    perror("iconv_open");
    exit(1);
  }
  size_t utf8_size = str_size * 6 + 1;
  size_t utf16_size = str_size;
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
  safe_strncpy(cpu_str, utf8_buffer, MAX_STRLEN); // or strlen(utf8_buffer)+1?

  free(utf8_buffer);
  iconv_close(cd);
  RegCloseKey(hkey);
  return;
}

void detect_gpu_windows(void) {

  typedef struct _DISPLAY_DEVICEW {
    uint32_t cb;                // DWORD
    char16_t DeviceName[32];    // WCHAR
    char16_t DeviceString[128]; // WCHAR
    uint32_t StateFlags;        // DWORD
    char16_t DeviceID[128];     // WCHAR
    char16_t DeviceKey[128];    // WCHAR
  } DISPLAY_DEVICEW, *PDISPLAY_DEVICEW, *LPDISPLAY_DEVICEW;

  typedef int32_t (*__attribute__((__ms_abi__)) fnEnumDisplayDevicesW)(
      char16_t *lpDevice, uint32_t iDevNum, PDISPLAY_DEVICEW lpDisplayDevice, uint32_t dwFlags);

  void *User32_dll = cosmo_dlopen("User32.dll", RTLD_LAZY);
  if (!User32_dll) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load User32.dll.");
    exit(1);
  }
  fnEnumDisplayDevicesW pEnumDisplayDevicesW = cosmo_dlsym(User32_dll, "EnumDisplayDevicesW");
  if (!pEnumDisplayDevicesW) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    ERR_REPORT("Could not load EnumDisplayDevicesW.");
    exit(1);
  }
  DISPLAY_DEVICEW displayDevice;
  displayDevice.cb = sizeof(DISPLAY_DEVICEW);
  pEnumDisplayDevicesW(NULL, 0, &displayDevice, 0);

  char *gpu_str_win32 = utf16_to_utf8(displayDevice.DeviceString);

  safe_strncpy(gpu_str, gpu_str_win32, MAX_STRLEN);

  free(gpu_str_win32);
  cosmo_dlclose(User32_dll);
  // free(gpu_str_win32);
  return;
}

void detect_disk_windows(void) {
  typedef uint32_t (*__attribute__((__ms_abi__))
                    fnGetDiskFreeSpaceExA)(char *lpDirectoryName,
                                           void *lpFreeBytesAvailableToCaller, // PULARGE_INTEGER
                                           void *lpTotalNumberOfBytes,         // PULARGE_INTEGER
                                           void *lpTotalNumberOfFreeBytes      // PULARGE_INTEGER
  );
  void *Kernel32_dll = cosmo_dlopen("Kernel32.dll", RTLD_LAZY);
  if (!Kernel32_dll) {
    ERR_REPORT("Could not load Kernel32.dll.");
    return;
  }

  fnGetDiskFreeSpaceExA pGetDiskFreeSpaceExA = cosmo_dlsym(Kernel32_dll, "GetDiskFreeSpaceExA");
  if (pGetDiskFreeSpaceExA) {
    char *disk_unit;
    long long totalBytes, freeBytes, usedBytes, disk_used, disk_total, disk_percentage;

    char drive[MAX_STRLEN];

    FILE *disk_file;

    char buf[MAX_STRLEN];
    if (GetSystemDirectoryA(buf, MAX_STRLEN)) {
      snprintf(drive, MAX_STRLEN, "%c:\\", buf[0]);
    } else {
      if (getenv("SystemDrive")) { // GetEnvironmentVariable("SystemDrive", buf, MAX_STRLEN)
        char *sysdrive = getenv("SystemDrive");
        snprintf(drive, MAX_STRLEN, "%c:\\", sysdrive[0]);
      } else {
        snprintf(drive, MAX_STRLEN, "C:\\");
      }
    }

    if (pGetDiskFreeSpaceExA(drive, NULL, &totalBytes, &freeBytes)) {
      usedBytes = totalBytes - freeBytes;

      if (usedBytes >= GB) {
        disk_used = usedBytes / GB;
        disk_unit = "G";
      } else {
        disk_used = usedBytes / MB;
        disk_unit = "M";
      }

      disk_total = totalBytes / GB;
      disk_percentage = (usedBytes * 100) / totalBytes;

      snprintf(disk_str, MAX_STRLEN, "%lld%s / %lldG (%lld%%) [%s]", disk_used, disk_unit,
               disk_total, disk_percentage, drive);
    }
  }
  cosmo_dlclose(Kernel32_dll);
  return;
}
void detect_shell_windows(void) {
  // Credits: fastfetch devs, microsoft copilot and baidu wenxinyiyan
  char *shell_name = getParentShellProcessName();
  if (shell_name) {
    safe_strncpy(shell_str, shell_name, MAX_STRLEN);

    free(shell_name);
    return;
  }
}
void detect_res_windows(void) {
  typedef int (*__attribute__((__ms_abi__)) fnGetSystemMetrics)(int nIndex);
  int width = 0;
  int height = 0;
  void *User32_dll = cosmo_dlopen("User32.dll", RTLD_LAZY);
  if (User32_dll) {
    fnGetSystemMetrics pGetSystemMetrics = cosmo_dlsym(User32_dll, "GetSystemMetrics");
    if (pGetSystemMetrics) {
      width = pGetSystemMetrics(78 /*SM_CXVIRTUALSCREEN*/);
      height = pGetSystemMetrics(79 /*SM_CYVIRTUALSCREEN*/);
    }
    cosmo_dlclose(User32_dll);
  } else {
    snprintf(res_str, MAX_STRLEN, "failed to call res");
    return;
  }
  snprintf(res_str, MAX_STRLEN, "%dx%d", width, height);
}

void detect_de_windows(void) {
  ///@see https://github.com/dylanaraps/neofetch/issues/1571
  if (strstr(distro_str, "Microsoft Windows 11") || strstr(distro_str, "Microsoft Windows 10") ||
      strstr(distro_str, "Microsoft Windows Server 2022") ||
      strstr(distro_str, "Microsoft Windows Server 2019") ||
      strstr(distro_str, "Microsoft Windows Server 2016")/*???*/) {
    safe_strncpy(de_str, "Fluent", MAX_STRLEN);
  } else if (strstr(distro_str, "Microsoft Windows 8")) {
    safe_strncpy(de_str, "Metro", MAX_STRLEN);
  }
}

/**
 *
 * static function definitions starts here
 *
 */

/// @brief GetRegU8StrValue - WIN32 utility functions (may fail)
/// @param predefined kNtHkeyClassesRoot kNtHkeyCurrentUser kNtHkeyLocalMachine kNtHkeyUsers
/// @param pathkey The path of a registry key relative to the key specified by the `predefined`
/// parameter.
/// @param key The name of the registry value.
/// @return char* - utf8 string (must be free()'d after use)
static char __attribute__((warn_unused_result)) *
    GetRegU8StrValue(int64_t predefined, const char16_t *pathkey, const char16_t *key) {

  const int key_read = 0x00020019;
  int64_t hkey;
  uint32_t str_size = MAX_STRLEN;
  char16_t buffer[MAX_STRLEN];
  int32_t nterrorval = 0;
  RegOpenKeyEx(predefined, pathkey, 0, key_read, &hkey);

  if (nterrorval = RegQueryValueEx(hkey, key, 0, NULL, (char *)buffer, &str_size)) {
    // perror("RegQueryValueEx");
    fprintf(
        stderr,
        "RegQueryValueEx returned %d !\ncheck "
        "https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes for more info\n",
        nterrorval);
    RegCloseKey(hkey);
    exit(1);
  }

  // //utf16le to utf8
  iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd == (iconv_t)-1) {
    perror("iconv_open");
    RegCloseKey(hkey);
    exit(1);
  }
  size_t utf8_size = str_size * 6 + 1;
  size_t utf16_size = str_size;
  char *utf8_buffer = (char *)malloc(utf8_size);
  if (utf8_buffer == NULL) {
    perror("malloc");
    iconv_close(cd);
    RegCloseKey(hkey);
    exit(1);
  }
  char *inptr = (char *)buffer;
  char *outptr = utf8_buffer;
  memset(utf8_buffer, 0, utf8_size);
  if (iconv(cd, &inptr, &utf16_size, &outptr, &utf8_size) == (size_t)-1) {
    perror("iconv");
    ERR_REPORT("Failed to convert to utf8.");
    free(utf8_buffer);
    iconv_close(cd);
    RegCloseKey(hkey);
    exit(1);
  }

  iconv_close(cd);
  RegCloseKey(hkey);

  // shrink_to_fit

  size_t utf8_str_len = strlen(utf8_buffer);
  char *dest = (char *)malloc(utf8_str_len + 1);
  if (dest == NULL) {
    perror("malloc");
    free(utf8_buffer);
    exit(1);
  }
  memcpy(dest, utf8_buffer, utf8_str_len);
  free(utf8_buffer);

  return dest;
}

static char __attribute__((warn_unused_result)) * utf16_to_utf8(const char16_t *input) {
  if (!input)
    exit(1);

  size_t input_length = strlen16(input);

  char *u8buf = (char *)malloc(input_length * 6 + 1);
  if (!u8buf)
    exit(1);

  memset(u8buf, 0, input_length * 6 + 1);

  iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd == (iconv_t)-1) {
    perror("iconv_open");
    free(u8buf);
    exit(1);
  }

  char16_t *input_ptr = (char16_t *)input;
  char *output_ptr = u8buf;
  size_t inbytesleft = input_length * sizeof(char16_t);
  size_t outbytesleft = input_length * 6;

  if (iconv(cd, (char **)&input_ptr, &inbytesleft, &output_ptr, &outbytesleft) == (size_t)-1) {
    perror("iconv");
    iconv_close(cd);
    free(u8buf);
    exit(1);
  }

  iconv_close(cd);

  size_t u8strsize = strlen(u8buf) + 1;
  char *new_str = realloc(u8buf, u8strsize);
  if (!new_str) {
    perror("realloc");
    return u8buf;
  }
  return new_str;
}

int isSpecialProcess(const char16_t *processName) {
  const char16_t *specialProcesses[] = {u"sudo",
                                        u"sudo.exe",
                                        u"gsudo.exe",
                                        u"su",
                                        u"su.exe",
                                        u"doas",
                                        u"doas.exe",
                                        u"strace",
                                        u"strace.exe",
                                        u"sshd",
                                        u"sshd.exe",
                                        u"gdb",
                                        u"gdb.exe",
                                        u"lldb",
                                        u"lldb.exe",
                                        u"guake-wrapped",
                                        u"guake-wrapped.exe",
                                        u"python",
                                        u"python.exe",
                                        u"screenfetch-c.com"};
  int numSpecialProcesses = sizeof(specialProcesses) / sizeof(specialProcesses[0]);
  for (int i = 0; i < numSpecialProcesses; i++) {
    if (strcasecmp16(processName, specialProcesses[i]) == 0) {
      return 1;
    }
  }
  return 0;
}
static SimpWin32ProcessInfo GetSimpWin32ProcessInfo(uint32_t processID) {
  SimpWin32ProcessInfo info = {0};
  int64_t hSnapshot = CreateToolhelp32Snapshot(0x00000002 /*TH32CS_SNAPPROCESS*/, 0);
  if (hSnapshot) {
    struct NtProcessEntry32 pe32 = {0};
    pe32.dwSize = sizeof(pe32);
    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (pe32.th32ProcessID == processID) {
          info.processID = pe32.th32ProcessID;
          info.parentProcessID = pe32.th32ParentProcessID;
          strncat16(info.processName, pe32.szExeFile, sizeof(info.processName) / sizeof(char16_t));
          break;
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }
  return info;
}

char *getParentShellProcessName() {
  uint32_t pid = GetCurrentProcessId();
  SimpWin32ProcessInfo pinfo = GetSimpWin32ProcessInfo(GetCurrentProcessId());
  char16_t currentProcessName[260] = {0};
  memcpy(currentProcessName, pinfo.processName, 260 * sizeof(char16_t));
  uint32_t ppid = pinfo.parentProcessID;
  SimpWin32ProcessInfo ppinfo = GetSimpWin32ProcessInfo(pinfo.parentProcessID);
  while (isSpecialProcess(ppinfo.processName) /*||
         !strcasecmp16(ppinfo.processName, currentProcessName)*/) {
    memmove(&pinfo, &ppinfo, sizeof(SimpWin32ProcessInfo));
    ppinfo = GetSimpWin32ProcessInfo(pinfo.parentProcessID);
  }
  char *shell_str = utf16_to_utf8(ppinfo.processName);
  return shell_str;
}
