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

/* gnu */
#include <iconv.h>
#include <regex.h>

/* cosmo libc includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <dlfcn.h> /* for loading native functions */
#include <mntent.h>
#include <glob.h>

/* program includes */
#include "../../detect.h"
#include "../../arrays.h"
#include "../../colors.h"
#include "../../misc.h"
#include "../../disp.h"
#include "../../util.h"
#include "../../error_flag.h"
#include "../detect_plat.h"
#include "x11_native_functions.h"

static void detect_gpu_xorg(void);
static void detect_gpu_wayland(void);
static void detect_gpu_lspci(void);

static void detect_res_wayland(void);
static void detect_res_xorg(void);
static void detect_res_drm(void);

/***
 *
 *  Modified from detect_distro(void); (src/plat/linux/detect.c, master branch)
 *
 *
 */
void detect_distro_linux(void) {
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
          fscanf(distro_file, "%s ",
                 distro_name_str); // maybe `lsb_release -d` is better than `lsb_release -i`???
          fclose(distro_file);

          snprintf(distro_str, MAX_STRLEN, "%s", distro_name_str + 11);
          safe_strncpy(host_color, TLRD, MAX_STRLEN);

          if (STREQ(distro_str, "neon")) { // workaround for KDE neon
            safe_strncpy(distro_str, "KDE neon", MAX_STRLEN);
            detected = true;
            safe_strncpy(host_color, TLGN, MAX_STRLEN);
            return;
          }
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
void detect_pkgs_linux(void) {

  //(default) package manager names
  char *package_manager_names[] = {"UnkownPKG", "dpkg", "pkg", "rpm", "pacman"};
  int main_pkg_type = 0; // 0==unknown 1==dpkg 2==pkg 3==rpm 4==pacman
  // char other_packages[MAX_STRLEN] = {0}; // snap, flatpak, brew, nix. etc... (optional managers)
  typedef struct {
    char *pkgmanname;
    int pkgs;
  } opt_pkg;
  opt_pkg opt_pkgs[4] = {{"flatpak", 0}, {"snap", 0}, {"brew", 0}, {"nix", 0}};

  if (has_command("flatpak")) {
    glob_t gl_flatpak;
    // https://github.com/fastfetch-cli/fastfetch/blob/50da7cabd61c6c7e7930375de4920b93c41aea07/src/detection/packages/packages_linux.c#L198
    if (!glob("/var/lib/flatpak/runtime/*", GLOB_NOSORT, NULL, &gl_flatpak)) {
      opt_pkgs[0].pkgs += gl_flatpak.gl_pathc;
      // printf("%d \n", gl_flatpak.gl_pathc);
      globfree(&gl_flatpak);
    }
  }
  if (has_command("snap")) {
    /// TODO: count snap pkgs
  }

  // parts from master branch
  FILE *pkgs_file;
  int packages = 0; // main pkgs, as original src suggest (default package manager)

  glob_t gl;

  if (STREQ(distro_str, "Arch Linux") || STREQ(distro_str, "ParabolaGNU/Linux-libre") ||
      STREQ(distro_str, "Chakra") || STREQ(distro_str, "Manjaro")) {
    if (!(glob("/var/lib/pacman/local/*", GLOB_NOSORT, NULL, &gl))) {
      packages = gl.gl_pathc;
      main_pkg_type = 4;
    } else if (error) {
      ERR_REPORT("Failure while globbing packages.");
    }

    globfree(&gl);
  } else if (STREQ(distro_str, "Frugalware")) {
    pkgs_file = popen("pacman-g2 -Q 2> /dev/null | wc -l", "r");
    fscanf(pkgs_file, "%d", &packages);
    pclose(pkgs_file);
    main_pkg_type = 4;
  } else if (STREQ(distro_str, "Ubuntu") || STREQ(distro_str, "Lubuntu") ||
             STREQ(distro_str, "Xubuntu") || STREQ(distro_str, "LinuxMint") ||
             STREQ(distro_str, "SolusOS") || STREQ(distro_str, "Debian") ||
             STREQ(distro_str, "LMDE") || STREQ(distro_str, "CrunchBang") ||
             STREQ(distro_str, "Peppermint") || STREQ(distro_str, "LinuxDeepin") ||
             STREQ(distro_str, "Trisquel") || STREQ(distro_str, "elementary OS") ||
             STREQ(distro_str, "Backtrack Linux") || STREQ(distro_str, "Kali Linux") ||
             STREQ(distro_str, "KDE neon")) {
    if (!(glob("/var/lib/dpkg/info/*.list", GLOB_NOSORT, NULL, &gl))) {
      packages = gl.gl_pathc;
      main_pkg_type = 1;
    } else if (error) {
      ERR_REPORT("Failure while globbing packages.");
    }

    globfree(&gl);
  } else if (STREQ(distro_str, "Slackware")) {
    if (!(glob("/var/log/packages/*", GLOB_NOSORT, NULL, &gl))) {
      packages = gl.gl_pathc;
    } else if (error) {
      ERR_REPORT("Failure while globbing packages.");
    }

    globfree(&gl);
  } else if (STREQ(distro_str, "Gentoo") || STREQ(distro_str, "Sabayon") ||
             STREQ(distro_str, "Funtoo")) {
    if (!(glob("/var/db/pkg/*/*", GLOB_NOSORT, NULL, &gl))) {
      packages = gl.gl_pathc;
    } else if (error) {
      ERR_REPORT("Failure while globbing packages.");
    }

    globfree(&gl);
  } else if (STREQ(distro_str, "Fuduntu") || STREQ(distro_str, "Fedora") ||
             STREQ(distro_str, "OpenSUSE") || STREQ(distro_str, "Red Hat Linux") ||
             STREQ(distro_str, "Mandriva") || STREQ(distro_str, "Mandrake") ||
             STREQ(distro_str, "Mageia") || STREQ(distro_str, "Viperr")) {
    /* RPM uses Berkeley DBs internally, so this won't change soon */
    pkgs_file = popen("rpm -qa 2> /dev/null | wc -l", "r");
    fscanf(pkgs_file, "%d", &packages);
    pclose(pkgs_file);
    main_pkg_type = 3;
  } else if (STREQ(distro_str, "Angstrom")) {
    pkgs_file = popen("opkg list-installed 2> /dev/null | wc -l", "r");
    fscanf(pkgs_file, "%d", &packages);
    pclose(pkgs_file);
  } else if (STREQ(distro_str, "Linux")) /* if linux disto detection failed */
  {
    safe_strncpy(pkgs_str, "Not Found", MAX_STRLEN);

    if (error)
      ERR_REPORT("Packages cannot be detected on an unknown "
                 "Linux distro.");
  }
  if (main_pkg_type) {
    int letters_written =
        snprintf(pkgs_str, MAX_STRLEN, "%d (%s)", packages, package_manager_names[main_pkg_type]);
    for (int i; i < 4; i++) {
      if (opt_pkgs[i].pkgs) {
        letters_written = snprintf(pkgs_str + letters_written, MAX_STRLEN, ", %d (%s)",
                                   opt_pkgs[i].pkgs, opt_pkgs[i].pkgmanname);
      }
    }
  } else {
    snprintf(pkgs_str, MAX_STRLEN, "%d", packages);
  }
  // hm... maybe I...

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

void detect_gpu_linux(void) {
  const char *sessionType = getenv("XDG_SESSION_TYPE");

  if (sessionType) {
    if (STREQ(sessionType, "wayland")) {
      detect_gpu_wayland();
      return;
    } else if (STREQ(sessionType, "x11")) {
      detect_gpu_xorg();
      // detect_gpu_lspci();
      return;
    } else {
      detect_gpu_lspci(); // from neofetch
      return;
    }
  }
}

void detect_disk_linux(void) {
  FILE *mnt_file;
  struct mntent *ent;
  struct statvfs fs;
  unsigned long long disk_total = 0, disk_used = 0, disk_pct = 0;

  if ((mnt_file = setmntent("/etc/mtab", "r"))) {
    while ((ent = getmntent(mnt_file))) {
      /* we only want to get the size of "real" disks (starting with /) */
      if (ent->mnt_dir && ent->mnt_fsname && ent->mnt_fsname[0] == '/') {
        if (!statvfs(ent->mnt_dir, &fs)) {
          disk_total += (fs.f_blocks * fs.f_bsize);
          disk_used += ((fs.f_blocks - fs.f_bfree) * fs.f_bsize);
        } else {
          ERR_REPORT("Could not stat filesystem for statistics (detect_disk).");
        }
      }
    }

    disk_total /= GB;
    disk_used /= GB;
    disk_pct = disk_total > 0 ? (disk_used * 100 / disk_total) : 0;

    snprintf(disk_str, MAX_STRLEN, "%llu%s / %llu%s (%llu%%)", disk_used, "GiB", disk_total, "GiB",
             disk_pct);

    endmntent(mnt_file);
  } else if (error) {
    ERR_REPORT("Could not open /etc/mtab (detect_disk).");
  }

  return;
}
void detect_res_linux(void) {
  const char *sessionType = getenv("XDG_SESSION_TYPE");
  if (sessionType) {
    if (STREQ(sessionType, "wayland")) {
      detect_res_wayland();
      return;
    } else if (STREQ(sessionType, "x11")) {
      detect_res_xorg();
      return;
    } else {
      detect_res_drm();
      return;
    }
  } else {
    detect_res_drm();
    return;
  }
}

/// @brief Copied from detect_de(void); (src/plat/linux/detect.c, master branch)
///  TODO: add more DEs
void detect_de_linux(void) {
  char *curr_de;

  if ((curr_de = getenv("XDG_CURRENT_DESKTOP"))) {
    if (STREQ(curr_de, "KDE")) {
      if (atoi(getenv("KDE_SESSION_VERSION")) > 4) {
        snprintf(de_str, MAX_STRLEN, "KDE Plasma %s", getenv("KDE_SESSION_VERSION"));
        /// TODO:https://github.com/dylanaraps/neofetch/blob/ccd5d9f52609bbdcd5d8fa78c4fdb0f12954125f/neofetch#L1859
      } else {
        snprintf(de_str, MAX_STRLEN, "KDE %s", getenv("KDE_SESSION_VERSION"));
      }
      return;
    }
    safe_strncpy(de_str, curr_de, MAX_STRLEN);
  } else {
    if (getenv("GNOME_DESKTOP_SESSION_ID")) {
      safe_strncpy(de_str, "Gnome", MAX_STRLEN);
    } else if (getenv("MATE_DESKTOP_SESSION_ID")) {
      safe_strncpy(de_str, "MATE", MAX_STRLEN);
    } else if (getenv("KDE_FULL_SESSION")) {
      /*	KDE_SESSION_VERSION only exists on KDE4+, so
        getenv will return NULL on KDE <= 3.
       */
      snprintf(de_str, MAX_STRLEN, "KDE%s", getenv("KDE_SESSION_VERSION"));
    } else if (error) {
      ERR_REPORT("No desktop environment found.");
    }
  }

  return;
}

/**
 *
 * static function definitions starts here
 *
 */

void detect_gpu_xorg(void) {
  void *libX11_so = cosmo_dlopen("libX11.so", RTLD_LAZY);
  void *libGL_so = cosmo_dlopen("libGL.so", RTLD_LAZY);
  void *libGLX_so = cosmo_dlopen("libGLX.so", RTLD_LAZY);
  if (!libGL_so || !libGL_so || !libGLX_so) {
    detect_gpu_lspci();
    return;
  }

  fnXOpenDisplay pXOpenDisplay;
  fnXCloseDisplay pXCloseDisplay;
  fnXFree pXFree;

  fnglXChooseVisual pglXChooseVisual;
  fnglXCreateContext pglXCreateContext;
  fnglXMakeCurrent pglXMakeCurrent;
  fnglGetString pglGetString;
  fnglXDestroyContext pglXDestroyContext;

  // load fns
  pXOpenDisplay = cosmo_dlsym(libX11_so, "XOpenDisplay");
  pXCloseDisplay = cosmo_dlsym(libX11_so, "XCloseDisplay");
  pXFree = cosmo_dlsym(libX11_so, "XFree");

  pglGetString = cosmo_dlsym(libGL_so, "glGetString");
  pglXChooseVisual = cosmo_dlsym(libGLX_so, "glXChooseVisual");
  pglXCreateContext = cosmo_dlsym(libGLX_so, "glXCreateContext");
  pglXMakeCurrent = cosmo_dlsym(libGLX_so, "glXMakeCurrent");
  pglXDestroyContext = cosmo_dlsym(libGLX_so, "glXDestroyContext");
  if (!pXOpenDisplay || !pXCloseDisplay || !pXFree || !pglGetString || !pglXChooseVisual ||
      !pglXCreateContext || !pglXMakeCurrent || !pglXDestroyContext) {

    exit(1);
  }

  {
    Display *disp = NULL;
    Window wind;
    GLint attr[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    XVisualInfo *visual_info = NULL;
    GLXContext context = NULL;

    if ((disp = pXOpenDisplay(NULL))) {
      wind = DefaultRootWindow(disp);

      if ((visual_info = pglXChooseVisual(disp, 0, attr))) {
        if ((context = pglXCreateContext(disp, visual_info, NULL, 1))) {
          pglXMakeCurrent(disp, wind, context);
          safe_strncpy(gpu_str, (const char *)pglGetString(GL_RENDERER), MAX_STRLEN);

          pglXDestroyContext(disp, context);
        } else if (error) {
          ERR_REPORT("Failed to create OpenGL context.");
        }

        pXFree(visual_info);
      } else if (error) {
        ERR_REPORT("Failed to select a proper X visual.");
      }

      pXCloseDisplay(disp);
    } else if (error) {
      safe_strncpy(gpu_str, "No X Server", MAX_STRLEN);
      ERR_REPORT("Could not open an X display (detect_gpu).");
    }
  }
  cosmo_dlclose(libX11_so);
  cosmo_dlclose(libGL_so);
  cosmo_dlclose(libGLX_so);
  return;
}

void detect_gpu_wayland(void) {
  // workaound, idk how to fetch gpu info via wayland's api atm
  detect_gpu_lspci();
  // uh...  has xwayland -> xorg ver?
  //

  // libEGL.so
  //  void* libEGL_so;//huh
  // libegl-wayland.so
  //  void* libwayland_egl_so;//huh

  // EGL+WAYLAND???
  // EGL+OPENGL ES???
}

// translated from neofetch by microsoft copilot
void detect_gpu_lspci(void) {
  FILE *fp;
  char gpu_name[MAX_STRLEN];
  // sh command:
  //  lspci -mm | awk -F '\"' '/"VGA compatible controller"/{print $4, $6}'
  fp = popen("lspci -mm | awk -F '\"' '/\"VGA compatible controller\"/{print $4, $6}'", "r");
  if (fp == NULL) {
    safe_strncpy(gpu_str, "Unknown", MAX_STRLEN);
    ERR_REPORT("Failed to run command (detect_gpu).");
    return;
  } else {
    if (fgets(gpu_name, sizeof(gpu_name) - 1, fp) != NULL) {
      gpu_name[strcspn(gpu_name, "\n")] = 0;
      safe_strncpy(gpu_str, gpu_name, MAX_STRLEN);
    }
  }

  /* close */
  pclose(fp);
}

void detect_res_wayland(void){
    /// TODO:impl this
};
void detect_res_xorg(void) {
  int width = 0, height = 0;
  Display *disp;
  Screen *screen;
  void *libX11_so = cosmo_dlopen("libX11.so", RTLD_LAZY);
  if (!libX11_so) {
    safe_strncpy(res_str, "No X Server", MAX_STRLEN);
    return;
  }
  fnXOpenDisplay pXOpenDisplay = cosmo_dlsym(libX11_so, "XOpenDisplay");
  fnXCloseDisplay pXCloseDisplay = cosmo_dlsym(libX11_so, "XCloseDisplay");
  fnXDefaultScreenOfDisplay pXDefaultScreenOfDisplay =
      cosmo_dlsym(libX11_so, "XDefaultScreenOfDisplay");
  if (pXOpenDisplay && pXCloseDisplay && pXDefaultScreenOfDisplay) {
    if ((disp = pXOpenDisplay(NULL))) {
      screen = pXDefaultScreenOfDisplay(disp);
      width = WidthOfScreen(screen);
      height = HeightOfScreen(screen);

      snprintf(res_str, MAX_STRLEN, "%dx%d", width, height);

      pXCloseDisplay(disp);
    } else {
      safe_strncpy(res_str, "No X Server", MAX_STRLEN);

      if (error)
        ERR_REPORT("Could not open an X display (detect_res)");
    }
  }
  dlclose(libX11_so);
  return;
};
void detect_res_drm(void) {

  safe_strncpy(res_str, "detect_res_drm() not implemented yet", MAX_STRLEN);

  /// TODO: "translate" following script(from neofetch) thing to C
  ///
  ///
  /// ``` sh
  ///             elif[[-d / sys / class / drm]]; then
  ///                 for dev in /sys/class/drm/*/modes; do
  ///                     read -r single_resolution _ < "$dev"
  ///
  ///                     [[ $single_resolution ]] && resolution="${single_resolution},
  ///                     ${resolution}"
  ///                 done
  ///             fi
  /// ```
};