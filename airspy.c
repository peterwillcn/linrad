/*
Copyright (c) 2012, Jared Boone <jared@sharebrained.com>
Copyright (c) 2013, Michael Ossmann <mike@ossmann.com>
Copyright (c) 2013/2014, Benjamin Vernoux <bvernoux@airspy.com>
Copyright (C) 2013/2014, Youssef Touil <youssef@airspy.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
		Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
	documentation and/or other materials provided with the distribution.
		Neither the name of AirSpy nor the names of its contributors may be used to endorse or promote products derived from this software
	without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

typedef enum
{
	RECEIVER_MODE_OFF = 0,
	RECEIVER_MODE_RX = 1
} receiver_mode_t;

typedef enum 
{
	AIRSPY_SAMPLERATE_10MSPS = 0, // 12bits 10MHz IQ
	AIRSPY_SAMPLERATE_2_5MSPS = 1, // 12bits 2.5MHz IQ
	AIRSPY_SAMPLERATE_END = 2 // End index for sample rate (corresponds to number of samplerate)
} airspy_samplerate_t;
#define AIRSPY_CONF_CMD_SHIFT_BIT (3) // Up to 3bits=8 samplerates (airspy_samplerate_t enum shall not exceed 7)

// Commands (usb vendor request) shared between Firmware and Host.
#define AIRSPY_CMD_MAX (24)
typedef enum
{
	AIRSPY_INVALID                    = 0 ,
	AIRSPY_RECEIVER_MODE              = 1 ,
	AIRSPY_SI5351C_WRITE              = 2 ,
	AIRSPY_SI5351C_READ               = 3 ,
	AIRSPY_R820T_WRITE                = 4 ,
	AIRSPY_R820T_READ                 = 5 ,
	AIRSPY_SPIFLASH_ERASE             = 6 ,
	AIRSPY_SPIFLASH_WRITE             = 7 ,
	AIRSPY_SPIFLASH_READ              = 8 ,
	AIRSPY_BOARD_ID_READ              = 9 ,
	AIRSPY_VERSION_STRING_READ        = 10,
	AIRSPY_BOARD_PARTID_SERIALNO_READ = 11,
	AIRSPY_SET_SAMPLERATE             = 12,
	AIRSPY_SET_FREQ                   = 13,
	AIRSPY_SET_LNA_GAIN               = 14,
	AIRSPY_SET_MIXER_GAIN             = 15,
	AIRSPY_SET_VGA_GAIN               = 16,
	AIRSPY_SET_LNA_AGC                = 17,
	AIRSPY_SET_MIXER_AGC              = 18,
	AIRSPY_MS_VENDOR_CMD              = 19,
	AIRSPY_SET_RF_BIAS_CMD            = 20,
	AIRSPY_GPIO_WRITE                 = 21,
	AIRSPY_GPIO_READ                  = 22,
	AIRSPY_GPIODIR_WRITE              = 23,
	AIRSPY_GPIODIR_READ               = AIRSPY_CMD_MAX
} airspy_vendor_request;

typedef enum
{
	GPIO_PORT0 = 0,
	GPIO_PORT1 = 1,
	GPIO_PORT2 = 2,
	GPIO_PORT3 = 3,
	GPIO_PORT4 = 4,
	GPIO_PORT5 = 5,
	GPIO_PORT6 = 6,
	GPIO_PORT7 = 7
} airspy_gpio_port_t;

typedef enum
{
	GPIO_PIN0 = 0,
	GPIO_PIN1 = 1,
	GPIO_PIN2 = 2,
	GPIO_PIN3 = 3,
	GPIO_PIN4 = 4,
	GPIO_PIN5 = 5,
	GPIO_PIN6 = 6,
	GPIO_PIN7 = 7,
	GPIO_PIN8 = 8,
	GPIO_PIN9 = 9,
	GPIO_PIN10 = 10,
	GPIO_PIN11 = 11,
	GPIO_PIN12 = 12,
	GPIO_PIN13 = 13,
	GPIO_PIN14 = 14,
	GPIO_PIN15 = 15,
	GPIO_PIN16 = 16,
	GPIO_PIN17 = 17,
	GPIO_PIN18 = 18,
	GPIO_PIN19 = 19,
	GPIO_PIN20 = 20,
	GPIO_PIN21 = 21,
	GPIO_PIN22 = 22,
	GPIO_PIN23 = 23,
	GPIO_PIN24 = 24,
	GPIO_PIN25 = 25,
	GPIO_PIN26 = 26,
	GPIO_PIN27 = 27,
	GPIO_PIN28 = 28,
	GPIO_PIN29 = 29,
	GPIO_PIN30 = 30,
	GPIO_PIN31 = 31
} airspy_gpio_pin_t;


#define AIRSPY_VERSION "1.0.2"
#define AIRSPY_VER_MAJOR 1
#define AIRSPY_VER_MINOR 0
#define AIRSPY_VER_REVISION 2

#ifdef _WIN32
	 #define ADD_EXPORTS
	 
	// You should define ADD_EXPORTS *only* when building the DLL.
	#ifdef ADD_EXPORTS
		#define ADDAPI __declspec(dllexport)
	#else
		#define ADDAPI __declspec(dllimport)
	#endif

	// Define calling convention in one place, for convenience.
	#define ADDCALL __cdecl

#else // _WIN32 not defined.

	// Define with no value on non-Windows OSes.
	#define ADDAPI
	#define ADDCALL

#endif


enum airspy_error
{
	AIRSPY_SUCCESS = 0,
	AIRSPY_TRUE = 1,
	AIRSPY_ERROR_INVALID_PARAM = -2,
	AIRSPY_ERROR_NOT_FOUND = -5,
	AIRSPY_ERROR_BUSY = -6,
	AIRSPY_ERROR_NO_MEM = -11,
	AIRSPY_ERROR_LIBUSB = -1000,
	AIRSPY_ERROR_THREAD = -1001,
	AIRSPY_ERROR_STREAMING_THREAD_ERR = -1002,
	AIRSPY_ERROR_STREAMING_STOPPED = -1003,
	AIRSPY_ERROR_OTHER = -9999,
};

enum airspy_board_id
{
	AIRSPY_BOARD_ID_PROTO_AIRSPY  = 0,
	AIRSPY_BOARD_ID_INVALID = 0xFF,
};

enum airspy_sample_type
{
	AIRSPY_SAMPLE_FLOAT32_IQ = 0,   // 2 * 32bit float per sample 
	AIRSPY_SAMPLE_FLOAT32_REAL = 1, // 1 * 32bit float per sample 
	AIRSPY_SAMPLE_INT16_IQ = 2,     // 2 * 16bit int per sample 
	AIRSPY_SAMPLE_INT16_REAL = 3,   // 1 * 16bit int per sample 
	AIRSPY_SAMPLE_UINT16_REAL = 4,  // 1 * 16bit unsigned int per sample 
	AIRSPY_SAMPLE_END = 5           // Number of supported sample types 
};

struct airspy_device;

typedef struct {
	struct airspy_device* device;
	void* ctx;
	void* samples;
	int sample_count;
	enum airspy_sample_type sample_type;
} airspy_transfer_t, airspy_transfer;

typedef struct {
	uint32_t part_id[2];
	uint32_t serial_no[4];
} airspy_read_partid_serialno_t;

typedef struct {
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t revision;
} airspy_lib_version_t;

typedef int (*airspy_sample_block_cb_fn)(airspy_transfer* transfer);

extern ADDAPI void ADDCALL airspy_lib_version(airspy_lib_version_t* lib_version);

extern ADDAPI int ADDCALL airspy_exit(void);
 
extern ADDAPI int ADDCALL airspy_open_sn(struct airspy_device** device, uint64_t serial_number);
extern ADDAPI int ADDCALL airspy_open(struct airspy_device** device);
extern ADDAPI int ADDCALL airspy_close(struct airspy_device* device);

extern ADDAPI int ADDCALL airspy_get_samplerates(struct airspy_device* device, uint32_t* buffer, const uint32_t len);
extern ADDAPI int ADDCALL airspy_set_samplerate(struct airspy_device* device, uint32_t samplerate);

extern ADDAPI int ADDCALL airspy_start_rx(struct airspy_device* device, airspy_sample_block_cb_fn callback, void* rx_ctx);
extern ADDAPI int ADDCALL airspy_stop_rx(struct airspy_device* device);

// return AIRSPY_TRUE if success 
extern ADDAPI int ADDCALL airspy_is_streaming(struct airspy_device* device);

extern ADDAPI int ADDCALL airspy_si5351c_write(struct airspy_device* device, uint8_t register_number, uint8_t value);
extern ADDAPI int ADDCALL airspy_si5351c_read(struct airspy_device* device, uint8_t register_number, uint8_t* value);

extern ADDAPI int ADDCALL airspy_r820t_write(struct airspy_device* device, uint8_t register_number, uint8_t value);
extern ADDAPI int ADDCALL airspy_r820t_read(struct airspy_device* device, uint8_t register_number, uint8_t* value);

// Parameter value shall be 0=clear GPIO or 1=set GPIO 
extern ADDAPI int ADDCALL airspy_gpio_write(struct airspy_device* device, airspy_gpio_port_t port, airspy_gpio_pin_t pin, uint8_t value);
// Parameter value corresponds to GPIO state 0 or 1 
extern ADDAPI int ADDCALL airspy_gpio_read(struct airspy_device* device, airspy_gpio_port_t port, airspy_gpio_pin_t pin, uint8_t* value);

// Parameter value shall be 0=GPIO Input direction or 1=GPIO Output direction 
extern ADDAPI int ADDCALL airspy_gpiodir_write(struct airspy_device* device, airspy_gpio_port_t port, airspy_gpio_pin_t pin, uint8_t value);
extern ADDAPI int ADDCALL airspy_gpiodir_read(struct airspy_device* device, airspy_gpio_port_t port, airspy_gpio_pin_t pin, uint8_t* value);

extern ADDAPI int ADDCALL airspy_spiflash_erase(struct airspy_device* device);
extern ADDAPI int ADDCALL airspy_spiflash_write(struct airspy_device* device, const uint32_t address, const uint16_t length, unsigned char* const data);
extern ADDAPI int ADDCALL airspy_spiflash_read(struct airspy_device* device, const uint32_t address, const uint16_t length, unsigned char* data);

extern ADDAPI int ADDCALL airspy_board_id_read(struct airspy_device* device, uint8_t* value);
extern ADDAPI int ADDCALL airspy_version_string_read(struct airspy_device* device, char* version, uint8_t length);

extern ADDAPI int ADDCALL airspy_board_partid_serialno_read(struct airspy_device* device, airspy_read_partid_serialno_t* read_partid_serialno);

extern ADDAPI int ADDCALL airspy_set_sample_type(struct airspy_device* device, enum airspy_sample_type sample_type);

// Parameter freq_hz shall be between 24000000(24MHz) and 1750000000(1.75GHz) 
extern ADDAPI int ADDCALL airspy_set_freq(struct airspy_device* device, const uint32_t freq_hz);

// Parameter value shall be between 0 and 15 
extern ADDAPI int ADDCALL airspy_set_lna_gain(struct airspy_device* device, uint8_t value);

// Parameter value shall be between 0 and 15 
extern ADDAPI int ADDCALL airspy_set_mixer_gain(struct airspy_device* device, uint8_t value);

// Parameter value shall be between 0 and 15 
extern ADDAPI int ADDCALL airspy_set_vga_gain(struct airspy_device* device, uint8_t value);

// Parameter value:
//	0=Disable LNA Automatic Gain Control
//	1=Enable LNA Automatic Gain Control

extern ADDAPI int ADDCALL airspy_set_lna_agc(struct airspy_device* device, uint8_t value);
// Parameter value:
//	0=Disable MIXER Automatic Gain Control
//	1=Enable MIXER Automatic Gain Control

extern ADDAPI int ADDCALL airspy_set_mixer_agc(struct airspy_device* device, uint8_t value);

// Parameter value shall be 0=Disable BiasT or 1=Enable BiasT 
extern ADDAPI int ADDCALL airspy_set_rf_bias(struct airspy_device* dev, uint8_t value);

extern ADDAPI const char* ADDCALL airspy_error_name(enum airspy_error errcode);
extern ADDAPI const char* ADDCALL airspy_board_id_name(enum airspy_board_id board_id);

*/

