
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>

#include "globdef.h"
#include "uidef.h"
#include "sdrdef.h"
#include "fft1def.h"
#include "screendef.h"
#include "rusage.h"
#include "thrdef.h"
#include "vernr.h"
#include "hwaredef.h"
#include "options.h"

#define MINIMAL_BUF_LENGTH		512
#define MAXIMAL_BUF_LENGTH		(256 * 16384)

#define MIRI_MIN_RATE 1500000
#define MIRI_MAX_RATE 15000000


typedef struct mirisdr_dev mirisdr_dev_t;
typedef void(*mirisdr_read_async_cb_t) (unsigned char *buf, uint32_t len, void *ctx);

typedef int (*p_mirisdr_open)(mirisdr_dev_t **p, uint32_t iindex);
typedef int (*p_mirisdr_close)(mirisdr_dev_t *p);
typedef int (*p_mirisdr_reset_buffer)(mirisdr_dev_t *p);
typedef int (*p_mirisdr_get_tuner_gains)(mirisdr_dev_t *dev, int *gains);
typedef int (*p_mirisdr_set_sample_rate)(mirisdr_dev_t *p, uint32_t rate);
typedef uint32_t (*p_mirisdr_get_sample_rate)(mirisdr_dev_t *p);
typedef uint32_t (*p_mirisdr_get_device_count)(void);
typedef const char* (*p_mirisdr_get_device_name)(uint32_t iindex);
typedef int (*p_mirisdr_get_device_usb_strings)(uint32_t iindex, char *manufact, char *product, char *serial);
typedef int (*p_mirisdr_set_transfer)(mirisdr_dev_t *p, char *v);
typedef int (*p_mirisdr_read_async)(mirisdr_dev_t *p, mirisdr_read_async_cb_t cb, void *ctx, uint32_t num, uint32_t len);
typedef int (*p_mirisdr_cancel_async)(mirisdr_dev_t *p);
typedef int (*p_mirisdr_read_sync)(mirisdr_dev_t *p, void *buf, int len, int *n_read);
typedef int (*p_mirisdr_set_bandwidth)(mirisdr_dev_t *p, uint32_t bw);
typedef int (*p_mirisdr_set_center_freq)(mirisdr_dev_t *p, uint32_t freq);
typedef int (*p_mirisdr_set_if_freq)(mirisdr_dev_t *p, uint32_t freq);
typedef int (*p_mirisdr_set_sample_format)(mirisdr_dev_t *p, char *v);
typedef int (*p_mirisdr_set_tuner_gain)(mirisdr_dev_t *p, int gain);
p_mirisdr_open mirisdr_open;
p_mirisdr_close mirisdr_close;
p_mirisdr_reset_buffer mirisdr_reset_buffer;
p_mirisdr_get_tuner_gains mirisdr_get_tuner_gains;
p_mirisdr_set_sample_rate mirisdr_set_sample_rate;
p_mirisdr_get_sample_rate mirisdr_get_sample_rate;
p_mirisdr_get_device_count mirisdr_get_device_count;
p_mirisdr_get_device_name mirisdr_get_device_name;
p_mirisdr_get_device_usb_strings mirisdr_get_device_usb_strings;
p_mirisdr_set_transfer mirisdr_set_transfer;
p_mirisdr_read_async mirisdr_read_async;
p_mirisdr_cancel_async mirisdr_cancel_async;
p_mirisdr_read_sync mirisdr_read_sync;
p_mirisdr_set_bandwidth mirisdr_set_bandwidth;
p_mirisdr_set_center_freq mirisdr_set_center_freq;
p_mirisdr_set_if_freq mirisdr_set_if_freq;
p_mirisdr_set_sample_format mirisdr_set_sample_format;
p_mirisdr_set_tuner_gain mirisdr_set_tuner_gain;

static mirisdr_dev_t *dev_mirics;

// Structure for Mirics parameters
typedef struct {
int sampling_speed;
int sernum1;
int sernum2;
int sernum3;
int freq_adjust;
int format;
int if_freq;
int if_bandwidth;
int transfer_type;
int check;
}P_MIRISDR;
#define MAX_MIRISDR_PARM 10

