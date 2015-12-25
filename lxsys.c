// Copyright (c) <2012> <Leif Asbrink>
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, 
// merge, publish, distribute, sublicense, and/or sell copies of 
// the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
// OR OTHER DEALINGS IN THE SOFTWARE.

#if DARWIN == 1
// include something here.......
#define TRUE_BSD FALSE
#else
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif
#ifdef BSD
#define TRUE_BSD TRUE
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <machine/cpufunc.h>
#include <machine/sysarch.h>
#else
#define TRUE_BSD FALSE
#include <sys/io.h>
#endif
#endif


#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "globdef.h"
#include "rusage.h"
#include "thrdef.h"
#include "uidef.h"
#include "lconf.h"
#include "xdef.h"
#include "ldef.h"
#include "hwaredef.h"
#include "options.h"
#include "sdrdef.h"


int open_USB2LPT(void);

struct termios old_options;
#if TRUE_BSD == TRUE
char serport_name[]="/dev/ttyd....";
#else
char serport_name[]="/dev/ttyS....?";
#endif

void print_procerr(int xxprint)
{
int i;
if( (xxprint&1) != 0)
  {
  i=0;
  if( (xxprint&2)==0)
    {
    i=1;
    printf("\nMMX supported by hardware");
    }
  if( (xxprint&4)==0 && simd_present != 0)
    {
    i=1;
    printf("\nSIMD (=sse) supported by hardware");
    }
  if(i!=0)
    {
    printf("\n/proc/cpuinfo says LINUX core is not compatible.");
    printf("\n\nLinrad will allow you to select routines that may be illegal");
    printf("\non your system.");
    printf("\nAny illegal instruction will cause a program crasch!!");
    printf("\nSeems just the sse flag in /proc/cpuinfo is missing.");
    printf("\nTo use fast routines with an older core, recompile LINUX");
    printf("\nwith appropriate patches.\n\n");
    }
  }
}

void mmxerr(void)
{
printf("\n\nCould not read file /proc/cpuinfo (flags:)");
printf("\nSetting MMX and SIMD flags from hardware");
printf("\nProgram may fail if kernel support is missing and modes"); 
printf("\nneeding MMX or SIMD are selected.");
printf("\n\n%s",press_enter);
fflush(stdout);
getchar();
}

int investigate_cpu(void)
{
int i;
#if TRUE_BSD == FALSE
int k, maxproc, maxproc_flag;
FILE *file;
char s[256];
char *flags="flags";
char *fmmx=" mmx";
char *fsse=" sse";
char *fht=" ht";
char *fprocessor="processor";
#endif
int xxprint;
int no_of_ht;
// If there is no mmx, do not use simd either.
tickspersec = sysconf(_SC_CLK_TCK);
xxprint=0;
#if CPU == CPU_INTEL
#if IA64 == 0
i=check_mmx();
#else
// We do not use assembly on 64 bit systems (yet??)
i=0;
i=2; // simd is OK, but not MMX.
#endif
mmx_present=i&1;
simd_present=i>>1;
#else
mmx_present=0;
simd_present=0;
#endif
#if TRUE_BSD == TRUE
size_t len=sizeof(no_of_processors);
int err;
if((err=sysctlbyname("hw.ncpu", &no_of_processors, 
                               &len, NULL, 0)) < 0)no_of_processors=1;
#else
no_of_processors=1;
no_of_ht=0;
maxproc=0;
maxproc_flag=0;
file = fopen("/proc/cpuinfo", "r");
if (file == NULL)
  {
  mmxerr();
  return 0;
  }
else
  {
  no_of_processors=0;
nxline:;
  if(fgets(s,256,file)==NULL)
    {
    if(no_of_processors>0)goto cpuinfo_ok;
    fclose(file);
#if CPU == CPU_INTEL
    mmxerr();
#endif
    no_of_processors=1;
    return 0;
    }
  else
    {
    for(i=0; i<9; i++) if(s[i]!=fprocessor[i])goto nxt; 
    k=8;
    while(s[k+1] == ' ' || s[k+1] == ':' || s[k+1] == 9)k++;
    i=atoi(&s[k]);
    if(i != no_of_processors)maxproc_flag++;
    no_of_processors++;
    if(i > maxproc)maxproc=i;
nxt:;
    k=0;
    while(s[k] != flags[0] && k < 10)k++;
    for(i=0; i<5; i++) if(s[i+k]!=flags[i])goto nxline; 
    k+=5;  
nxbln:      
    while(s[k]!=0 && s[k]!=' ')k++;
    while(s[k]!=0 && s[k]==' ')k++;
    if(s[k]!=0)
      {
      k--;
      for(i=0; i<4; i++)if(s[i+k]!=fmmx[i])goto notmmx;
      if(s[k+4] != ' ' && s[k+4] != 10)goto notmmx;
      xxprint|=2;
notmmx:;      
      for(i=0; i<4; i++)if(s[i+k]!=fsse[i])goto notsse;
      if(s[k+4] != ' ' && s[k+4] != 10)goto notsse;
      xxprint|=4;
notsse:;
      for(i=0; i<3; i++)if(s[i+k]!=fht[i])goto notht;
      if(s[k+3] != ' ' && s[k+3] != 10)goto notht;
      no_of_ht++;
notht:;
      k++;
      goto nxbln;
      }      
    xxprint|=1;
    }
  goto nxline;
cpuinfo_ok:;      
  fclose(file);
  }
if(maxproc_flag != 0)
  {
// If something went wrong, better underestimate the number of CPUs.
  maxproc++;
  if(no_of_processors > maxproc)no_of_processors=maxproc;
  }
if(no_of_processors == no_of_ht)
  {
// This is incorrect. In e.g. Ubuntu Studio 13.10 the hyperthreaded
// cores are not listed, but nevertheless the ht flag is set.
// In case it turns out to be interesting to control cpu affinities
// it will be necessary do something else here.
// Look for 'core id' and 'cpu cores'
  hyperthread_flag=TRUE;
  }
else
  {
  hyperthread_flag=FALSE;
  }  
#endif
return xxprint;
}