#define _WIN32_WINNT 0x0502


#include <stdint.h>
#include "globdef.h"
#include "uidef.h"
#include "fft1def.h"
#include "screendef.h"
#include "vernr.h"
#include "rusage.h"
#include "thrdef.h"
#include "hwaredef.h"
#include "options.h"



typedef struct {
	struct airspy_device* device;
	void* ctx;
	void* samples;
	int sample_count;
	int sample_type;
} airspy_transfer_t, airspy_transfer;

typedef struct {
uint32_t part_id[2];
uint32_t serial_no[4];
} airspy_read_partid_serialno_t;

typedef int (*airspy_sample_block_cb_fn)(airspy_transfer* transfer);


#define AIRSPY_SUCCESS 0
#define AIRSPY_TRUE 1
#define AIRSPY_ERROR_INVALID_PARAM -2
#define AIRSPY_ERROR_NOT_FOUND -5
#define AIRSPY_ERROR_BUSY -6
#define AIRSPY_ERROR_NO_MEM -11
#define AIRSPY_ERROR_LIBUSB -1000
#define AIRSPY_ERROR_THREAD -1001
#define AIRSPY_ERROR_STREAMING_THREAD_ERR -1002
#define AIRSPY_ERROR_STREAMING_STOPPED -1003
#define AIRSPY_ERROR_OTHER -9999