P_MIRISDR mirics;
char *mirics_parfil_name="par_mirics";
int cancel_flag;
int mirisdr_library_flag;
char *mirics_parm_text[MAX_MIRISDR_PARM]={"Sampling speed",
                                     "Serno1",
                                     "Serno2",
                                     "Serno3",
                                     "Freq adjust",
                                     "Sampl format",
                                     "IF freq",
                                     "IF bandwidth",
                                     "USB transfer", 
                                     "Check"};

#if(OSNUM == OSNUM_WINDOWS)
#include <windows.h>
HANDLE mirisdr_libhandle;

void load_mirisdr_library(void)
{
char mirics_dllname[80];
int info;
if(mirisdr_library_flag)return;
info=0;
sprintf(mirics_dllname,"%smirisdr%s",DLLDIR,DLLEXT);
mirisdr_libhandle=LoadLibraryEx(mirics_dllname, NULL, 
                                             LOAD_WITH_ALTERED_SEARCH_PATH);
if(!mirisdr_libhandle)goto mirisdr_load_error;
info=1;
mirisdr_open=(p_mirisdr_open)GetProcAddress(mirisdr_libhandle, "mirisdr_open");
if(!mirisdr_open)goto mirisdr_sym_error;
mirisdr_close=(p_mirisdr_close)GetProcAddress(mirisdr_libhandle, "mirisdr_close");
if(!mirisdr_close)goto mirisdr_sym_error;
mirisdr_reset_buffer=(p_mirisdr_reset_buffer)GetProcAddress(mirisdr_libhandle, "mirisdr_reset_buffer");
if(!mirisdr_reset_buffer)goto mirisdr_sym_error;
mirisdr_get_tuner_gains=(p_mirisdr_get_tuner_gains)GetProcAddress(mirisdr_libhandle, "mirisdr_get_tuner_gains");
if(!mirisdr_get_tuner_gains)goto mirisdr_sym_error;
mirisdr_set_sample_rate=(p_mirisdr_set_sample_rate)GetProcAddress(mirisdr_libhandle, "mirisdr_set_sample_rate");
if(!mirisdr_set_sample_rate)goto mirisdr_sym_error;
mirisdr_get_sample_rate=(p_mirisdr_get_sample_rate)GetProcAddress(mirisdr_libhandle, "mirisdr_get_sample_rate");
if(!mirisdr_get_sample_rate)goto mirisdr_sym_error;
mirisdr_get_device_count=(p_mirisdr_get_device_count)GetProcAddress(mirisdr_libhandle, "mirisdr_get_device_count");
if(!mirisdr_get_device_count)goto mirisdr_sym_error;
mirisdr_get_device_name=(p_mirisdr_get_device_name)GetProcAddress(mirisdr_libhandle, "mirisdr_get_device_name");
if(!mirisdr_get_device_name)goto mirisdr_sym_error;
mirisdr_get_device_usb_strings=(p_mirisdr_get_device_usb_strings)GetProcAddress(mirisdr_libhandle, "mirisdr_get_device_usb_strings");
if(!mirisdr_get_device_usb_strings)goto mirisdr_sym_error;
mirisdr_set_transfer=(p_mirisdr_set_transfer)GetProcAddress(mirisdr_libhandle, "mirisdr_set_transfer");
if(!mirisdr_set_transfer)goto mirisdr_sym_error;
mirisdr_read_async=(p_mirisdr_read_async)GetProcAddress(mirisdr_libhandle, "mirisdr_read_async");
if(!mirisdr_read_async)goto mirisdr_sym_error;
mirisdr_cancel_async=(p_mirisdr_cancel_async)GetProcAddress(mirisdr_libhandle, "mirisdr_cancel_async");
if(!mirisdr_cancel_async)goto mirisdr_sym_error;
mirisdr_read_sync=(p_mirisdr_read_sync)GetProcAddress(mirisdr_libhandle, "mirisdr_read_sync");
if(!mirisdr_read_sync)goto mirisdr_sym_error;
mirisdr_set_bandwidth=(p_mirisdr_set_bandwidth)GetProcAddress(mirisdr_libhandle, "mirisdr_set_bandwidth");
if(!mirisdr_set_bandwidth)goto mirisdr_sym_error;
mirisdr_set_center_freq=(p_mirisdr_set_center_freq)GetProcAddress(mirisdr_libhandle, "mirisdr_set_center_freq");
if(!mirisdr_set_center_freq)goto mirisdr_sym_error;
mirisdr_set_if_freq=(p_mirisdr_set_if_freq)GetProcAddress(mirisdr_libhandle, "mirisdr_set_if_freq");
if(!mirisdr_set_if_freq)goto mirisdr_sym_error;
mirisdr_set_sample_format=(p_mirisdr_set_sample_format)GetProcAddress(mirisdr_libhandle, "mirisdr_set_sample_format");
if(!mirisdr_set_sample_format)goto mirisdr_sym_error;
mirisdr_set_tuner_gain=(p_mirisdr_set_tuner_gain)GetProcAddress(mirisdr_libhandle, "mirisdr_set_tuner_gain");
if(!mirisdr_set_tuner_gain)goto mirisdr_sym_error;
mirisdr_library_flag=TRUE;
return;
mirisdr_sym_error:;
FreeLibrary(mirisdr_libhandle);
mirisdr_load_error:;
library_error_screen(mirics_dllname,info);
}

