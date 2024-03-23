/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <third_party/getopt/long2.h>
#include <unistd.h>
#include <getopt.h>
#include <glob.h>

/* ints and chars */
#include <stdint.h>
#include <uchar.h>

/* GNU libs*/
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
#include "darwin_native_functions.h"

/**
 *
 * bruh, dlopen isn't supported on x86-64 macos yet ... these codes only works on aarch64 macos...
 * :( (tested on macos14-vmware)
 *
 */

// atm this only works on arm64
void detect_distro_darwin() {
  // read /System/Library/CoreServices/SystemVersion to obtain ver

  /*
    an example of /System/Library/CoreServices/SystemVersion.plist file(XML VER):
  ```xml
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"> 
    <plist version="1.0"> 
    <dict> 
      <key>BuildID</key>
      <string>8B216EB4-548C-11EE-AB67-32FDD54239E7</string>
      <key>ProductBuildVersion</key>
      <string>23A344</string>
      <key>ProductCopyright</key>
      <string>1983-2023 Apple Inc.</string>
      <key>ProductName</key>
      <string>macOS</string>
      <key>ProductUserVisibleVersion</key>
      <string>14.0</string>
      <key>ProductVersion</key>
      <string>14.0</string>
      <key>iOSSupportVersion</key>
      <string>17.0</string>
    </dict>
    </plist>
  ```
  */

  // ported from old screenfetch-c


  typedef void* CFArrayRef;
  fnCFStringCreateWithCString pCFStringCreateWithCString=NULL;
  fnCFStringCreateArrayBySeparatingStrings pCFStringCreateArrayBySeparatingStrings=NULL;
  fnCFPreferencesCopyAppValue pCFPreferencesCopyAppValue=NULL;
  fnCFStringGetIntValue pCFStringGetIntValue=NULL;
  fnCFStringGetCString pCFStringGetCString=NULL;
  fnCFArrayGetCount pCFArrayGetCount=NULL;
  fnCFArrayGetValueAtIndex pCFArrayGetValueAtIndex=NULL;

#define CFSTR(cstr_arg) pCFStringCreateWithCString(NULL, cstr_arg, kCFStringEncodingUTF8)

      void *CoreFoundation = cosmo_dlopen(
          "/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation", RTLD_LAZY);
  if (!CoreFoundation) {
  return;
  }
  pCFStringCreateWithCString = cosmo_dlsym(CoreFoundation, "CFStringCreateWithCString");
  pCFStringCreateArrayBySeparatingStrings =
      cosmo_dlsym(CoreFoundation, "CFStringCreateArrayBySeparatingStrings");
  pCFPreferencesCopyAppValue = cosmo_dlsym(CoreFoundation, "CFPreferencesCopyAppValue");
  pCFStringGetIntValue = cosmo_dlsym(CoreFoundation, "CFStringGetIntValue");
  pCFStringGetCString = cosmo_dlsym(CoreFoundation, "CFStringGetCString");
  pCFArrayGetCount = cosmo_dlsym(CoreFoundation, "CFArrayGetCount");
  pCFArrayGetValueAtIndex = cosmo_dlsym(CoreFoundation, "CFArrayGetValueAtIndex");

  if (!pCFStringCreateWithCString || !pCFStringCreateArrayBySeparatingStrings ||
      !pCFPreferencesCopyAppValue || !pCFStringGetIntValue || !pCFStringGetCString ||
      !pCFArrayGetCount || !pCFArrayGetValueAtIndex) {
    cosmo_dlclose(CoreFoundation);
    return;
  }
  //
  {
    char *codenames[] = {"Cheetah",       "Puma",        "Jaguar",       "Panther",
                         "Tiger",         "Leopard",     "Snow Leopard", "Lion",
                         "Mountain Lion", "Mavericks",   "Yosemite",     "El Capitan",
                         "Sierra",        "High Sierra", "Mojave",       "Catalina"};
    CFArrayRef split = pCFStringCreateArrayBySeparatingStrings(
        NULL,
        pCFPreferencesCopyAppValue(CFSTR("ProductVersion"),
                                  CFSTR("/System/Library/CoreServices/SystemVersion")),
        CFSTR("."));
    unsigned maj = pCFStringGetIntValue(pCFArrayGetValueAtIndex(split, 0));
    unsigned min = pCFStringGetIntValue(pCFArrayGetValueAtIndex(split, 1));
    unsigned fix = 0;
    if (pCFArrayGetCount(split) == 3) {
      fix = pCFStringGetIntValue(pCFArrayGetValueAtIndex(split, 2));
    }

    char build_ver[16];
    pCFStringGetCString(
        pCFPreferencesCopyAppValue(CFSTR("ProductBuildVersion"),
                                  CFSTR("/System/Library/CoreServices/SystemVersion")),
        build_ver, 16, kCFStringEncodingUTF8);

    char *codename = "Mac OS";
    char buf[128];
    if (maj<11&&min < sizeof(codenames) / sizeof(*codenames)) {
      snprintf(buf, sizeof(buf), "%s %s",
               min < 8    ? "Mac OS X"
               : min < 12 ? "OS X"
                          : "macOS",
               codenames[min]);
      codename = buf;
    } else {
      char *lookfor = "SOFTWARE LICENSE AGREEMENT FOR ";
      FILE *fp = fopen("/System/Library/CoreServices/Setup "
                       "Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf",
                       "r");
      if (fp != NULL) {
        for (int i = 0; i < 50 && fgets(buf, sizeof(buf), fp); ++i) {
          char *p = strstr(buf, lookfor);
          if (p) {
            codename = p + strlen(lookfor);
            codename[strlen(p) - strlen(lookfor) - 1] = '\0';
            break;
          }
        }
        fclose(fp);
      }
    }

    snprintf(distro_str, MAX_STRLEN, "%s %d.%d.%d (%s)", codename, maj, min, fix, build_ver);
    safe_strncpy(host_color, TLBL, MAX_STRLEN);
    cosmo_dlclose(CoreFoundation);
    return;
  }
#undef CFSTR
}