// Old versions of Linux have a special implementation of getrusage
// that returns the timing for the calling thread and not the timing for
// the process as prescribed by POSIX
// The configure script will set RESAGE_OLD=TRUE for those old linux
// kernels.
#if RUSAGE_OLD != TRUE
#include <sys/syscall.h>

void lir_system_times(double *cpu_time, double *total_time)
{
cpu_time[0]=lir_get_cpu_time();
total_time[0]=current_time();
}

void clear_thread_times(int no)
{
#if TRUE_BSD == TRUE 
thread_pid[no]=no;
#else
thread_pid[no]=syscall(SYS_gettid);
#endif
thread_tottim1[no]=current_time();
thread_cputim1[no]=lir_get_cpu_time();
thread_workload[no]=0;
}

void make_thread_times(int no)
{
float t1,t2;
thread_tottim2[no]=current_time();
thread_cputim2[no]=lir_get_thread_time(no);
t1=100*(thread_cputim2[no]-thread_cputim1[no]);
t2=thread_tottim2[no]-thread_tottim1[no];
if(t1>0 && t2>0)
  {
  thread_workload[no]=t1/t2;
  }
else
  {
  thread_workload[no]=0;
  }
}

double lir_get_cpu_time(void)
{
double tm;
struct rusage rudat;
getrusage(RUSAGE_SELF,&rudat);
tm=0.000001*(rudat.ru_utime.tv_usec + rudat.ru_stime.tv_usec)+ 
                    rudat.ru_utime.tv_sec + rudat.ru_stime.tv_sec;
return tm;
}

double lir_get_thread_time(int no)
{
#if TRUE_BSD == TRUE 
(void) no;
return 0.0;
#else
char fnam[80], info[512];
FILE *pidstat;
int j,k,m;
long long int ii1, ii2;
sprintf(fnam,"/proc/%d/task/%d/stat",thread_pid[no],thread_pid[no]);
pidstat=fopen(fnam,"r");
if(pidstat==NULL)return 0;
#if VALGRIND == TRUE
memset(info, 0, 512);
#endif
m=fread(info,1,512,pidstat);  
fclose(pidstat);  
j=0;
k=0;
while(k<13)
  {
  while(info[j]!= ' ' && j<m)j++;
  while(info[j]== ' ' && j<m)j++;
  k++;
  }
if(j>=m)return 0;
sscanf(&info[j], "%lld %lld", &ii1, &ii2);
return (double)(ii1+ii2)/tickspersec;
#endif
}
#endif

// Old versions of Linux have a special implementation of getrusage
// that returns the timing for the calling thread and not the timing for
// the process as prescribed by POSIX
// The configure script will set RESAGE_OLD=TRUE for those old linux
// kernels.
#if RUSAGE_OLD == TRUE