void unload_mirisdr_library(void)
{
if(!mirisdr_library_flag)return;
FreeLibrary(mirisdr_libhandle);
mirisdr_library_flag=FALSE;
}
#endif

#if(OSNUM == OSNUM_LINUX)
#include <dlfcn.h>

void *mirisdr_libhandle;

void load_mirisdr_library(void)
{
int info;
if(mirisdr_library_flag)return;
info=0;
mirisdr_libhandle=dlopen(MIRISDR_LIBNAME, RTLD_LAZY);
if(!mirisdr_libhandle)goto mirisdr_load_error;
info=1;
mirisdr_open=(p_mirisdr_open)dlsym(mirisdr_libhandle, "mirisdr_open");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_close=(p_mirisdr_close)dlsym(mirisdr_libhandle, "mirisdr_close");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_reset_buffer=(p_mirisdr_reset_buffer)dlsym(mirisdr_libhandle, "mirisdr_reset_buffer");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_get_tuner_gains=(p_mirisdr_get_tuner_gains)dlsym(mirisdr_libhandle, "mirisdr_get_tuner_gains");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_sample_rate=(p_mirisdr_set_sample_rate)dlsym(mirisdr_libhandle, "mirisdr_set_sample_rate");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_get_sample_rate=(p_mirisdr_get_sample_rate)dlsym(mirisdr_libhandle, "mirisdr_get_sample_rate");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_get_device_count=(p_mirisdr_get_device_count)dlsym(mirisdr_libhandle, "mirisdr_get_device_count");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_get_device_name=(p_mirisdr_get_device_name)dlsym(mirisdr_libhandle, "mirisdr_get_device_name");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_get_device_usb_strings=(p_mirisdr_get_device_usb_strings)dlsym(mirisdr_libhandle, "mirisdr_get_device_usb_strings");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_transfer=(p_mirisdr_set_transfer)dlsym(mirisdr_libhandle, "mirisdr_set_transfer");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_read_async=(p_mirisdr_read_async)dlsym(mirisdr_libhandle, "mirisdr_read_async");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_cancel_async=(p_mirisdr_cancel_async)dlsym(mirisdr_libhandle, "mirisdr_cancel_async");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_read_sync=(p_mirisdr_read_sync)dlsym(mirisdr_libhandle, "mirisdr_read_sync");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_bandwidth=(p_mirisdr_set_bandwidth)dlsym(mirisdr_libhandle, "mirisdr_set_bandwidth");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_center_freq=(p_mirisdr_set_center_freq)dlsym(mirisdr_libhandle, "mirisdr_set_center_freq");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_if_freq=(p_mirisdr_set_if_freq)dlsym(mirisdr_libhandle, "mirisdr_set_if_freq");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_sample_format=(p_mirisdr_set_sample_format)dlsym(mirisdr_libhandle, "mirisdr_set_sample_format");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_set_tuner_gain=(p_mirisdr_set_tuner_gain)dlsym(mirisdr_libhandle, "mirisdr_set_tuner_gain");
if(dlerror() != 0)goto mirisdr_sym_error;
mirisdr_library_flag=TRUE;
return;
mirisdr_sym_error:;
dlclose(mirisdr_libhandle);
mirisdr_load_error:;
library_error_screen(MIRISDR_LIBNAME,info);
}