void detect_pkgs_darwin(void){
  int packages = 0;
  glob_t gl;

  // TODO: count macport and nix pkgs
  // via popen("type -p prog")???
  // cmdreturnval==pclose
  // macport path =""/var/macports/software""???

  //brew pkgs
  if (glob("/usr/local/Cellar/*", GLOB_NOSORT, NULL, &gl) == 0) {
    packages = gl.gl_pathc;
  } else if (error) {
    ERR_REPORT("Failure while globbing packages.");
  }

  globfree(&gl);

  int letter_written = snprintf(pkgs_str, MAX_STRLEN, "%d (brew)", packages);

  return;
};

void detect_cpu_darwin(void) {
  void *libSystem_dylib = cosmo_dlopen("/usr/lib/libSystem.B.dylib", RTLD_LAZY);
  if (!libSystem_dylib) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    exit(1);
  }
  fnsysctlbyname psysctlbyname = cosmo_dlsym(libSystem_dylib, "sysctlbyname");
  // todo:handle errors?

  size_t size = MAX_STRLEN;

  psysctlbyname("machdep.cpu.brand_string", cpu_str, &size, NULL, 0);

  cosmo_dlclose(libSystem_dylib);
}

void detect_gpu_darwin(void) {

  typedef void *CFMutableDictionaryRef;
  typedef uint32_t io_iterator_t;
  typedef uint32_t io_registry_entry_t;
  typedef void *CFDataRef;
  const uint64_t kCFAllocatorDefault = 0;

  void *IOKit = cosmo_dlopen("/System/Library/Frameworks/IOKit.framework/IOKit", RTLD_LAZY);
  void *CoreFoundation =
      cosmo_dlopen("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation", RTLD_LAZY);
  if (!IOKit || !CoreFoundation) {
    fprintf(stderr, "%s\n", cosmo_dlerror());
    exit(1);
  }

  fnIOServiceMatching pIOServiceMatching = cosmo_dlsym(IOKit, "IOServiceMatching");
  fnIOServiceGetMatchingServices pIOServiceGetMatchingServices =
      cosmo_dlsym(IOKit, "IOServiceGetMatchingServices");
  fnIOIteratorNext pIOIteratorNext = cosmo_dlsym(IOKit, "IOIteratorNext");
  fnIORegistryEntryCreateCFProperties pIORegistryEntryCreateCFProperties =
      cosmo_dlsym(IOKit, "IORegistryEntryCreateCFProperties");
  fnIOObjectRelease pIOObjectRelease = cosmo_dlsym(IOKit, "IOObjectRelease");

  fnCFDictionaryGetValue pCFDictionaryGetValue =
      cosmo_dlsym(CoreFoundation, "CFDictionaryGetValue");
  fnCFStringCreateWithCString pCFStringCreateWithCString =
      cosmo_dlsym(CoreFoundation, "CFStringCreateWithCString");
  fnCFGetTypeID pCFGetTypeID = cosmo_dlsym(CoreFoundation, "CFGetTypeID");
  fnCFDataGetTypeID pCFDataGetTypeID = cosmo_dlsym(CoreFoundation, "CFDataGetTypeID");
  fnCFDataGetBytePtr pCFDataGetBytePtr = cosmo_dlsym(CoreFoundation, "CFDataGetBytePtr");
  fnCFRelease pCFRelease = cosmo_dlsym(CoreFoundation, "CFRelease");

  {
    CFMutableDictionaryRef matchDict = pIOServiceMatching("IOPCIDevice");
    io_iterator_t iterator;
    if (pIOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) ==
        kIOReturnSuccess) {
      io_registry_entry_t regEntry;
      while ((regEntry = pIOIteratorNext(iterator))) {
        CFMutableDictionaryRef serviceDictionary;
        if (pIORegistryEntryCreateCFProperties(regEntry, &serviceDictionary,
                                               (void *)kCFAllocatorDefault,
                                               kNilOptions) != kIOReturnSuccess) {
          pIOObjectRelease(regEntry);
          continue;
        }
        const void *GPUModel = pCFDictionaryGetValue(
            serviceDictionary, pCFStringCreateWithCString(NULL, "model", kCFStringEncodingUTF8));
        if (GPUModel && pCFGetTypeID(GPUModel) == pCFDataGetTypeID())
          safe_strncpy(gpu_str, (char *)pCFDataGetBytePtr((CFDataRef)GPUModel), MAX_STRLEN);
        pCFRelease(serviceDictionary);
        pIOObjectRelease(regEntry);
      }
      pIOObjectRelease(iterator);
    }
  }

  // clean-up
  cosmo_dlclose(IOKit);
  cosmo_dlclose(CoreFoundation);
}