void make_thread_times(int no)
{
float t1,t2;
thread_tottim2[no]=current_time();
thread_cputim2[no]=lir_get_thread_time();
t1=100*(thread_cputim2[no]-thread_cputim1[no]);
t2=thread_tottim2[no]-thread_tottim1[no];
if(t1>0 && t2>0)
  {
  thread_workload[no]=t1/t2;
  }
else
  {
  thread_workload[no]=0;
  }
}

void clear_thread_times(int no)
{
thread_tottim1[no]=current_time();
thread_cputim1[no]=lir_get_cpu_time();
thread_workload[no]=0;
}

double lir_get_cpu_time(void)
{
int i;
float t1;
t1=0;
for(i=0; i<THREAD_MAX; i++)
  {
  if(thread_command_flag[i]!=THRFLAG_NOT_ACTIVE)
    {
    t1+=thread_workload[i];
    }
  }
return t1;
}

double lir_get_thread_time(void)
{
double tm;
struct rusage rudat;
getrusage(RUSAGE_SELF,&rudat);
tm=0.000001*(rudat.ru_utime.tv_usec + rudat.ru_stime.tv_usec)+ 
                    rudat.ru_utime.tv_sec + rudat.ru_stime.tv_sec;
return tm;
}

#endif


void lir_init_event(int i)
{
pthread_mutex_init(&lir_event_mutex[i],NULL);
lir_event_flag[i]=FALSE;
pthread_cond_init(&lir_event_cond[i], NULL);
}

void lir_close_event(int i)
{
pthread_mutex_destroy(&lir_event_mutex[i]);
pthread_cond_destroy(&lir_event_cond[i]);
}  


void lir_set_event(int no)
{
while(pthread_mutex_lock(&lir_event_mutex[no]) != 0)lir_sleep(10);
lir_event_flag[no]=TRUE;
pthread_cond_signal(&lir_event_cond[no]);
pthread_mutex_unlock(&lir_event_mutex[no]);
}    

void lir_await_event(int no)
{
while(pthread_mutex_lock(&lir_event_mutex[no]) != 0)lir_sleep(10);
if(lir_event_flag[no] != TRUE)
  {
  pthread_cond_wait(&lir_event_cond[no], &lir_event_mutex[no]);
  }
lir_event_flag[no]=FALSE;
while(pthread_mutex_unlock(&lir_event_mutex[no]) != 0)lir_sleep(10);
}

float lir_noisegen(int level) 
{
// Return a number distributed following a gaussian
// Mean value is 0 and sigma pow(2,level)
double y, z;
long int k;
k=random();
y = (double)k/(double)RAND_MAX; 
k=random();
z = (double)k/(double)RAND_MAX; 
return sin(z*2*PI_L)*sqrt(-2*log(y))*pow(2.0,0.5*(double)level);
}

void lir_mutex_init(void)
{
int i;
for(i=0; i<MAX_LIRMUTEX; i++)
  {
  pthread_mutex_init(&linux_mutex[i],NULL);
  }
}

void lir_mutex_destroy(void)
{
int i;
for(i=0; i<MAX_LIRMUTEX; i++)
  {
  pthread_mutex_destroy(&linux_mutex[i]);
  }
}

void lir_mutex_lock(int no)
{
pthread_mutex_lock(&linux_mutex[no]);
}

void lir_mutex_unlock(int no)
{
pthread_mutex_unlock(&linux_mutex[no]);
}

void lirerr(int errcod)
{
fprintf(STDERR,"\nlir error %d",errcod);
if(kill_all_flag) return;
DEB"\nlirerr(%d)",errcod);
if(dmp != 0)fflush(dmp);
lir_errcod=errcod;
lir_set_event(EVENT_KILL_ALL);
while(!kill_all_flag)lir_sleep(10000);
}

