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
 * bruh, dlopen isn't supported on x86-64 macos yet ... these codes only works on aarch64 macos... :(
 * (tested on macos14-vmware)
 * 
*/



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
        if (pIORegistryEntryCreateCFProperties(regEntry, &serviceDictionary, (void*)kCFAllocatorDefault,
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