#define AIRSPY_SAMPLE_INT16_IQ 2    // 2 * 16bit int per sample 
#define AIRSPY_SAMPLE_INT16_REAL 3  // 1 * 16bit int per sample 


struct airspy_device;

typedef int (*p_airspy_open)(struct airspy_device** device);
typedef char* (*p_airspy_error_name)(int code);
typedef int (*p_airspy_board_partid_serialno_read)(struct airspy_device* 
                              device, airspy_read_partid_serialno_t* id);
typedef int (*p_airspy_close)(struct airspy_device* device);
typedef int (*p_airspy_get_samplerates)(struct airspy_device* device, 
                                    uint32_t* buffer, const uint32_t len);
typedef int (*p_airspy_set_samplerate)(struct airspy_device* device, 
                                                           uint32_t rate);
typedef int (*p_airspy_set_sample_type)(struct airspy_device* 
                                                   device, int sample_type);
typedef int (*p_airspy_set_rf_bias)(struct airspy_device* dev, uint8_t x);
typedef int (*p_airspy_start_rx)(struct airspy_device* device, 
                          airspy_sample_block_cb_fn callback, void* rx_ctx);
typedef int (*p_airspy_set_freq)(struct airspy_device* device, 
                                                         uint32_t freq_hz);
typedef int (*p_airspy_set_lna_gain)(struct airspy_device* device, 
                                                          uint8_t value);
typedef int (*p_airspy_set_mixer_gain)(struct airspy_device* device,
                                                          uint8_t value);
typedef int (*p_airspy_set_vga_gain)(struct airspy_device* device, 
                                                          uint8_t value);
typedef int (*p_airspy_r820t_write)(struct airspy_device* device, 
                                    uint8_t register_number, uint8_t value);


p_airspy_open airspy_open;
p_airspy_error_name airspy_error_name;
p_airspy_board_partid_serialno_read airspy_board_partid_serialno_read;
p_airspy_close airspy_close;
p_airspy_get_samplerates airspy_get_samplerates;
p_airspy_set_samplerate airspy_set_samplerate;
p_airspy_set_sample_type airspy_set_sample_type;
p_airspy_set_rf_bias airspy_set_rf_bias;
p_airspy_start_rx airspy_start_rx;
p_airspy_set_freq airspy_set_freq;
p_airspy_set_lna_gain airspy_set_lna_gain;
p_airspy_set_mixer_gain airspy_set_mixer_gain;
p_airspy_set_vga_gain airspy_set_vga_gain;
p_airspy_r820t_write airspy_r820t_write;

#define MAX_AIRSPY_DEVICES 8

// Structure for Airspy parameters
typedef struct {
int ratenum;
unsigned int sernum1;
unsigned int sernum2;
int freq_adjust;
unsigned int gain_mode;
unsigned int bandwidth_no;
int bias_t;
int check;
}P_AIRSPY;
#define MAX_AIRSPY_PARM 8

#define MAX_AIRSPY_BW 12
float airspy_bw[MAX_AIRSPY_BW]={9.0F,8.0F,7.0F,6.0F,5.0F,3.4F,
                              3.0F,2.5F,2.0F,1.5F,1.0F,0.5F};
uint8_t reg_0xa[MAX_AIRSPY_BW]={0xa0,0xa4,0xa9,0xae,0xaf,0xa0,
                                0xa4,0xa9,0xa2,0xaf,0xaf,0xaf};
uint8_t reg_0xb[MAX_AIRSPY_BW]={0x0f,0x0f,0x0f,0x4f,0x6f,0x8f,
                               0x8f,0x8f,0xef,0xef,0xeb,0xe9};


P_AIRSPY airspy;
char *airspy_parfil_name="par_airspy";
struct airspy_device *dev_airspy;
int airspy_library_flag;
int cancel_flag;

char *airspy_parm_text[MAX_AIRSPY_PARM]={"Ratenum",
                                          "Serno1",
                                          "Serno2",
                                          "Freq adjust",
                                          "Gain Mode",
                                          "Bandwidth No",
                                          "Bias T",
                                          "Check"};