int lir_open_serport(int serport_number, int baudrate,int stopbit_flag, int rts_mode)
{
int rte;
int rc;
struct termios options;
rc=0;
if(serport != -1)return rc ;
if(serport_number < 1 || serport_number > 8)
  {
  rc=1279;
  return rc;
  }
if(serport_number <= 4)
  {
  sprintf(&serport_name[0],"%s","/dev/ttyS");
  sprintf(&serport_name[9],"%d",serport_number-1);
  }
else
  {
  sprintf(&serport_name[0],"%s","/dev/ttyUSB");
  sprintf(&serport_name[11],"%d",serport_number-5);
  }
serport=open(serport_name,O_RDWR  | O_NOCTTY | O_NDELAY);
if (serport == -1)
  {
  rc=1244;
  return rc;
  }
fcntl(serport,F_SETFL,0);           //blocking I/O
//fcntl(serport, F_SETFL, FNDELAY);     //non blocking I/O
if(tcgetattr(serport,&options) != 0)
  {
  rc=1277;
  goto close_x;
  }
switch ( baudrate )
  {
  case 110: 
  rte=B110;
  break;
  
  case 300: 
  rte=B300;
  break;
  
  case 600: 
  rte=B600;
  break;
  
  case 1200: 
  rte=B1200;
  break;
  
  case 2400: 
  rte=B2400;
  break;
  
  case 4800: 
  rte=B4800;
  break;
  
  case 9600: 
  rte=B9600;
  break;
  
  case 19200: 
  rte=B19200;
  break;
  
  case 38400: 
  rte=B38400;
  break;
  
  case 57600: 
  rte=B57600;
  break;

  default: 
  rc=1280;
  goto close_x;
  }
old_options=options;
cfsetispeed(&options,rte);
cfsetospeed(&options,rte);
//CLOCAL means don’t allow
//control of the port to be changed
//CREAD says to enable the receiver
options.c_cflag|= (CLOCAL | CREAD);
// no parity, 2 or 1 stopbits, 8 bits per word
options.c_cflag&= ~PARENB; //  no parity=>disable the "enable parity bit" PARENB
if(stopbit_flag)
  {
  options.c_cflag|= CSTOPB;// =>2 stopbits
  }
else
  {
  options.c_cflag&= ~CSTOPB;//=> 1 stopbit   
  }
options.c_cflag&= ~CSIZE;   // clear size-bit by anding with negation
options.c_cflag|= CS8;      // =>set size to 8 bits per word
// raw input /output
options.c_oflag &= ~OPOST; 
options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//Set the timeouts:
//VMIN is the minimum amount characters to read
options.c_cc[VMIN]=0;
//The amount of time to wait
//for the amount of data
//specified by VMIN in tenths
//of a second.
options.c_cc[VTIME] = 1;
//  Select  flow control  mode
switch (rts_mode)
  {
  case 0:
  options.c_cflag &= ~CRTSCTS;       // no handshaking
  options.c_iflag &= ~IXON;
  break;
  
  case 1:
  options.c_cflag &= ~CRTSCTS;       // Enable Xon/Xoff software handshaking 
  options.c_iflag |= IXON;		
  break;
  
  case 2:
  options.c_cflag |= CRTSCTS;        // Enable Hardware RTS handshaking 
  options.c_iflag &= ~IXON;
  break;
  }
//flush I/O  buffers and apply the settings
if(tcsetattr(serport, TCSAFLUSH,&options) != 0) 
  {
  rc=1278;
close_x:  
  close(serport);
  serport=-1;
  }
return rc;
}

void lir_close_serport(void)
{
if(serport == -1)return;
if(tcsetattr(serport, TCSAFLUSH,&old_options) != 0)lirerr(1278);
close(serport);
serport=-1;
}

int lir_write_serport(void *s, int bytes)
{
return write(serport,s,bytes);
}

int lir_read_serport(void *s, int bytes)
{
int retnum, nread = 0;
while (bytes > 0) {
	retnum = read (serport, (char *)(s + nread), bytes);
	if (retnum <= 0) return -1;
	nread += retnum;
	bytes -= retnum;
	}	
return nread;
}

int lir_parport_permission(int portno)
{
int i;
// If the user selected USB2LPT16_PORT_NUMBER we will use USB for
// the 25 pin parallel port. It may not be permitted in case
// the user does not have privileges.
// verify that we can open the device.
if (wse.parport == USB2LPT16_PORT_NUMBER)
  {
  if(!usb2lpt_flag)usb2lpt_flag=open_USB2LPT();
  return usb2lpt_flag;
  }
#if DARWIN == 0
// Get permission to write to the parallel port
#ifdef BSD
#ifdef i386_set_ioperm
i=i386_set_ioperm(portno,4,1);
#else
i=portno;
return FALSE;
#endif
#else
if(portno < 0x400-4)
  {
  i=ioperm(portno,4,1);
  }
else
  {
  i=iopl(3);
  }
#endif  
if(i != 0)
  {
  lir_text(2,2,"Access to parallel port denied.");
  lir_text(2,4,"Use sudo or run as root");
  lir_text(2,6,press_any_key);
  await_keyboard();
  clear_screen();
  return FALSE;
  }
return TRUE;
#else
i=portno;
i=FALSE;
return i;
#endif
}

