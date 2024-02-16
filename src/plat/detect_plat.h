#ifndef SCREENFETCH_C_DETECT_PLAT_H
#define SCREENFETCH_C_DETECT_PLAT_H



void detect_distro_linux(void);
// void detect_distro_bsd(void){};//*bsd
// void detect_distro_darwin(void){};
// void detect_distro_solaris(void);//not supported by cosmo 
void detect_distro_windows(void);

void detect_cpu_linux(void);
// void detect_cpu_bsd(void);
// void detect_cpu_darwin(void);
// void detect_cpu_solaris(void);
void detect_cpu_windows(void);

void detect_gpu_linux(void);
// void detect_gpu_bsd(void);
// void detect_gpu_darwin(void);
// void detect_gpu_solaris(void);
void detect_gpu_windows(void);





#endif