#if(OSNUM == OSNUM_WINDOWS)
#include <windows.h>
HANDLE airspy_libhandle;

void load_airspy_library(void)
{
char airspy_dllname[80];
int info;
if(airspy_library_flag)return;
info=0;
sprintf(airspy_dllname,"%sairspy%s",DLLDIR,DLLEXT);
airspy_libhandle=LoadLibraryEx(airspy_dllname, NULL, 
                                                LOAD_WITH_ALTERED_SEARCH_PATH);
if(!airspy_libhandle)goto airspy_load_error;
info=1;
airspy_open=(p_airspy_open)GetProcAddress(airspy_libhandle, "airspy_open");
if(!airspy_open)goto airspy_sym_error;
airspy_error_name=(p_airspy_error_name)GetProcAddress(
                                   airspy_libhandle, "airspy_error_name");
if(!airspy_error_name)goto airspy_sym_error;
airspy_board_partid_serialno_read=(p_airspy_board_partid_serialno_read) 
               GetProcAddress(airspy_libhandle, "airspy_board_partid_serialno_read");
if(!airspy_board_partid_serialno_read)goto airspy_sym_error;
airspy_close=(p_airspy_close)GetProcAddress(airspy_libhandle, "airspy_close");
if(!airspy_close)goto airspy_sym_error;
airspy_get_samplerates=(p_airspy_get_samplerates)GetProcAddress(airspy_libhandle, 
                                                   "airspy_get_samplerates");
airspy_set_samplerate=(p_airspy_set_samplerate)GetProcAddress(airspy_libhandle, 
                                                   "airspy_set_samplerate");
if(!airspy_set_samplerate)goto airspy_sym_error;
airspy_set_sample_type=(p_airspy_set_sample_type)GetProcAddress(airspy_libhandle, 
                                                   "airspy_set_sample_type");
if(!airspy_set_sample_type)goto airspy_sym_error;
airspy_set_rf_bias=(p_airspy_set_rf_bias)GetProcAddress(airspy_libhandle, 
                                                     "airspy_set_rf_bias");
if(!airspy_set_rf_bias)goto airspy_sym_error;
airspy_start_rx=(p_airspy_start_rx)GetProcAddress(airspy_libhandle, 
                                                       "airspy_start_rx");
if(!airspy_start_rx)goto airspy_sym_error;
airspy_set_freq=(p_airspy_set_freq)GetProcAddress(airspy_libhandle, 
                                                       "airspy_set_freq");
if(!airspy_set_freq)goto airspy_sym_error;
airspy_set_lna_gain=(p_airspy_set_lna_gain)GetProcAddress(airspy_libhandle, 
                                                   "airspy_set_lna_gain");
if(!airspy_set_lna_gain)goto airspy_sym_error;
airspy_set_mixer_gain=(p_airspy_set_mixer_gain)GetProcAddress(airspy_libhandle, 
                                                 "airspy_set_mixer_gain");
if(!airspy_set_mixer_gain)goto airspy_sym_error;
airspy_set_vga_gain=(p_airspy_set_vga_gain)GetProcAddress(airspy_libhandle, 
                                                  "airspy_set_vga_gain");
if(!airspy_set_vga_gain)goto airspy_sym_error;
airspy_r820t_write=(p_airspy_r820t_write)GetProcAddress(airspy_libhandle, 
                                                       "airspy_r820t_write");
if(!airspy_r820t_write)goto airspy_sym_error;
airspy_library_flag=TRUE;
return;
airspy_sym_error:;
FreeLibrary(airspy_libhandle);
airspy_load_error:;
library_error_screen(airspy_dllname,info);
}

void unload_airspy_library(void)
{
if(!airspy_library_flag)return;
FreeLibrary(airspy_libhandle);
airspy_library_flag=FALSE;
}
#endif

#if(OSNUM == OSNUM_LINUX)
#include <dlfcn.h>
static void *airspy_libhandle=NULL;

void load_airspy_library(void)
{ 
int info;
info=0;
if(airspy_library_flag)return;
airspy_libhandle=dlopen(AIRSPY_LIBNAME, RTLD_LAZY);
if(!airspy_libhandle)goto airspy_load_error;
info=1;
airspy_open=(p_airspy_open)dlsym(airspy_libhandle, "airspy_open");
if(dlerror() != 0)goto airspy_sym_error;
airspy_error_name=(p_airspy_error_name)dlsym(
                                   airspy_libhandle, "airspy_error_name");
if(dlerror() != 0)goto airspy_sym_error;
airspy_board_partid_serialno_read=(p_airspy_board_partid_serialno_read) 
               dlsym(airspy_libhandle, "airspy_board_partid_serialno_read");
if(dlerror() != 0)goto airspy_sym_error;
airspy_close=(p_airspy_close)dlsym(airspy_libhandle, "airspy_close");
if(dlerror() != 0)goto airspy_sym_error;
airspy_get_samplerates=(p_airspy_get_samplerates)dlsym(airspy_libhandle, 
                                                   "airspy_get_samplerates");
airspy_set_samplerate=(p_airspy_set_samplerate)dlsym(airspy_libhandle, 
                                                   "airspy_set_samplerate");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_sample_type=(p_airspy_set_sample_type)dlsym(airspy_libhandle, 
                                                   "airspy_set_sample_type");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_rf_bias=(p_airspy_set_rf_bias)dlsym(airspy_libhandle, 
                                                     "airspy_set_rf_bias");
if(dlerror() != 0)goto airspy_sym_error;
airspy_start_rx=(p_airspy_start_rx)dlsym(airspy_libhandle, 
                                                       "airspy_start_rx");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_freq=(p_airspy_set_freq)dlsym(airspy_libhandle, 
                                                       "airspy_set_freq");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_lna_gain=(p_airspy_set_lna_gain)dlsym(airspy_libhandle, 
                                                   "airspy_set_lna_gain");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_mixer_gain=(p_airspy_set_mixer_gain)dlsym(airspy_libhandle, 
                                                 "airspy_set_mixer_gain");
if(dlerror() != 0)goto airspy_sym_error;
airspy_set_vga_gain=(p_airspy_set_vga_gain)dlsym(airspy_libhandle, 
                                                  "airspy_set_vga_gain");
if(dlerror() != 0)goto airspy_sym_error;
airspy_r820t_write=(p_airspy_r820t_write)dlsym(airspy_libhandle, 
                                                    "airspy_r820t_write");
if(dlerror() != 0)goto airspy_sym_error;
airspy_library_flag=TRUE;
return;
airspy_sym_error:;
dlclose(airspy_libhandle);
airspy_library_flag=FALSE;
airspy_load_error:;
library_error_screen(AIRSPY_LIBNAME,info);
}