void lir_sched_yield(void)
{
sched_yield();
}

void win_global_uiparms(int n)
{
(void) n;
// Dummy routine. Not used under Linux.
}

void linrad_thread_create(int no)
{
thread_status_flag[no]=THRFLAG_INIT;
thread_command_flag[no]=THRFLAG_ACTIVE;
pthread_create(&thread_identifier[no],NULL,(void*)thread_routine[no], NULL);
threads_running=TRUE;
}

void fix_prio(int thread)
{
#if OSNUM == OSNUM_LINUX
int policy;
int i, k;
if(ui.process_priority != 0)
  {
  struct sched_param parms;
  pthread_getschedparam(thread_identifier[thread],&policy,&parms); 
  policy=SCHED_FIFO;
  i=sched_get_priority_max(policy);
  switch (ui.process_priority)
    {
    case 3:
    k=i-1;
    break;
    
    case 2:
    k=(i+parms.sched_priority)/2;
    break;
    
    default:
    k=parms.sched_priority+1;
    if(k > i) k=i;
    }
  parms.sched_priority=k;
  pthread_setschedparam(thread_identifier[thread],policy,&parms); 
  }
#endif
}


void thread_rx_output(void)
{
fix_prio(THREAD_RX_OUTPUT);
rx_output();
thread_status_flag[THREAD_RX_OUTPUT]=THRFLAG_RETURNED;
}

void thread_kill_all(void)
{
// Wait until the event is set.
// Then stop all processing threads so main can write any
// error code/message and exit.
lir_await_event(EVENT_KILL_ALL);
kill_all();
}

double current_time(void)
{
struct timeval t;
gettimeofday(&t,NULL);
recent_time=0.000001*t.tv_usec+t.tv_sec;
return recent_time;
}

int ms_since_midnight(int set)
{
int i;
double dt1;
dt1=current_time();
if(set)netstart_time=dt1;
i=dt1/(24*3600);
dt1-=24*3600*i;
i=1000*dt1;
return i%(24*3600000);
}

void lir_sync(void)
{
// This routine is called to force a write to the hard disk
sync();
}


int lir_get_epoch_seconds(void)
{
struct timeval tim;
gettimeofday(&tim,NULL);
return tim.tv_sec;
}

void lir_join(int no)
{
lir_sched_yield();
if(thread_command_flag[no]==THRFLAG_NOT_ACTIVE)return;
pthread_join(thread_identifier[no],0);
thread_status_flag[no]=THRFLAG_NOT_ACTIVE;
}

void lir_sleep(int us)
{
usleep(us);
}

void lir_outb(char byte, int port)
{    
int i;
if(libusb1_library_flag == TRUE || libusb0_library_flag == TRUE) 
  {
  if(wse.parport==USB2LPT16_PORT_NUMBER)
    {
    out_USB2LPT(byte,(port-wse.parport));
    }
  }
else
  {
#if DARWIN == 0 
#ifdef BSD
#ifdef i386_set_ioperm
  i=i386_set_ioperm(port,4,1);
#else
  return;
#endif  
#else
  if(port < 0x400-4)
    {
    i=ioperm(port,4,1);
    }
  else
    {
    i=iopl(3);
    }
  #endif  
  if(i!=0)lirerr(764921);
#if TRUE_BSD == TRUE
    {
    int k;
    k=byte;
    outb(k,port);
    }
#else
  outb(byte,port);
#endif
#else
i=byte+port;
#endif
  }
i=1000;
while(i>0)i--;
}

char lir_inb(int port)
{
#if DARWIN == 0
int i;
#endif
if(wse.parport==USB2LPT16_PORT_NUMBER)
  {
  if(libusb1_library_flag == TRUE || libusb0_library_flag == TRUE) 
    {
    return in_USB2LPT((port-wse.parport));
    }
  else
    {
    lirerr(7211332);
    return 0;
    }
  }
else
  {
#if TRUE_BSD == TRUE
#ifdef i386_set_ioperm
  i=i386_set_ioperm(port,4,1);
#else
  return 0;
#endif
#else
#if DARWIN == 0
  if(port < 0x400-4)
    {
    i=ioperm(port,4,1);
    }
  else
    {
    i=iopl(3);
    }
#endif   
#endif
#if DARWIN == 0
if(i!=0)lirerr(764921);
return inb(port);
#endif
  }  
return port;
}

