
extern int  X11_accesstype;
#define X11_MIT_SHM      0X01 /* Use Xlib shared memory extension */
#define X11_STANDARD     0x00 /* Use standard Xlib calls */

extern unsigned char *mempix_char;
extern unsigned short int *mempix_shi;
extern int first_mempix;
extern int last_mempix;
extern pthread_t thread_identifier_process_event;
extern pthread_t thread_identifier_refresh_screen;

extern int process_event_flag;
extern int expose_event_done;
extern int shift_key_status;
extern int alt_key_status;
extern int color_depth;
extern unsigned char *xpalette;
void ui_setup(void);
void thread_process_event(void);
void thread_refresh_screen(void);
void lir_remove_mouse_thread(void);
void store_in_kbdbuf(int c);
void wxmouse(void);