void unload_mirisdr_library(void)
{
if(!mirisdr_library_flag)return;
dlclose(mirisdr_libhandle);
mirisdr_library_flag=FALSE;
}

#endif



static void mirisdr_callback(unsigned char *buf, uint32_t len, void *ctx)
{
(void) ctx;
unsigned int i, ll;
short int *iz, *buf16;
buf16=(short int*)buf;
// assume len is in bytes.
if( (ui.rx_input_mode&BYTE_INPUT) != 0)
  {
  for(i=0; i<len; i+=2)
    {
    iz=(short int*)&timf1_char[timf1p_sdr];
    iz[0]=((short int)buf[i  ])<<8;
    iz[1]=((short int)buf[i+1])<<8;
    timf1p_sdr=(timf1p_sdr+4)&timf1_bytemask;
    } 
  }
else
  {
  ll=len/2;
  for(i=0; i<ll; i+=2)
    {
    iz=(short int*)&timf1_char[timf1p_sdr];
    iz[0]=buf16[i];
    iz[1]=buf16[i+1];
    timf1p_sdr=(timf1p_sdr+4)&timf1_bytemask;
    }
  }
if( ((timf1p_sdr-timf1p_pa+timf1_bytes)&timf1_bytemask) >= 
                                                     snd[RXAD].block_bytes)
  {
  lir_set_event(EVENT_HWARE1_RXREADY);
  }
if(cancel_flag)
  {
  i=mirisdr_cancel_async(dev_mirics);
  }
}


void mirisdr_starter(void)
{
int i, j, k, no_of_buffers;
char *transfer_types[2]={"ISOC", "BULK"};
cancel_flag=FALSE;
k=snd[RXAD].block_bytes/2;
if(k < MINIMAL_BUF_LENGTH)k=MINIMAL_BUF_LENGTH;
if(k > MAXIMAL_BUF_LENGTH)k=MAXIMAL_BUF_LENGTH;
// Allocate buffers for 50 ms minimum.
mirisdr_set_transfer(dev_mirics, transfer_types[mirics.transfer_type]); 
no_of_buffers=(int)((0.1*mirics.sampling_speed)/k);
// Never use less than 4 buffers.
if(no_of_buffers < 4)no_of_buffers=4;
while(thread_status_flag[THREAD_MIRISDR_INPUT]!=THRFLAG_ACTIVE)
  {
  lir_sleep(10000);
  if(kill_all_flag)return;
  }
thread_status_flag[THREAD_MIRISDR_STARTER]=THRFLAG_ACTIVE;
lir_sched_yield();      
// Reset endpoint before we start reading from the MIRISDR (mandatory)
i=mirisdr_reset_buffer(dev_mirics);
if(i < 0)
  {
  lirerr(1378);
  return;
  }
mirisdr_read_async(dev_mirics, mirisdr_callback, NULL,
				      no_of_buffers, k);
i=mirisdr_read_sync(dev_mirics, timf1_char, k, &j);
}

void set_mirisdr_att(void)
{
mirisdr_set_tuner_gain(dev_mirics, old_mirics_gain);
}

void set_mirisdr_frequency(void)
{
mirisdr_set_center_freq(dev_mirics, 1000000*fg.passband_center);
}