void unload_airspy_library(void)
{
if(!airspy_library_flag)return;
dlclose(airspy_libhandle);
airspy_library_flag=FALSE;
}


#endif


int airspy_callback(airspy_transfer_t* transfer)
{
int i;
short int *iz, *ix;
if(cancel_flag)return -1;
ix=transfer->samples;
#if AIRSPY_REAL == TRUE
for(i=0; i<transfer->sample_count; i++)
  {
  iz=(short int*)&timf1_char[timf1p_sdr];
  iz[0]=ix[i];
  timf1p_sdr=(timf1p_sdr+2)&timf1_bytemask;
  }                 
#else
for(i=0; i<transfer->sample_count; i++)
  {
  iz=(short int*)&timf1_char[timf1p_sdr];
  iz[0]=ix[2*i  ];
  iz[1]=ix[2*i+1];
  timf1p_sdr=(timf1p_sdr+4)&timf1_bytemask;
  }                 
#endif
if( ((timf1p_sdr-timf1p_pa+timf1_bytes)&timf1_bytemask) >= 
                                                     snd[RXAD].block_bytes)
  {
  lir_set_event(EVENT_HWARE1_RXREADY);
  }
return 0;
}

void airspy_starter(void)
{
//int i, j, k, no_of_buffers;
int res;
cancel_flag=FALSE;
while(thread_status_flag[THREAD_AIRSPY_INPUT]!=THRFLAG_ACTIVE)
  {
  lir_sleep(10000);
  if(kill_all_flag)return;
  }
thread_status_flag[THREAD_AIRSPY_STARTER]=THRFLAG_ACTIVE;
lir_sched_yield();      
res = airspy_start_rx(dev_airspy, airspy_callback, NULL);
if(res != AIRSPY_SUCCESS )lirerr(564989); 
}

uint8_t airspy_vga_gain0[]={13,12,11,11,11,11,11,10,10,10,10,10,10,10,10,10,
                          9,8,7,6,5,4};
uint8_t airspy_lna_gain0[]={14,14,14,13,12,10, 9, 9, 8, 9, 8, 6, 5, 3, 1, 0,
                          0,0,0,0,0,0};                           
uint8_t airspy_mix_gain0[]={12,12,11, 9, 8, 7, 6, 6, 5, 0, 0, 1, 0, 0, 2, 2,
                          1,1,1,1,0,0};
uint8_t airspy_vga_gain1[]={13,12,11,10, 9, 8, 7, 6, 5, 5, 5, 5, 5, 4, 4, 4,
                          4,4,4,4,4,4};
uint8_t airspy_lna_gain1[]={14,14,14,14,14,14,14,14,14,13,12,12, 9, 9, 8, 7,
                          6,5,3,2,1,0};                           
uint8_t airspy_mix_gain1[]={12,12,12,12,11,10,10, 9, 9, 8, 7, 4, 4, 4, 3, 2,
                          2,1,0,0,0,0};
                          

void set_airspy_att(void)
{
unsigned int gain_no=-fg.gain/4;
uint8_t vga_gain;
uint8_t lna_gain;
uint8_t mix_gain;
if(airspy.gain_mode == 0)
  {
  vga_gain=airspy_vga_gain0[gain_no];
  lna_gain=airspy_lna_gain0[gain_no];
  mix_gain=airspy_mix_gain0[gain_no];
  }
else
  {
  vga_gain=airspy_vga_gain1[gain_no];
  lna_gain=airspy_lna_gain1[gain_no];
  mix_gain=airspy_mix_gain1[gain_no];
  }
/*
{
char s[80];
sprintf(s,"vga=%d  lna=%d    mix=%d        ",vga_gain,lna_gain,mix_gain);
lir_text(0,0,s);
}
*/
airspy_set_vga_gain(dev_airspy, vga_gain);
airspy_set_mixer_gain(dev_airspy, mix_gain);
airspy_set_lna_gain(dev_airspy, lna_gain);
}

void set_airspy_frequency(void)
{
unsigned int freq_hz;
freq_hz=(int)(1000000.*fg.passband_center);
fg_truncation_error=freq_hz-1000000.*fg.passband_center;
airspy_set_freq(dev_airspy, freq_hz);
}

