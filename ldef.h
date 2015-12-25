

extern pthread_mutex_t linux_mutex[MAX_LIRMUTEX];
extern int uninit_mem_end;
extern int uninit_mem_begin;
#if HAVE_OSS == 1
extern audio_buf_info rx_da_info;
extern audio_buf_info tx_da_info;
extern audio_buf_info tx_ad_info;
#endif
extern pthread_t thread_identifier_kill_all;
extern pthread_t thread_identifier_keyboard;
extern pthread_t thread_identifier_mouse;
extern pthread_t thread_identifier_main_menu;
extern pthread_t thread_identifier_html_server;
extern char *behind_mouse;
extern pthread_t thread_identifier[THREAD_MAX];
extern pthread_mutex_t lir_event_mutex[MAX_LIREVENT];
extern int lir_event_flag[MAX_LIREVENT];
extern pthread_cond_t lir_event_cond[MAX_LIREVENT];

extern ROUTINE thread_routine[THREAD_MAX];
extern int serport;


void mmxerr(void);
void graphics_init(void);
void remove_keyboard_and_mouse(void);
void thread_main_menu(void);
void thread_rx_adinput(void);
void thread_rx_raw_netinput(void);
void thread_rx_fft1_netinput(void);
void thread_rx_file_input(void);
void thread_rx_output(void);
void thread_screen(void);
void thread_tx_input(void);
void thread_tx_output(void);
void thread_wideband_dsp(void);
void thread_second_fft(void);
void thread_timf2(void);
void thread_narrowband_dsp(void);
void thread_user_command(void);
void thread_txtest(void);
void thread_powtim(void);
void thread_rx_adtest(void);
void thread_cal_interval(void);
void thread_cal_filtercorr(void);
void thread_sdr14_input(void);
void thread_tune(void);
void thread_kill_all(void);
void thread_keyboard(void);
void thread_mouse(void);
void thread_lir_server(void);
void thread_perseus_input(void);
void thread_radar(void);
void thread_blocking_rxout(void);
void thread_syscall(void);
void thread_sdrip_input(void);
void thread_hware_command(void);
void thread_excalibur_input(void);
void thread_extio_input(void);
void thread_write_raw_file(void);
void thread_rtl2832_input(void);
void thread_rtl_starter(void);
void thread_mirics_input(void);
void thread_bladerf_input(void);
void thread_pcie9842_input(void);
void thread_openhpsdr_input(void);
void thread_mirisdr_starter(void);
void thread_bladerf_starter(void);
void thread_html_server(void);
void thread_netafedri_input(void);
void thread_do_fft1c(void);
void thread_fft1b(void);
void thread_fdms1_input(void);
void thread_fdms1_starter(void);
void thread_airspy_input(void);
void thread_airspy_starter(void);
void thread_tx_hand_key(void);

int investigate_cpu(void);
void print_procerr(int xxprint);