char* get_mirics_format_string(void)
{
char* str;
str=NULL;
switch (mirics.format) 
  {
  case 0:
  str="AUTO";
  break;

  case 1:
  str="504_S8";
  break;

  case 2:
  str="504_S16";
  break;
  
  case 3:
  str="384_S16";
  break;

  case 4:
  str="336_S16";
  break;

  case 5:
  str="252_S16";
  break;
  }
return str;  
}    

void mirics_input(void)
{
#if RUSAGE_OLD == TRUE
int local_workload_counter;
#endif
int i, j;
int rxin_local_workload_reset;
char s[80];
double dt1, read_start_time, total_reads;
int timing_loop_counter,timing_loop_counter_max,initial_skip_flag;
int local_att_counter;
int local_nco_counter;
char vendor[256], product[256], serial[256];
int *ise;
float t1;
unsigned int no_of_mirics;
uint32_t idx;
lir_init_event(EVENT_HWARE1_RXREADY);
#if OSNUM == OSNUM_LINUX
clear_thread_times(THREAD_MIRISDR_INPUT);
#endif
#if RUSAGE_OLD == TRUE
local_workload_counter=workload_counter;
#endif
local_att_counter=sdr_att_counter;
local_nco_counter=sdr_nco_counter;
dt1=current_time();
i=read_sdrpar(mirics_parfil_name, MAX_MIRISDR_PARM, 
                                     mirics_parm_text, (int*)((void*)&mirics));
if(i != 0 || mirics.check != MIRISDRPAR_VERNR)
  {
  lirerr(1377);
  goto mirics_init_error_exit;
  }
t1=((rint)((mirics.sampling_speed*
                     (100000000L+(double)mirics.freq_adjust))/100000000L));
if(ui.rx_ad_speed != (int)t1)
  {
  lirerr(1377);
  goto mirics_init_error_exit;
  } 
timf1p_sdr=timf1p_pa;  
j=0;
load_mirisdr_library();
if(!mirisdr_library_flag)goto usb_error_exit;
while(sdr == -1)
  {
  ise=(int*)(void*)s;
  dev_mirics=NULL;
  no_of_mirics = mirisdr_get_device_count();
  for(idx=0; idx<no_of_mirics; idx++)
    {
    mirisdr_get_device_usb_strings(idx, vendor, product, serial);
    sprintf(s,"%s",serial);
    i=0;     
    while(i<16 && s[i]!=0)i++;
    while(i<16 )
      {
      s[i]=' ';
      i++;
      }
    if( mirics.sernum1 == ise[0] &&
        mirics.sernum2 == ise[1] &&
        mirics.sernum3 == ise[2])
      {
      sdr=mirisdr_open(&dev_mirics, idx);
      if(kill_all_flag)return;
      if(sdr >= 0)
        {
        i=mirisdr_set_sample_rate(dev_mirics, (uint32_t)mirics.sampling_speed);
        if(i < 0)
          {
          lirerr(1368);
opnerr:;        
          mirisdr_close(dev_mirics);
          return;
          }
        no_of_mirics_gains=mirisdr_get_tuner_gains(dev_mirics, NULL);
        if(no_of_mirics_gains < 1 || no_of_mirics_gains > 256)
          {
          lirerr(1413);
          goto opnerr;        
          }
        mirics_gains=malloc(no_of_mirics_gains*sizeof(int));
        if(mirics_gains == NULL)goto opnerr;
        mirisdr_get_tuner_gains(dev_mirics, mirics_gains);
        break;
        }
      else
        {
        if(j==0)
          {
          clear_screen();
          j=1;
          }
        sprintf(s,"Waiting %.2f", current_time()-dt1);
        lir_text(3,5,s);
        lir_refresh_screen();
        if(kill_all_flag)goto mirics_init_error_exit;
        lir_sleep(100000);
        }
      }
    }
  }
set_hardware_rx_gain();
set_mirisdr_att();
set_mirisdr_frequency();
get_mirics_format_string();
mirisdr_set_if_freq(dev_mirics, mirics.if_freq);
mirisdr_set_bandwidth(dev_mirics, mirics.if_bandwidth); 
mirisdr_set_sample_format(dev_mirics,get_mirics_format_string()); 
fft1_block_timing();
if(thread_command_flag[THREAD_SCREEN]!=THRFLAG_NOT_ACTIVE)
  {
  while(thread_status_flag[THREAD_SCREEN]!=THRFLAG_ACTIVE &&
        thread_status_flag[THREAD_SCREEN]!=THRFLAG_IDLE &&
        thread_status_flag[THREAD_SCREEN]!=THRFLAG_SEM_WAIT)
    {
    if(thread_command_flag[THREAD_MIRISDR_INPUT] ==
                                           THRFLAG_KILL)goto mirics_error_exit1;
    lir_sleep(10000);
    }
  }
#include "timing_setup.c"
linrad_thread_create(THREAD_MIRISDR_STARTER);
thread_status_flag[THREAD_MIRISDR_INPUT]=THRFLAG_ACTIVE;
lir_status=LIR_OK;
while(!kill_all_flag && 
            thread_command_flag[THREAD_MIRISDR_INPUT] == THRFLAG_ACTIVE)
  {
#if RUSAGE_OLD == TRUE
  if(local_workload_counter != workload_counter)
    {
    local_workload_counter=workload_counter;
    make_thread_times(THREAD_MIRISDR_INPUT);
    }
#endif
  if(local_att_counter != sdr_att_counter)
    {
    local_att_counter=sdr_att_counter;
    set_mirisdr_att();
    }
  if(local_nco_counter != sdr_nco_counter)
    {
    local_nco_counter=sdr_nco_counter;
    set_mirisdr_frequency();
    }
  lir_await_event(EVENT_HWARE1_RXREADY);
  if(kill_all_flag)goto mirics_error_exit1;
  while (!kill_all_flag && ((timf1p_pa-timf1p_sdr+timf1_bytes)&
                                  timf1_bytemask)>snd[RXAD].block_bytes)
    {
#include "input_speed.c"
    finish_rx_read();
    }
  }
mirics_error_exit1:;
cancel_flag=TRUE;
lir_join(THREAD_MIRISDR_STARTER);
mirisdr_close(dev_mirics);
free(mirics_gains);
usb_error_exit:;
unload_mirisdr_library();
mirics_init_error_exit:;
sdr=-1;
thread_status_flag[THREAD_MIRISDR_INPUT]=THRFLAG_RETURNED;
while(thread_command_flag[THREAD_MIRISDR_INPUT] != THRFLAG_NOT_ACTIVE)
  {
  lir_sleep(1000);
  }
lir_close_event(EVENT_HWARE1_RXREADY);
} 