void airspy_input(void)
{
#if RUSAGE_OLD == TRUE
int local_workload_counter;
#endif
int i, no_of_airspy;
int rxin_local_workload_reset;
char s[80];
double dt1, read_start_time, total_reads;
int timing_loop_counter,timing_loop_counter_max,initial_skip_flag;
int local_att_counter;
int local_nco_counter;
int res, errcod;
float t1;
airspy_read_partid_serialno_t id;
struct airspy_device* devices[MAX_AIRSPY_DEVICES];
lir_init_event(EVENT_HWARE1_RXREADY);
#if OSNUM == OSNUM_LINUX
clear_thread_times(THREAD_AIRSPY_INPUT);
#endif
#if RUSAGE_OLD == TRUE
local_workload_counter=workload_counter;
#endif
local_att_counter=sdr_att_counter;
local_nco_counter=sdr_nco_counter;
dt1=current_time();
i=read_sdrpar(airspy_parfil_name, MAX_AIRSPY_PARM, 
                                  airspy_parm_text, (int*)((void*)&airspy));
errcod=1400;
if(i != 0 || 
   airspy.check != AIRSPY_PAR_VERNR ||
   airspy.bandwidth_no >= MAX_AIRSPY_BW)goto airspy_init_error_exit;
if(airspy.ratenum == 0)
  {
  t1=10000000.F;
  }
else
  {
  t1=2500000.F;
  }    
t1=((rint)((t1*(100000000L+(double)airspy.freq_adjust))/100000000L));
#if AIRSPY_REAL == TRUE
t1*=2.F;
#endif
if(ui.rx_ad_speed != (int)t1)
  {
  errcod=1401;
  goto airspy_init_error_exit;
  } 
timf1p_sdr=timf1p_pa;  
load_usb1_library(FALSE);
errcod=1402;
if(libusb1_library_flag == FALSE)goto airspy_init_error_exit;
load_airspy_library();
errcod=1403;
if(!airspy_library_flag)goto airspy_init_error_exit1;
errcod=23723;
no_of_airspy=0;
while(!kill_all_flag)
  {
  res=AIRSPY_SUCCESS;
  while( res == AIRSPY_SUCCESS && no_of_airspy < MAX_AIRSPY_DEVICES)  
    {
    devices[no_of_airspy]=NULL;
    res=airspy_open(&devices[no_of_airspy]);
    if(res == AIRSPY_SUCCESS)
      {
      res=airspy_board_partid_serialno_read(devices[no_of_airspy], &id);
      if(res == AIRSPY_SUCCESS)
        {
        if( airspy.sernum1 == id.serial_no[2] &&
            airspy.sernum2 == id.serial_no[3])
          {
          goto open_ok;
          }
        }
      }  
    else
      {
      no_of_airspy++;
      }
    }
  sprintf(s,"Waiting %.2f", current_time()-dt1);
  lir_text(3,5,s);
  lir_refresh_screen();
  if(kill_all_flag)goto airspy_init_error_exit2;
  lir_sleep(300000);
  for(i=0; i< no_of_airspy; i++)
    {
    if(devices[no_of_airspy] != NULL)airspy_close(devices[i]);
    }
  no_of_airspy=0;
  }
open_ok:;
if(no_of_airspy > 0)
  {
  for(i=0; i< no_of_airspy; i++)
    {
    if(devices[i] != NULL)airspy_close(devices[i]);
    }
  }  
dev_airspy=devices[no_of_airspy];
res=airspy_set_samplerate(dev_airspy, airspy.ratenum);
if(res != AIRSPY_SUCCESS)
  {  
  errcod=1404;
  goto airspy_error_exit1;
  }
res=airspy_set_rf_bias(dev_airspy, airspy.bias_t);
if(res != AIRSPY_SUCCESS)
  {  
  errcod=1405;
  goto airspy_error_exit1;
  }
#if AIRSPY_REAL == TRUE
res=airspy_set_sample_type(dev_airspy, AIRSPY_SAMPLE_INT16_REAL);
#else
res=airspy_set_sample_type(dev_airspy, AIRSPY_SAMPLE_INT16_IQ);
#endif
if(res != AIRSPY_SUCCESS)
  {  
  errcod=1406;
  goto airspy_error_exit1;
  }
set_hardware_rx_gain();
set_airspy_att();
set_airspy_frequency();
fft1_block_timing();
if(thread_command_flag[THREAD_SCREEN]!=THRFLAG_NOT_ACTIVE)
  {
  while(thread_status_flag[THREAD_SCREEN]!=THRFLAG_ACTIVE &&
        thread_status_flag[THREAD_SCREEN]!=THRFLAG_IDLE &&
        thread_status_flag[THREAD_SCREEN]!=THRFLAG_SEM_WAIT)
    {
    if(thread_command_flag[THREAD_AIRSPY_INPUT] ==
                                 THRFLAG_KILL)goto airspy_error_exit1;
    lir_sleep(10000);
    }
  }
#include "timing_setup.c"
linrad_thread_create(THREAD_AIRSPY_STARTER);
thread_status_flag[THREAD_AIRSPY_INPUT]=THRFLAG_ACTIVE;
lir_status=LIR_OK;
res=airspy_r820t_write(dev_airspy, 0x0a, reg_0xa[airspy.bandwidth_no]);
if(res != AIRSPY_SUCCESS)
  {  
  errcod=2311405;
  goto airspy_error_exit;
  }
res=airspy_r820t_write(dev_airspy, 0x0b, reg_0xb[airspy.bandwidth_no]);
if(res != AIRSPY_SUCCESS)
  {  
  errcod=2311405;
  goto airspy_error_exit;
  }
errcod=0;
while(!kill_all_flag && 
            thread_command_flag[THREAD_AIRSPY_INPUT] == THRFLAG_ACTIVE)
  {
#if RUSAGE_OLD == TRUE
  if(local_workload_counter != workload_counter)
    {
    local_workload_counter=workload_counter;
    make_thread_times(THREAD_AIRSPY_INPUT);
    }
#endif
  if(local_att_counter != sdr_att_counter)
    {
    local_att_counter=sdr_att_counter;
    set_airspy_att();
    }
  if(local_nco_counter != sdr_nco_counter)
    {
    local_nco_counter=sdr_nco_counter;
    set_airspy_frequency();
    }
  lir_await_event(EVENT_HWARE1_RXREADY);
  if(kill_all_flag)goto airspy_error_exit;
  while (!kill_all_flag && ((timf1p_pa-timf1p_sdr+timf1_bytes)&
                                  timf1_bytemask)>snd[RXAD].block_bytes)
    {
#include "input_speed.c"
    finish_rx_read();
    if(kill_all_flag)goto airspy_error_exit;
    }
  }
airspy_error_exit:;
cancel_flag=TRUE;
lir_join(THREAD_AIRSPY_STARTER);
airspy_error_exit1:;
airspy_close(dev_airspy);
airspy_init_error_exit2:;
unload_airspy_library();
airspy_init_error_exit1:;
unload_usb1_library();
airspy_init_error_exit:;
sdr=-1;
if(errcod != 0)lirerr(errcod);
thread_status_flag[THREAD_AIRSPY_INPUT]=THRFLAG_RETURNED;
while(thread_command_flag[THREAD_AIRSPY_INPUT] != THRFLAG_NOT_ACTIVE)
  {
  lir_sleep(1000);
  }
lir_close_event(EVENT_HWARE1_RXREADY);
} 