// *********************************************************************
// Thread entries for Linux
// *********************************************************************

void thread_main_menu(void)
{
main_menu();
}

void thread_tune(void)
{
tune();
}

void thread_sdr14_input(void)
{
fix_prio(THREAD_SDR14_INPUT);
sdr14_input();
}

void thread_radar(void)
{
fix_prio(THREAD_RADAR);
run_radar();
}

void thread_blocking_rxout(void)
{
fix_prio(THREAD_BLOCKING_RXOUT);
blocking_rxout();
}

void thread_write_raw_file(void)
{
write_raw_file();
}

void thread_perseus_input(void)
{
fix_prio(THREAD_PERSEUS_INPUT);
perseus_input();
}

void thread_rtl2832_input(void)
{
fix_prio(THREAD_RTL2832_INPUT);
rtl2832_input();
}

void thread_mirics_input(void)
{
fix_prio(THREAD_MIRISDR_INPUT);
mirics_input();
}

void thread_bladerf_input(void)
{
fix_prio(THREAD_BLADERF_INPUT);
bladerf_input();
}

void thread_pcie9842_input(void)
{
fix_prio(THREAD_PCIE9842_INPUT);
pcie9842_input();
}

void thread_openhpsdr_input(void)
{
fix_prio(THREAD_OPENHPSDR_INPUT);
openhpsdr_input();
}

void thread_hware_command(void)
{
hware_command();
}

void thread_sdrip_input(void)
{
fix_prio(THREAD_SDRIP_INPUT);
sdrip_input();
}

void thread_netafedri_input(void)
{
fix_prio(THREAD_NETAFEDRI_INPUT);
netafedri_input();
}

void thread_excalibur_input(void)
{
fix_prio(THREAD_EXCALIBUR_INPUT);
excalibur_input();
}

void thread_cal_filtercorr(void)
{
cal_filtercorr();
}

void thread_cal_interval(void)
{
do_cal_interval();
}

void thread_user_command(void)
{
user_command();
}

void thread_narrowband_dsp(void)
{
fix_prio(THREAD_NARROWBAND_DSP);
narrowband_dsp();
}

void thread_do_fft1c(void)
{
fix_prio(THREAD_DO_FFT1C);
do_fft1c();
}

void thread_fft1b(void)
{
do_fft1b();
}

void thread_wideband_dsp(void)
{
fix_prio(THREAD_WIDEBAND_DSP);
wideband_dsp();
}

void thread_second_fft(void)
{
fix_prio(THREAD_SECOND_FFT);
second_fft();
}

void thread_timf2(void)
{
fix_prio(THREAD_TIMF2);
timf2_routine();
}

void thread_tx_input(void)
{
fix_prio(THREAD_TX_INPUT);
tx_input();
}

void thread_tx_output(void)
{
fix_prio(THREAD_TX_OUTPUT);
run_tx_output();
}

void thread_screen(void)
{
screen_routine();
}

void thread_rx_file_input(void)
{
fix_prio(THREAD_RX_FILE_INPUT);
rx_file_input();
}

void thread_cal_iqbalance(void)
{
cal_iqbalance();
}

void thread_rx_adtest(void)
{
rx_adtest();
}

void thread_powtim(void)
{
powtim();
}

void thread_txtest(void)
{
txtest();
}

void thread_syscall(void)
{
do_syscall();
}

void thread_extio_input(void)
{
fix_prio(THREAD_EXTIO_INPUT);
extio_input();
}

void thread_rtl_starter(void)
{
rtl_starter();
}

void thread_fdms1_input(void)
{
fix_prio(THREAD_FDMS1_INPUT);
fdms1_input();
}

void thread_fdms1_starter(void)
{
fdms1_starter();
}

void thread_airspy_input(void)
{
fix_prio(THREAD_FDMS1_INPUT);
airspy_input();
}

void thread_airspy_starter(void)
{
airspy_starter();
}

void thread_mirisdr_starter(void)
{
mirisdr_starter();
}

void thread_bladerf_starter(void)
{
bladerf_starter();
}

void thread_tx_hand_key(void)
{
tx_hand_key();
}

void thread_html_server(void)
{
#if SERVER == 1
html_server();
#endif
}