int display_mirics_parm_info(int *line)
{
char s[80];
char *transfer_types[2]={"isosynchronous", "bulk"};
int errcod;
errcod=read_sdrpar(mirics_parfil_name, MAX_MIRISDR_PARM, 
                               mirics_parm_text, (int*)((void*)&mirics));
if(errcod == 0)
  {
  settextcolor(7);
  sprintf(s,"Sampling rate      = %i, Xtal adjust = %.0f ppb",  
                                        mirics.sampling_speed,
                                       10.*mirics.freq_adjust);
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;
  sprintf(s,"Sampling format    = %s  USB transfer: %s", 
                                get_mirics_format_string(),
                                   transfer_types[mirics.transfer_type]); 
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;
  sprintf(s,"IF frequency       = %d kHz, IF bandwidth = %d kHz",
                          mirics.if_freq/1000, mirics.if_bandwidth/1000); 
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;
  }
return (errcod);
}


void init_mirics(void)
{
FILE *mirics_file;
int i, line, no_of_mirics, devno;
int *ise, *sdr_pi;
char vendor[256], product[256], serial[256];
char s[120];
char *mirics_formats[]={"0=Auto",
                        "1=8 bit (504_S8) Max 12.096 MHz",
                        "2=8 bit (504_S16) Max 12.096 MHz",
                        "3=10+2 bit (384_S16) Max 9.216 MHz",
                        "4=12 bit (336_S16) Max 8.064 MHz",
                        "5=14 bit (252_S16) Max 6.048 MHz"
                        };
float t1;
ise=(int*)(void*)s;
line=0;
load_mirisdr_library();
if(!mirisdr_library_flag)return;
lir_text(3,2,"SEARCHING");
no_of_mirics = mirisdr_get_device_count();
clear_lines(2,2);
if (no_of_mirics == 0) 
  {
  lir_text(5,5,"Mirics not found.");
  lir_text(5,7,press_any_key);
  await_keyboard();
  goto unload_lib;
  }
line=2;
for(i=0; i<no_of_mirics; i++)
  {
  mirisdr_get_device_usb_strings(i, vendor, product, serial);
  sprintf(s," %2d: %s   MFG:%s, PROD:%s, SN: %s.", i, 
                mirisdr_get_device_name(i), vendor, product, serial);
  lir_text(3,line,s);
  line++;
  }
line++;
if (no_of_mirics == 1) 
  {
  lir_text(3,line,"Device autoselected.");
  devno=0;
  }
else
  {  
  lir_text(3, line, "Select device by line number:");
  devno=lir_get_integer(32,line,2,0,no_of_mirics-1);
  }
if(kill_all_flag)goto unload_lib;
mirisdr_get_device_usb_strings(devno, vendor, product, serial);
line+=2;
sprintf(s,"%s",serial);
i=0;
while(i<16 && s[i]!=0)i++;
while(i<16)
  {
  s[i]=' ';
  i++;
  }
mirics.sernum1=ise[0];
mirics.sernum2=ise[1];
mirics.sernum3=ise[2];

lir_text(3,line,mirics_formats[0]);
line++;
lir_text(3,line,mirics_formats[1]);
line++;
lir_text(3,line,mirics_formats[2]);
line++;
lir_text(3,line,mirics_formats[3]);
line++;
lir_text(3,line,mirics_formats[4]);
line++;
lir_text(3,line,mirics_formats[5]);
line++;
lir_text(3,line,"Select format for USB transfer (0 to 5) =>");
mirics.format=lir_get_integer(46,line,1,0,5);
if(kill_all_flag)goto unload_lib;
clear_screen();
lir_text(3,0,mirics_formats[mirics.format]);
line=2;
lir_text(3,line,"A=Zero IF");
line++;
lir_text(3,line,"B=450 kHz");
line++;
lir_text(3,line,"C=1620 kHz");
line++;
lir_text(3,line,"D=2048 kHz");
line++;
lir_text(3,line,"Select IF frequency (A to D) =>");
get_if_freq:;
await_processed_keyboard();
if(kill_all_flag)goto unload_lib;
switch(lir_inkey)
  {
  case 'A':
  mirics.if_freq=0;
  break;

  case 'B':
  mirics.if_freq=450000;
  break;

  case 'C':
  mirics.if_freq=1620000;
  break;

  case 'D':
  mirics.if_freq=2048000;
  break;

  default:
  goto get_if_freq;
  }
clear_lines(2,line);
line=2;  
sprintf(s,"Selected IF frequency is %d Hz",mirics.if_freq);
lir_text(3,line,s);
line +=2;  
lir_text(3,line,"A=200 kHz");
line++;
lir_text(3,line,"B=300 kHz");
line++;
lir_text(3,line,"C=600 kHz");
line++;
lir_text(3,line,"D=1536 kHz");
line++;
lir_text(3,line,"E=5 MHz");
line++;
lir_text(3,line,"F=6 MHz");
line++;
lir_text(3,line,"G=7 MHz");
line++;
lir_text(3,line,"H=8 MHz");
line++;
lir_text(3,line,"Select IF bandwidth (A to H) =>");
get_bw:;
to_upper_await_keyboard();
if(kill_all_flag)goto unload_lib;
switch(lir_inkey)
  {
  case 'A':
  mirics.if_bandwidth=200000;
  break;
  
  case 'B':
  mirics.if_bandwidth=300000;
  break;
  
  case 'C':
  mirics.if_bandwidth=600000;
  break;
  
  case 'D':
  mirics.if_bandwidth=153600;
  break;
  
  case 'E':
  mirics.if_bandwidth=5000000;
  break;
  
  case 'F':
  mirics.if_bandwidth=6000000;
  break;
  
  case 'G':
  mirics.if_bandwidth=7000000;
  break;
  
  case 'H':
  mirics.if_bandwidth=8000000;
  break;

  default:
  goto get_bw;
  }
line++;
sprintf(s,"Selected bandwidth is %d Hz",mirics.if_bandwidth);
lir_text(3,line,s);
line+=2;  
i=mirisdr_open(&dev_mirics, (uint32_t)devno);
if(i<0)
  {
#if(OSNUM == OSNUM_WINDOWS)
  lirerr(1398);
#else
  lirerr(1399);
#endif
  goto unload_lib;
  }
mirisdr_set_sample_format(dev_mirics,get_mirics_format_string()); 
mirisdr_set_if_freq(dev_mirics, mirics.if_freq);
mirisdr_set_bandwidth(dev_mirics, mirics.if_bandwidth); 
getspeed:;
sprintf(s,"Set sampling speed in Hz (%d to %d) =>",
                              MIRI_MIN_RATE, MIRI_MAX_RATE);
lir_text(3,line,s);
mirics.sampling_speed=lir_get_integer(4+strlen(s),line,10,
                              MIRI_MIN_RATE, MIRI_MAX_RATE);
line+=2;
i=mirisdr_set_sample_rate(dev_mirics, (uint32_t)mirics.sampling_speed);
mirics.sampling_speed=mirisdr_get_sample_rate(dev_mirics);
if(kill_all_flag)
  {
close_ret:;
  mirisdr_close(dev_mirics);
  goto unload_lib;
  }
if(i < 0)
  {
  lir_text(3,line,"Failed to set sampling rate");
  lir_text(3,line+1,press_any_key);
  await_keyboard();
  if(kill_all_flag)goto close_ret;
  clear_lines(line,line+1);
  goto getspeed;
  }
mirisdr_close(dev_mirics);
line++;
sprintf(s,"Actual sampling speed is %d Hz",mirics.sampling_speed);
lir_text(3,line,s);
line+=2;
lir_text(3,line,"Enter xtal error in ppb =>");
mirics.freq_adjust=0.1*lir_get_float(32,line,9,-300000.,300000.);
if(kill_all_flag)goto unload_lib;
line+=2;
#if OSNUM == OSNUM_LINUX
lir_text(3,line,"Set transfer type isosynchronous or bulk (I/B)=>");
get_transfer:;
await_processed_keyboard();
if(lir_inkey == 'I')
  {
  mirics.transfer_type=0;
  }
else
  {
  if(lir_inkey != 'B')goto get_transfer;
  mirics.transfer_type=1;
  }
#else
mirics.transfer_type=1;
#endif
mirics_file=fopen(mirics_parfil_name,"w");
if(mirics_file == NULL)
  {
  lirerr(381268);
  goto unload_lib;
  }
mirics.check=MIRISDRPAR_VERNR;
sdr_pi=(int*)(&mirics);
for(i=0; i<MAX_MIRISDR_PARM; i++)
  {
  fprintf(mirics_file,"%s [%d]\n",mirics_parm_text[i],sdr_pi[i]);
  }
parfile_end(mirics_file);
ui.rx_addev_no=MIRISDR_DEVICE_CODE;
t1=((rint)((mirics.sampling_speed*
                     (100000000L+(double)mirics.freq_adjust))/100000000L));
ui.rx_ad_speed=(int)t1;
ui.rx_input_mode=IQ_DATA;
if(mirics.format == 1)ui.rx_input_mode|=BYTE_INPUT;
ui.rx_rf_channels=1;
ui.rx_ad_channels=2;
ui.rx_admode=0;
unload_lib:;
unload_mirisdr_library();
}