int display_airspy_parm_info(int *line)
{
char s[80];
char* sp;
int errcod;
float t1;
char *offon[2]={"OFF","ON"};
errcod=read_sdrpar(airspy_parfil_name, MAX_AIRSPY_PARM, 
                               airspy_parm_text, (int*)((void*)&airspy));
if(errcod == 0)
  {
  sprintf(s,"Serial no          = 0x%08X%08X",
                                         airspy.sernum1 ,airspy.sernum2);
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;
  if(airspy.ratenum == 0)
    {
    t1=10.F;
    }
  else
    {
    t1=2.5F;  
    }
  sprintf(s,"Sampling rate      = %.1f Mhz, Xtal adjust = %.0f ppb",t1,  
                                                   10.*airspy.freq_adjust);
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;
  if(airspy.gain_mode == 0)
    {
    sp="Linearity";
    }
  else
    {
    sp="Sensitivity";
    }
  sprintf(s,"Gain mode          = %s, Bandwidth = %.1f MHz",sp,
                                           airspy_bw[airspy.bandwidth_no]);
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  line[0]++;

  sprintf(s,"Bias T (5V)        = %s",offon[airspy.bias_t]);
  lir_text(24,line[0],s);
  SNDLOG"\n%s",s);
  }
return errcod;  
}

