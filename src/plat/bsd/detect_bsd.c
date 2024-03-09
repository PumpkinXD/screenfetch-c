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


//GPU
//  case $kernel_name in
//                 "FreeBSD"* | "DragonFly"*)
//                     gpu="$(pciconf -lv | grep -B 4 -F "VGA" | grep -F "device")"
//                     gpu="${gpu/*device*= }"
//                     gpu="$(trim_quotes "$gpu")"
//                 ;;

//                 *)





void detect_gpu_bsd(){
  FILE *fp;
  char line[1024];
  char gpu_vendor_id[16] = {0};
  char gpu_device_id[16] = {0};
  char *tmp;
  const char freebsd_cmd[] = "pciconf -lv | grep -B 4 -F \"VGA\" | grep -F \"device\"";

  const char freebsd_command[] =
      "pciconf -lv | grep -B 4 -F \"VGA\" | awk '/device = / {print substr($0, "
      "index($0, \"'\") + 1, length($0) - index($0, \"'\") - 2)}'";
}