

#define THRFLAG_NOT_ACTIVE 0
#define THRFLAG_INIT 1
#define THRFLAG_ACTIVE 2
#define THRFLAG_IDLE 4
#define THRFLAG_RESET 5
#define THRFLAG_SEM_WAIT 7
#define THRFLAG_AWAIT_INPUT 10
#define THRFLAG_SEMCLEAR 11
#define THRFLAG_RETURNED 12
#define THRFLAG_INPUT_WAIT 13

#define THRFLAG_PORTAUDIO_STARTSTOP 100
#define THRFLAG_OPEN_RX_SNDIN 101
#define THRFLAG_CLOSE_RX_SNDIN 102
#define THRFLAG_OPEN_RX_SNDOUT 103
#define THRFLAG_CLOSE_RX_SNDOUT 104
#define THRFLAG_OPEN_TX_SNDIN 105
#define THRFLAG_CLOSE_TX_SNDIN 106
#define THRFLAG_OPEN_TX_SNDOUT 107
#define THRFLAG_CLOSE_TX_SNDOUT 108
#define THRFLAG_SET_RX_IO 109
#define THRFLAG_TX_SETUP 110
#define THRFLAG_PORTAUDIO_STOP 111
#define THRFLAG_SET_SDRIP_ATT 112
#define THRFLAG_SET_SDRIP_FREQUENCY 113
#define THRFLAG_CALIBRATE_BLADERF_RX 114
#define THRFLAG_SET_NETAFEDRI_ATT 115
#define THRFLAG_SET_NETAFEDRI_FREQUENCY 116
#define THRFLAG_KILL -1

#define THREAD_RX_ADINPUT 0
#define THREAD_RX_RAW_NETINPUT 1
#define THREAD_RX_FFT1_NETINPUT 2
#define THREAD_RX_FILE_INPUT 3
#define THREAD_SDR14_INPUT 4
#define THREAD_RX_OUTPUT 5
#define THREAD_SCREEN 6
#define THREAD_TX_INPUT 7
#define THREAD_TX_OUTPUT 8
#define THREAD_WIDEBAND_DSP 9
#define THREAD_NARROWBAND_DSP 10
#define THREAD_USER_COMMAND 11
#define THREAD_TXTEST 12
#define THREAD_POWTIM 13
#define THREAD_RX_ADTEST 14
#define THREAD_CAL_IQBALANCE 15
#define THREAD_CAL_INTERVAL 16
#define THREAD_CAL_FILTERCORR 17
#define THREAD_TUNE 18
#define THREAD_LIR_SERVER 19
#define THREAD_PERSEUS_INPUT 20
#define THREAD_RADAR 21
#define THREAD_SECOND_FFT 22
#define THREAD_TIMF2 23
#define THREAD_BLOCKING_RXOUT 24
#define THREAD_SYSCALL 25
#define THREAD_SDRIP_INPUT 26
#define THREAD_EXCALIBUR_INPUT 27
#define THREAD_HWARE_COMMAND 28
#define THREAD_EXTIO_INPUT 29
#define THREAD_WRITE_RAW_FILE 30
#define THREAD_RTL2832_INPUT 31
#define THREAD_RTL_STARTER 32
#define THREAD_MIRISDR_INPUT 33
#define THREAD_BLADERF_INPUT 34
#define THREAD_PCIE9842_INPUT 35
#define THREAD_OPENHPSDR_INPUT 36
#define THREAD_MIRISDR_STARTER 37
#define THREAD_BLADERF_STARTER 38
#define THREAD_NETAFEDRI_INPUT 39
#define THREAD_DO_FFT1C 40
#define THREAD_FFT1B1 41
#define THREAD_FFT1B2 42
#define THREAD_FFT1B3 43
#define THREAD_FFT1B4 44
#define THREAD_FFT1B5 45
#define THREAD_FFT1B6 46
#define THREAD_FDMS1_INPUT 47
#define THREAD_FDMS1_STARTER 48
#define THREAD_AIRSPY_INPUT 49
#define THREAD_AIRSPY_STARTER 50
#define THREAD_TX_HAND_KEY 51
#define THREAD_MAX 52

#define MAX_FFT1_THREADS (THREAD_FFT1B6-THREAD_FFT1B1+1)

extern signed char thread_command_flag[THREAD_MAX];
extern signed char thread_status_flag[THREAD_MAX];
extern int thread_waitsem[THREAD_MAX];
extern char mouse_thread_flag;
extern float thread_workload[THREAD_MAX];
extern double thread_tottim1[THREAD_MAX];
extern double thread_cputim1[THREAD_MAX];
extern double thread_tottim2[THREAD_MAX];
extern double thread_cputim2[THREAD_MAX];
#if RUSAGE_OLD != TRUE
extern int thread_pid[THREAD_MAX];
#endif

extern char *thread_names[THREAD_MAX];
extern int rx_input_thread;
extern int threads_running;
extern int ampinfo_reset;


#define EVENT_TIMF1 0
#define EVENT_FFT1_READY 1
#define EVENT_RX_START_DA 2
#define EVENT_BASEB 3
#define EVENT_TIMF2 4
#define EVENT_FFT2 5
#define EVENT_SCREEN 6
#define EVENT_PERSEUS_INPUT 7
#define EVENT_DO_FFT1C 8
#define EVENT_DO_FFT1B1 9
#define EVENT_DO_FFT1B2 10
#define EVENT_DO_FFT1B3 11
#define EVENT_DO_FFT1B4 12
#define EVENT_DO_FFT1B5 13
#define EVENT_DO_FFT1B6 14
#define EVENT_FDMS1_INPUT 15
//  - - - - - - - - - - - - -
#define EVENT_BLOCKING_RXOUT 16
#define EVENT_PORTAUDIO_RXAD_READY 17
#define EVENT_WRITE_RAW_FILE 18
#define EVENT_SYSCALL 19
#define EVENT_KILL_ALL 20
#define EVENT_MOUSE 21
#define EVENT_TX_INPUT 22
#define EVENT_KEYBOARD 23
#define EVENT_EXTIO_RXREADY 24
#define EVENT_REFRESH_SCREEN 25
#define EVENT_HWARE1_RXREADY 26
#define EVENT_MANAGE_EXTIO 27
#define EVENT_PORTAUDIO_TXAD_READY 28
#define MAX_LIREVENT 29
#define EVENT_AUTOINIT_MAX EVENT_BLOCKING_RXOUT


#define MUTEX_PARPORT 0
#define MUTEX_FFT1SLOWSUM 1
#define MUTEX_LIBUSB 2
#define MAX_LIRMUTEX 3

void linrad_thread_create(int no);
void linrad_thread_stop_and_join(int no);

#if RUSAGE_OLD == TRUE
double lir_get_thread_time(void);
#else
double lir_get_thread_time(int no);
#endif
double lir_get_cpu_time(void);

