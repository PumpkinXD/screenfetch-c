#ifndef SCREENFETCH_C_DETECT_PLAT_H
#define SCREENFETCH_C_DETECT_PLAT_H

void detect_distro_linux(void);
// void detect_distro_bsd(void);//*bsd
// void detect_distro_darwin(void);
void detect_distro_darwin_x86workaround(void);
// void detect_distro_solaris(void);//not supported by cosmo
void detect_distro_windows(void);
void detect_distro_windows_v2(void);

void detect_pkgs_linux(void);
void detect_pkgs_darwin(void);
void detect_pkgs_windows(void);

void detect_cpu_linux(void);
// void detect_cpu_bsd(void);
void detect_cpu_darwin(void);
void detect_cpu_darwin_x86workaround(void);
// void detect_cpu_solaris(void);
void detect_cpu_windows(void);

void detect_gpu_linux(void);
// void detect_gpu_bsd(void);
void detect_gpu_darwin(void);
void detect_gpu_darwin_x86workaround(void);
// void detect_gpu_solaris(void);
void detect_gpu_windows(void);

void detect_disk_linux(void);
void detect_disk_windows(void);


void detect_shell_windows(void);

void detect_res_linux(void);
void detect_res_windows(void);

void detect_de_linux(void);
void detect_de_windows(void);



/*
* misc functions?
*/
int has_command(const char *cmd);
int has_command2(const char *cmd);
int has_command3(const char *cmd);

#endif