void init_airspy(void)
{
FILE *airspy_file;
char cind[4]={'-','/','|','\\'};
struct airspy_device* devices[MAX_AIRSPY_DEVICES];
uint32_t *rates;
int i, k, m, line, res;
int no_of_airspy, devno, devnum[MAX_AIRSPY_DEVICES];
char s[80];
char *ss;
int *sdr_pi;
uint32_t no_of_rates;
float nominal_freq;
airspy_read_partid_serialno_t id[MAX_AIRSPY_DEVICES];
clear_screen();
load_usb1_library(TRUE);
if(libusb1_library_flag == FALSE)return;
load_airspy_library();
if(airspy_library_flag == FALSE)return;
no_of_airspy=0;
line=5;
k=0;
res=AIRSPY_ERROR_NOT_FOUND;
while( (res != AIRSPY_SUCCESS && no_of_airspy == 0) || 
       (res == AIRSPY_SUCCESS && no_of_airspy < MAX_AIRSPY_DEVICES) )
  {
  res=airspy_open(&devices[no_of_airspy]);
  if(res == AIRSPY_SUCCESS)
    {
    no_of_airspy++;
    } 
  else
    {
    if(no_of_airspy == 0)
      {  
      sprintf(s,"%s  %c", airspy_error_name(res), cind[k]);
      lir_text(5,line,s);
      lir_refresh_screen();
      lir_sleep(100000);
      test_keyboard();
      if(kill_all_flag || lir_inkey != 0)
        {
        unload_usb1_library();
        return;
        }
      k++;
      k&=3;
      }
    }
  }
clear_lines(line,line);  
for(k=0; k<no_of_airspy; k++)
  {
  res=airspy_board_partid_serialno_read(devices[k], &id[k]);
  if(res != AIRSPY_SUCCESS) 
    {
    airspy_close(devices[k]);
    devices[k]=NULL;
    }
  }
m=0;
// Windows XP will open the same device multiple times so
// a second device will not be opened. Make sure we just close it once.
if( no_of_airspy > 1 && 
    id[0].serial_no[2] == id[1].serial_no[2] && 
    id[0].serial_no[3] == id[1].serial_no[3])
  {
  no_of_airspy=1;
  }   
for(k=0; k<no_of_airspy; k++)
  {
  if(devices[k])
    {
    
    sprintf(s,"%2d: serial no = 0x%08X%08X",m,id[k].serial_no[2],
                                              id[k].serial_no[3]);
    lir_text(3,line,s);
    line++;
    devnum[m]=k;
    m++;
    }
  }   
line++;
if(m == 0)
  {
  lir_text(5,line,"No usable Airspy device found");
errexit:;
  await_processed_keyboard();
  unload_airspy_library();
  unload_usb1_library();
  return;
  }
else 
  { 
  if(m == 1)
    {
    lir_text(5,line,"Airspy device autoselected");
    devno=devnum[0];
    }
  else
    {
    lir_text(5,line,"Select device by line number.");
    devno=devnum[lir_get_integer(34,line,2,0,m-1)];
    if(kill_all_flag)goto unload_lib;
    }
  }  
airspy.sernum1=id[devno].serial_no[2];
airspy.sernum2=id[devno].serial_no[3];
dev_airspy=devices[devno];
line+=2;
if(!airspy_get_samplerates)
  {
  lir_text(2,line,"Your Airspy library is old. If you have a modern firmware");
  line++;
#if OSNUM == OSNUM_WINDOWS
ss="airspy.dll";
#else
#if DARWIN == 1
ss="libairspy.dylib";
#else
ss="libairspy.so";
#endif
#endif
  sprintf(s,"that can support other sampling rates you should update your %s",ss);
  lir_text(2,line,s);
  line++;
  lir_text(2,line,"Select sampling rate H=10 MHz or L=2.5MHz");
get_rate:;
  await_processed_keyboard();
  if(kill_all_flag)goto unload_lib;;
  if(lir_inkey == 'H')
    {
    airspy.ratenum=0; 
    }
  else
    {
    if(lir_inkey != 'L')goto get_rate;
    airspy.ratenum=1; 
    }  
  if(airspy.ratenum == 0)
    {
    nominal_freq=10000000;
    }
  else
    {
    nominal_freq=2500000;  
    }
  }
else
  {
  airspy_get_samplerates(dev_airspy, &no_of_rates, 0);
  rates = (uint32_t *) malloc(no_of_rates * sizeof(uint32_t));
  airspy_get_samplerates(dev_airspy, rates, no_of_rates);
  for(i=0; i<(int)no_of_rates; i++)
    {
    sprintf(s,"%2d  %d Hz",i,rates[i]);
    lir_text(2,line,s);
    line++;
    }
  line++;
  lir_text(2,line,"Select rate by line number.");
  airspy.ratenum=lir_get_integer(30,line,2,0,(int)no_of_rates);
  nominal_freq=rates[airspy.ratenum];
  }
res=airspy_set_samplerate(dev_airspy, airspy.ratenum);
if(res != AIRSPY_SUCCESS)
  {  
  sprintf(s,"Failed to set sample rate: %s", airspy_error_name(res));
  lir_text(2,line,s);
errexit_close:;
  for(k=0; k<no_of_airspy; k++)
    {
    if(devices[k])
      {
      airspy_close(devices[k]);
      }
    }
  goto errexit;
  }
#if AIRSPY_REAL == TRUE
res=airspy_set_sample_type(dev_airspy, AIRSPY_SAMPLE_INT16_REAL);
#else
res=airspy_set_sample_type(dev_airspy, AIRSPY_SAMPLE_INT16_IQ);
#endif
if(res != AIRSPY_SUCCESS)
  {  
  sprintf(s,"Failed to set sample type: %s", airspy_error_name(res));
  lir_text(2,line,s);
  goto errexit_close;
  }
line+=2;
lir_text(2,line,"Enable bias T (Y/N)?");
get_bias_t:;
await_processed_keyboard();
if(kill_all_flag)goto unload_lib;
if(lir_inkey == 'Y')
  {
  airspy.bias_t=1; 
  lir_text(0,line+2,
      "WARNING!! You have selected to send 5V DC to the input connector");
  lir_text(0,line+3,"The current can be 0.5 A.");
  lir_text(0,line+4,"Do not connect signal generators etc.");
  lir_text(0,line+6,"Do you really want this? (Y/N)");
confirm:;
  await_processed_keyboard(); 
  if(kill_all_flag)goto unload_lib;
  if(lir_inkey == 'Y')
    {
    clear_lines(line+2,line+6);    
    }
  else
    {
    if(lir_inkey != 'N')goto confirm;
    clear_lines(line+2,line+6);    
    goto get_bias_t;
    }
  }    
else
  {
  if(lir_inkey != 'N')goto get_bias_t;
  airspy.bias_t=0; 
  }
airspy_set_rf_bias(dev_airspy, airspy.bias_t);
line+=2;
if(res != AIRSPY_SUCCESS)
  {  
  sprintf(s,"Failed to set bias T: %s", airspy_error_name(res));
  lir_text(2,line,s);
  goto errexit_close;
  }
for(i=0; i<MAX_AIRSPY_BW; i++)
  {
  if(i==6)i++;
  sprintf(s,"%c => %.1f MHz",'A'+(char)i,airspy_bw[i]);
  lir_text(5,line,s);
  line++;
  }
line++;
lir_text(2,line,"Select bandwidth from table.");  
getbw:;
await_processed_keyboard();
if(kill_all_flag)goto unload_lib;
i=lir_inkey-'A';
if(i>'G')i--;
if(i < 0)goto getbw;
if(i >= MAX_AIRSPY_BW)goto getbw;
airspy.bandwidth_no=(unsigned int)i;
clear_lines(line,line);
sprintf(s,"Selected bandwidth is %.1f MHz",airspy_bw[i]);
line +=2;
lir_text(2,line,s);
line +=2;
lir_text(3,line,"Select gain mode.");
line++;
lir_text(3,line,"0=Linearity.");
line++;
lir_text(3,line,"1=Sensitivity.");
line+=2;
lir_text(3,line,"=>");
airspy.gain_mode=lir_get_integer(6,line,1,0,1);
line+=2;
lir_text(3,line,"Enter xtal error in ppb =>");
airspy.freq_adjust=0.1*lir_get_float(32,line,9,-300000.,300000.);
if(kill_all_flag)goto unload_lib;
nominal_freq=((rint)((nominal_freq*(100000000L+(double)airspy.freq_adjust))/100000000L));
ui.rx_ad_speed=(int)nominal_freq;
#if AIRSPY_REAL == TRUE
ui.rx_ad_speed*=2;
#endif
airspy_file=fopen(airspy_parfil_name,"w");
if(airspy_file == NULL)
  {
  lirerr(382368);
  goto unload_lib;
  }
airspy.check=AIRSPY_PAR_VERNR;
sdr_pi=(int*)(&airspy);
for(i=0; i<MAX_AIRSPY_PARM; i++)
  {
  fprintf(airspy_file,"%s [%d]\n",airspy_parm_text[i],sdr_pi[i]);
  }
parfile_end(airspy_file);
ui.rx_addev_no=AIRSPY_DEVICE_CODE;
ui.rx_rf_channels=1;
#if AIRSPY_REAL == TRUE
ui.rx_input_mode=0;
ui.rx_ad_channels=1;
#else
ui.rx_input_mode=IQ_DATA;
ui.rx_ad_channels=2;
#endif
ui.rx_admode=0;
unload_lib:;
m=0;  
for(k=0; k<no_of_airspy; k++)
  {
  if(devices[k])
    {
    res=airspy_close(devices[k]);
    if(res != AIRSPY_SUCCESS)m++;
    }
  }
unload_airspy_library();     
unload_usb1_library();
if(m != 0)
  {  
  lirerr(1315);
  return;
  }
}
