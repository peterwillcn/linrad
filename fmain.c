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



// for Linux/svgalib.
// We set up the screen and mouse here and then we start
// the version independent main menu thread and the keyboard thread.
// We return here with lir_status=LIR_NEW_SCREEN in case the user
// wants a different screen resolution or LIR_UIPARMS if the
// user wants to change other system parameters.

//#define UNINIT_MEMDATA 0x57
#define UNINIT_MEMDATA 0
#define MAX_MOUSE_CURSIZE 50


#include <sys/io.h>
#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <termios.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <locale.h>


#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdarg.h>     
#include <linux/keyboard.h> /* to use KG_SHIFT and so on */



#include "globdef.h"
#include "lconf.h"
#include "conf.h"
#include "rusage.h"
#include "thrdef.h"
#include "fdef.h"
#include "uidef.h"
#include "screendef.h"
#include "vernr.h"
#include "options.h"
#include "keyboard_def.h"


typedef struct {
unsigned short int red;
unsigned short int green;
unsigned short int blue;unsigned int pixel;
short int flag;
float total;
}PIXINFO;

struct termios termios_pp;
int terminal_flag;
int gpm_fd;
struct fb_cmap *fb_palette;
int mouse_wheel_flag;

int main(int argc, char *argv[])
{
int i,ir,j,k;
int xxprint;
char *parinfo;
FILE *file;
int *uiparm;
(void) *argv;
(void) argc;
setlocale(LC_ALL,"POSIX");
#if UNINIT_MEMDATA != 0
  {
  parinfo=(char*)(&uninit_mem_begin);
  k=&uninit_mem_end-parinfo;
  for(i=0; i<k; i++)parinfo[i]=UNINIT_MEMDATA;
  }
#endif  
i=system("reset");
terminal_flag=FALSE;
fb_palette=NULL;
gpm_fd=-1;
parinfo=(char *)getenv("TERM");
if(strncmp(parinfo,"xterm",5) == 0)
  {
  printf("Use xlinrad under X11!!\n");
  exit(0);
  }
// put something nonzero in scratch memory to make sure we not rely on 
// it being zero when starting (this may help us detect bugs).
// Init svgalib and set it to text mode.
vga_simple_init();
// Permissions to write to the hardware have to be set
// before any new threads are started. Reason unknown........
init_os_independent_globals();
serport=-1;
// Allocate buffers for keyboard and mouse, then start their threads.
keyboard_buffer=malloc(KEYBOARD_BUFFER_SIZE*sizeof(int)+
                     4*(MAX_MOUSE_CURSIZE+1)*sizeof(char));
behind_mouse=(char*)keyboard_buffer+KEYBOARD_BUFFER_SIZE*sizeof(int);
// Set the flags that tell whether daughter processes are running.
#ifdef THREAD_TIMERS
for(i=0; i<THREAD_MAX; i++)
  {
  thread_workload[i]=0;
  thread_tottim1[i]=0;
  thread_cputim1[i]=0;
  thread_tottim2[i]=0;
  thread_cputim2[i]=0;
  }
#endif
lir_init_event(EVENT_KEYBOARD);
lir_init_event(EVENT_SYSCALL);
pthread_create(&thread_identifier_keyboard,NULL,(void*)thread_keyboard, NULL);
pthread_create(&thread_identifier_mouse,NULL,(void*)thread_mouse, NULL);
// Create a thread that will close the entire process in a controlled way
// in case lirerr() is called or the ESC key is pressed.
lir_init_event(EVENT_KILL_ALL);
pthread_create(&thread_identifier_kill_all,NULL,(void*)thread_kill_all, NULL);
uiparm=(int*)(&ui);
if(DUMPFILE)
  {
  dmp = fopen("dmp", "w");
  DEB"\n******************************\n");
  }
else
  {  
  dmp=NULL;
  }
os_flag=OS_FLAG_SVGALIB;
#if DARWIN == 0
xxprint=investigate_cpu();
#else
xxprint=0;
simd_present=0;
mmx_present=0;
no_of_processors=1;
#endif
file = fopen(userint_filename, "rb");
if (file == NULL)
  {
  printf("\n\nWELCOME TO LINRAD");
  printf("\nThis message is not an error, but an indication that setup");
  printf("\nhas not yet been done.");
  print_procerr(xxprint);
  printf("\nSetup file %s missing.",userint_filename);
full_setup:;
  for(i=0; i<MAX_UIPARM; i++) uiparm[i]=0;
  welcome_msg();
  to_upper_await_keyboard();
  if(lir_inkey != 'S' && lir_inkey != 'N' && lir_inkey != 'E') goto skip;
  if(lir_inkey == 'N')
    {
    ui.operator_skil=OPERATOR_SKIL_NEWCOMER;
    }
  else
    {
    if(lir_inkey == 'S')
      {
      ui.operator_skil=OPERATOR_SKIL_NORMAL;
      }
    else
      {
      ui.operator_skil=OPERATOR_SKIL_EXPERT;
      }
    }
  lin_global_uiparms(0);
  if(kill_all_flag) goto exitmain;
  uiparm_change_flag=TRUE;
  ui.rx_input_mode=-1;
  ui.tx_dadev_no=UNDEFINED_DEVICE_CODE;
  ui.rx_dadev_no=UNDEFINED_DEVICE_CODE;
  ui.rx_addev_no=UNDEFINED_DEVICE_CODE;
  ui.max_dma_rate=DEFAULT_MAX_DMA_RATE;
  ui.min_dma_rate=DEFAULT_MIN_DMA_RATE;
  ui.rx_soundcard_radio=RX_SOUNDCARD_RADIO_UNDEFINED;
  ui.converter_mhz=0;
  ui.converter_hz=0;
  }
else
  {
  parinfo=malloc(4096);
  if(parinfo == NULL)
    {
    lir_errcod=1078;
    goto exitmain;
    }
  for(i=0; i<4096; i++) parinfo[i]=0;
  ir=(int)fread(parinfo,1,4095,file);
  fclose(file);
  file=NULL;
  if(ir >= 4095)
    {    
    goto go_full_setup;
    }
  k=0;
  for(i=0; i<MAX_UIPARM; i++)
    {
    while(parinfo[k]==' ' ||
          parinfo[k]== '\n' )k++;
    j=0;
    while(parinfo[k]== uiparm_text[i][j])
      {
      k++;
      j++;
      } 
    if(uiparm_text[i][j] != 0)goto go_full_setup;
    while(parinfo[k]!='[' && k< ir)k++;
    sscanf(&parinfo[k],"[%d]",&uiparm[i]);
    while(parinfo[k]!='\n' && k< ir)k++;
    }
  if( ui.check != UI_VERNR ||
      ui.operator_skil < OPERATOR_SKIL_NEWCOMER ||
      ui.operator_skil > OPERATOR_SKIL_EXPERT ||
      ui.rx_soundcard_radio < RX_SOUNDCARD_RADIO_UNDEFINED ||
      ui.rx_soundcard_radio >= MAX_RX_SOUNDCARD_RADIO)
    {
go_full_setup:;    
    printf("\n\nSetup file %s has errors",userint_filename);
    parinfo=chk_free(parinfo);
    goto full_setup;
    }
  graphics_init();
  init_font(ui.font_scale);
  if(lir_errcod != 0)goto exitmain;
  uiparm_change_flag=FALSE;
  }
//extio_error=load_extio_library();  
lir_init_event(EVENT_MANAGE_EXTIO);
users_open_devices();
if(kill_all_flag) goto exitmain;
pthread_create(&thread_identifier_main_menu,NULL,
                                              (void*)thread_main_menu, NULL);
file=freopen( "stderr.log", "w", stderr );
usleep(50000);

lir_inkey=0;

pthread_join(thread_identifier_main_menu,0);
exitmain:;
//unload_extio_library();
if(terminal_flag)
  {
  clear_keyboard();
  tcsetattr(0, TCSANOW, &termios_pp);
  }
if(gpm_fd>=0) close(gpm_fd);
printf( "\e[?25h" );	// restore cursor visibility 
users_close_devices();
lir_close_serport();
pthread_cancel(thread_identifier_mouse);
pthread_join(thread_identifier_mouse,0);
lir_sleep(20000);
pthread_cancel(thread_identifier_keyboard);
pthread_join(thread_identifier_keyboard,0);
show_errmsg(0);
skip:;
printf("\nLeaving %s\n",PROGRAM_NAME);
if(dmp!=NULL)fclose(dmp);
munmap(mempix_char, framebuffer_screensize);
if(framebuffer_handle)close(framebuffer_handle);
if(fb_palette)free(fb_palette);
lir_close_event(EVENT_KILL_ALL);
lir_close_event(EVENT_SYSCALL);
lir_close_event(EVENT_KEYBOARD);
lir_close_event(EVENT_MANAGE_EXTIO);
return lir_errcod;
}


void graphics_init(void)
{
int i, k;
unsigned short int *ipalette;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
short unsigned int red[MAX_SVGA_PALETTE];
short unsigned int green[MAX_SVGA_PALETTE];
short unsigned int blue[MAX_SVGA_PALETTE];
short unsigned int transp[MAX_SVGA_PALETTE];
struct sockaddr_un addr;
struct MouseCaps caps;
struct termios termios_p;
framebuffer_screensize=0;
framebuffer_handle = open("/dev/fb0", O_RDWR);
if (!framebuffer_handle) 
  {
  lirerr(1349);
  return;
  }
if (ioctl(framebuffer_handle, FBIOGET_VSCREENINFO, &vinfo)) 
  {
  lirerr(1351);
  return;
  }
color_depth=vinfo.bits_per_pixel;
framebuffer_screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
mempix_char = mmap(0, framebuffer_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer_handle, 0);
#if IA64 == 0
if ((int)mempix_char == -1) 
#else
if ((long int)mempix_char == -1) 
#endif
  {
  lirerr(1355);
  return;
  }
color_depth=vinfo.bits_per_pixel;
printf("color_depth=%d",color_depth);
tcgetattr(0, &termios_pp);
memcpy(&termios_p, &termios_pp, sizeof(termios_p));
terminal_flag=TRUE;
termios_p.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                                      | INLCR | IGNCR | ICRNL | IXON);
termios_p.c_oflag &= ~OPOST;
termios_p.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
termios_p.c_cflag &= ~(CSIZE | PARENB);
termios_p.c_cflag |= CS8;
tcsetattr(0, TCSANOW, &termios_p);
printf( "\e[?25l\n " ); // hide the cursor
// Open gpmctl. This will hide the gpm cursor.
gpm_fd=socket(AF_UNIX,SOCK_STREAM,0);
if(gpm_fd >= 0)
  {
  memset(&addr,0,sizeof(addr));
  addr.sun_family=AF_UNIX;
  strcpy(addr.sun_path, GPMCTL);
  i=sizeof(addr.sun_family)+strlen(GPMCTL);
  if(connect(gpm_fd,(struct sockaddr *)(&addr),i)<0 ) 
    {
    close(gpm_fd);
    gpm_fd=open(GPMCTL,O_RDWR);
    }
  }  




screen_width=vinfo.xres;
screen_height=vinfo.yres;
screen_totpix=screen_width*screen_height;
i=vga_getmousetype();
if(i == MOUSE_NONE)
  {
  lirerr(1354);
  return;
  }

mouse_init("/dev/mouse",i,MOUSE_DEFAULTSAMPLERATE);
mouse_setdefaulteventhandler();
mouse_setxrange(0, screen_width-1);
mouse_setyrange(0, screen_height-1);
mouse_setwrap(MOUSE_NOWRAP);
lir_status=LIR_OK;
// Set up mouse and system parameters in case it has not been done already.
mouse_cursize=screen_height/80;
if(mouse_cursize > MAX_MOUSE_CURSIZE)mouse_cursize=MAX_MOUSE_CURSIZE;
if(ui.mouse_speed < 1)ui.mouse_speed=1;
if(ui.mouse_speed > 999)ui.mouse_speed=999;
mouse_setscale(ui.mouse_speed);
mouse_x=screen_width/2;
mouse_y=screen_height/2;
new_mouse_x=screen_width/2;
new_mouse_y=screen_height/2;
mouse_setposition( mouse_x, mouse_y);
if(mouse_getcaps(&caps)) 
  {
// Failed! Old library version... Check the mouse type.
  i=vga_getmousetype() & MOUSE_TYPE_MASK;
  if(i == MOUSE_INTELLIMOUSE || i==MOUSE_IMPS2)
    {
    mouse_wheel_flag=1;
    }
  else
    {
    mouse_wheel_flag=0;
    }
  }
else
  {
// If this is a mouse_wheel_flag mouse, interpret rx as a wheel
  mouse_wheel_flag = ((caps.info & MOUSE_INFO_WHEEL) != 0);
  }
if(mouse_wheel_flag)
  {
  mouse_setrange_6d(0,0, 0,0, 0, 0, -180,180, 0,0, 0,0, MOUSE_RXDIM);
  }




if(screen_width < 640 || screen_height < 480)
  {
  lirerr(1242);
  return;
  }
switch (color_depth)
  {
  case 32:
  case 24:
  for(i=0; i<screen_totpix*4; i++)mempix_char[i]=0;
// ******************************************************************
// Rearrange the palette. It was designed for svgalib under Linux
  for(i=0; i<3*MAX_SVGA_PALETTE; i++)
    {
    svga_palette[i]=(unsigned char)(svga_palette[i]<<2);
    if(svga_palette[i] != 0) svga_palette[i]|=3;
    }
  break;
  
  case 16:
  mempix_shi=(unsigned short int*)mempix_char;
  for(i=0; i<screen_totpix; i++)mempix_shi[i]=0;
// ******************************************************************
  ipalette=(unsigned short int*)(&svga_palette[0]);
  for(i=0; i<256; i++)
    {
    k=svga_palette[3*i+2];
    k&=0xfffe;
    k<<=5;
    k|=svga_palette[3*i+1];
    k&=0xfffe;
    k<<=6;
    k|=svga_palette[3*i  ];
    k>>=1;
    ipalette[i]=(short unsigned int)k;
    }
  break;

  
  case 8:
  if (ioctl(framebuffer_handle, FBIOGET_FSCREENINFO, &finfo)) 
    {
    lirerr(1350);
    return;
    }
  fb_palette=malloc(256*sizeof(fb_palette));
  if(finfo.visual == FB_VISUAL_DIRECTCOLOR) 
    {
    if (ioctl(framebuffer_handle, FBIOGETCMAP , &fb_palette)) 
      {
      free(fb_palette);
      lirerr(1352);
      return;
      }
    }
  fb_palette->start=0;
  fb_palette->len=MAX_SVGA_PALETTE;
  for(i=0; i<MAX_SVGA_PALETTE; i++)
    {
    red[i]=(short unsigned int)(svga_palette[3*i+2]);
    green[i]=(short unsigned int)(svga_palette[3*i+1]);
    blue[i]=(short unsigned int)(svga_palette[3*i  ]);
    transp[i]=0;
    }
  fb_palette->red=red;  
  fb_palette->green=green;  
  fb_palette->blue=blue;  
  fb_palette->transp=transp;  

  vinfo.activate=FB_ACTIVATE_ALL|FB_CHANGE_CMAP_VBL;
  ioctl(framebuffer_handle, FBIOPUT_VSCREENINFO, &vinfo);

  i=ioctl(framebuffer_handle, FBIOPUTCMAP , &fb_palette); 


printf("\nioctl FBIOPUTCMAP returned %d\n",i);



/*
  defpix=(PIXINFO*)malloc(sizeof(PIXINFO)*256);
  lirpix=(PIXINFO*)malloc(sizeof(PIXINFO)*256);
  pixdiff=(float*)malloc(256*256*sizeof(float));
  lir_colormap=XCreateColormap(xdis, xwin, visual, AllocAll);
  default_colormap = DefaultColormap(xdis, screen_num);
// Store the default colormap in defpix
  for(id=0; id<256; id++)
    {
    xco.pixel=(long unsigned int)id;
    k=XQueryColor (xdis,default_colormap,&xco);  
    defpix[id].red=xco.red;
    defpix[id].green=xco.green;
    defpix[id].blue=xco.blue;
    defpix[id].flag=0;
    defpix[id].pixel=(unsigned int)id;
    defpix[id].total=(float)defpix[id].red*(float)defpix[id].red+
                     (float)defpix[id].green*(float)defpix[id].green+
                     (float)defpix[id].blue*(float)defpix[id].blue;
    }
// svga_palette uses the six lowest bits for the colour intensities.
// shift left by 10 to move our data to occupy the six highest bits.
// Store the svgalib palette.
  for(il=0; il<MAX_SVGA_PALETTE; il++)
    {
    lirpix[il].red=(short unsigned int)(svga_palette[3*il+2]<<2);
    lirpix[il].green=(short unsigned int)(svga_palette[3*il+1]<<2);
    lirpix[il].blue=(short unsigned int)(svga_palette[3*il  ]<<2);
    if(lirpix[il].red != 0)lirpix[il].red|=3;
    if(lirpix[il].green != 0)lirpix[il].green|=3;
    if(lirpix[il].blue != 0)lirpix[il].blue|=3;
    lirpix[il].red=(short unsigned int)(lirpix[il].red<<8);
    lirpix[il].green=(short unsigned int)(lirpix[il].green<<8);
    lirpix[il].blue=(short unsigned int)(lirpix[il].blue<<8);
    lirpix[il].pixel=(unsigned int)il;
    lirpix[il].flag=1;
    lirpix[il].total=(float)lirpix[il].red*(float)lirpix[il].red+
                     (float)lirpix[il].green*(float)lirpix[il].green+
                     (float)lirpix[il].blue*(float)lirpix[il].blue;
    }
  for(il=MAX_SVGA_PALETTE; il<256; il++)
    {
    lirpix[il].red=0;
    lirpix[il].green=0;
    lirpix[il].blue=0;
    lirpix[il].pixel=(unsigned int)il;
    lirpix[il].flag=0;
    }
#define M 0.00000001
#define N 0x100
// Sort lirpix in order of ascending total intensity.
  for(il=0; il<MAX_SVGA_PALETTE-1; il++)
    {
    t1=0;
    m=il;
    for(kl=il; kl<MAX_SVGA_PALETTE; kl++)
      {
      if(lirpix[kl].total > t1)
        {
        t1=lirpix[kl].total;
        m=kl;
        }
      }  
    tmppix=lirpix[il];
    lirpix[il]=lirpix[m];
    lirpix[m]=tmppix;
    }
// Compute the similarity between lirpix and defpix and store in a matrix.
  for(il=0; il<MAX_SVGA_PALETTE; il++)
    {
    for(id=0; id<256; id++)
      {
      t2=(float)(pow((float)(lirpix[il].red-defpix[id].red),2.0)+
                 pow((float)(lirpix[il].green-defpix[id].green),2.0)+
                 pow((float)(lirpix[il].blue-defpix[id].blue),2.0));
      pixdiff[id+il*256]=t2;
      }
    }  
// Reorder the default colormap for the diagonal elements
// of pixdiff (up to MAX_SVGA_PALETTE-1) to become as small
// as possible when stepping in the ascending order that
// lirpix currently is sorted in.
  for(il=0; il<MAX_SVGA_PALETTE; il++)
    {
    t1=(float)BIG;
    kd=0;
    for(id=0; id<256; id++)
      {
      if(pixdiff[id+il*256] < t1)
        {
        t1=pixdiff[id+il*256];
        kd=id;
        }
      }
    tmppix=defpix[il];
    defpix[il]=defpix[kd];
    defpix[kd]=tmppix;
    for(kl=0; kl<MAX_SVGA_PALETTE; kl++)
      {
      t1=pixdiff[kd+kl*256];
      pixdiff[kd+kl*256]=pixdiff[il+kl*256];
      pixdiff[il+kl*256]=t1;
      }
    }
  for(i=0; i<MAX_SVGA_PALETTE; i++)
    {
    xco.pixel=defpix[i].pixel;
    xco.red=lirpix[i].red;
    xco.green=lirpix[i].green;
    xco.blue=lirpix[i].blue;
    xco.flags=DoRed|DoGreen|DoBlue;
    xco.pad=0;
    k=XStoreColor(xdis, lir_colormap, &xco);  
    if(k==0)
      {
      printf("\nPalette failed\n");
      goto exitmain;
      }
    xpalette[lirpix[i].pixel]=(unsigned char)xco.pixel;
    }
  for(i=MAX_SVGA_PALETTE; i<256; i++)
    {
    xco.pixel=defpix[i].pixel;
    xco.red=defpix[i].red;
    xco.green=defpix[i].green;
    xco.blue=defpix[i].blue;
    xco.flags=DoRed|DoGreen|DoBlue;
    xco.pad=0;
    k=XStoreColor(xdis, lir_colormap, &xco);  
    if(k==0)
      {
      printf("\nPalette failed\n");
      goto exitmain;
      }
    xpalette[i]=(unsigned char)lirpix[i].pixel;
    }
  XSetWindowColormap(xdis, xwin, lir_colormap);
  free(defpix);
  free(lirpix);
  free(pixdiff);
*/
  break;


  default:
  printf("\nUnknown color depth: %d\n",color_depth);
  lirerr(1353);
  return;
  } 
}



int force_getchar(void)
{
// Get a character from keyboard, but set a 0.1 s timeout
// and return -1 if no key was pressed.
struct termios term, oterm;
int c = 0;
// get the terminal settings 
tcgetattr(0, &oterm);
// get a copy of the settings, which we modify 
memcpy(&term, &oterm, sizeof(term));
// put the terminal in non-canonical mode, any 
// reads timeout after 0.1 seconds or when a 
// single character is read
term.c_lflag = term.c_lflag & (!ICANON);
term.c_cc[VMIN] = 0;
term.c_cc[VTIME] = 1;
tcsetattr(0, TCSANOW, &term);
// get input - timeout after 0.1 seconds or
// when one character is read. If timed out
// getchar() returns -1, otherwise it returns
// the character
c=getchar();
//   reset the terminal to original state 
tcsetattr(0, TCSANOW, &oterm);
return c;
}


void thread_keyboard(void)
{
int c, i, k;
char s[80];
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&i);
while(1)
  {
// It is silly to use force_getchar here. 
// We should set up stdin so it will return on each char of an ESC sequence.
// so we would not have to loop around at 10 Hz here.
  c=getchar();
  keyboard_buffer[keyboard_buffer_ptr]=c;
  if(c == 27)
    {
// Set the flag and wait a while to make it unlikely
// it will not be visible for the kill_all thread.
// We must not kill the keyboard thread while force_getchar() is
// executed because the changed terminal setting will survive
// after Linrad is finished and make the command line behave
// incorrectly.
      {
      c=force_getchar();
      if(c == -1)
        {
// The ESC key was pressed. Exit from Linrad NOW!    
        DEB"\nESC pressed");
        if(ui.operator_skil == OPERATOR_SKIL_NEWCOMER && !kill_all_flag)
          {
          newcomer_escpress(0);
newco_esc:;          
          while(c != 'Y' && c != 'N')c=to_upper(getchar());
          if(c=='N')
            {
            while(c!=-1)c=force_getchar();
            newcomer_escpress(1);
            goto next;
            }
          if(c != 'Y')goto newco_esc;  
          }
        lir_set_event(EVENT_KILL_ALL);
        while(!kill_all_flag)lir_sleep(3000);
check_threads:;
        k=0; 
        for(i=0; i<THREAD_MAX; i++)    
          {
          if(i != THREAD_SYSCALL && 
             thread_command_flag[i] != THRFLAG_NOT_ACTIVE &&
             thread_waitsem[i] == -2)
            {
            k++;
            }   
          }
        if(k != 0)
          {
          lir_sleep(30000);
          goto check_threads;
          }  
        keyboard_buffer[keyboard_buffer_ptr]=0;
        keyboard_buffer_ptr=(keyboard_buffer_ptr+1)&
                                                  (KEYBOARD_BUFFER_SIZE-1);
        lir_sleep(10000);  
        lir_set_event(EVENT_KEYBOARD);
        while(1)
          {
          lir_sleep(30000);
          }
        return;
        }    
// The ESC sequences are all [27],[91],.....
      if(c == 91)
        {
        c=force_getchar();
// Arrow keys are:
// ARROW_UP=65
// ARROW_DOWN=66
// ARROW_RIGHT=67
// ARROW_LEFT=68
        if(c >= 65 && c <= 68)
          {
          c+=ARROW_UP_KEY-65;
          goto fkn_key_ok;
          }
        if(c >= 49 && c <= 54)
          {
          i=force_getchar();
// These keys have to be followed by 126
// HOME=49
// INSERT=50
// DELETE=51
// END=52
// PAGE_UP=53
// PAGE_DOWN=54
          if(i == 126)
            {
            c+=HOME_KEY-49;
            goto fkn_key_ok;
            }
          k=force_getchar();
          if(k == 126)
            {  
            if(c == 49)
              {
// F6=55
// F7=56
// F8=57
              if(i >= 55 && i <= 57)
                {
                c=i+F6_KEY-55;
                goto fkn_key_ok;
                }
              }  
            if(c == 50)
              {
// F9=48
// F10=49
// F10_PAD=50
// F11=51
// F12=52
// SHIFT_F1=53
// SHIFT_F2=54
// SHIFT_F2_PAD=55
// SHIFT_F3=56
// SHIFT_F4=57
              if(i >= 48 && i <= 57)
                { 
                c=i+F9_KEY-48;
                goto fkn_key_ok;
                }
              }    
            if(c == 51)
              {
// SHIFT_F5=49
// SHIFT_F6=50
// SHIFT_F7=51
// SHIFT_F8=52
             if(i >= 49 && i <= 52)
                { 
                c=i+SHIFT_F5_KEY-49;
                goto fkn_key_ok;
                }
              }
            }    
          sprintf(s,"ESC sequence):[27][91][%d][%d][%d]",c,i,k);
          goto skip;
          }
// PAUSE = 80
        if(c == 80)
          {
          c=PAUSE_KEY;
          goto fkn_key_ok;
          }
        if(c == 91)
          {

          c=force_getchar();
// F1=65
// F2=66
// F3=67
// F4=68
// F5=69
          if(c >= 65 && c <= 69)
            {
            c+=F1_KEY-65;
            goto fkn_key_ok;
            }
          sprintf(s,"ESC sequence:[27][91][91][%d]",c);
          goto skip;
          }
        sprintf(s,"ESC sequence:[27][91][%d]",c);
        goto skip;
        }
      if(c >= 'a' && c <= 'z')c=toupper(c);  
      if(c >= 'A' && c <= 'Z')
        {
        c=c-'A'+ALT_A_KEY;
        goto fkn_key_ok;
        }        
// ESC followed by something else than 91 or ALT keys)
      sprintf(s,"ESC sequence:[27][%d]",c);
skip:;
      lir_text(1,5,s);
      c=force_getchar();
      if(c != -1)
        {
        i=0;
        while(s[i]!=0 && i<70)i++;
        sprintf(&s[i],"[%d]",c);
        goto skip;
        }
      if(c == -1) goto next;
fkn_key_ok:;
      keyboard_buffer[keyboard_buffer_ptr]=c;
      }
    }
  keyboard_buffer_ptr=(keyboard_buffer_ptr+1)&(KEYBOARD_BUFFER_SIZE-1);
  lir_sleep(10000);  
  lir_set_event(EVENT_KEYBOARD);
next:;
  }
}

void thread_mouse(void)
{
int i, m, button, rx;
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&i);
while(TRUE)
  {
// **********************************************
// Wait for svgalib to report a mouse event
  mouse_waitforupdate();
// We do nothing unless mouse_flag is set.
  if(mouse_flag != 0)
    {
    button= mouse_getbutton();
    mouse_getposition_6d(&new_mouse_x, &new_mouse_y, NULL, &rx, NULL, NULL);
    if(rx!=0)
      {
      if (button & MOUSE_MIDDLEBUTTON) 
        {
        m=bg.wheel_stepn;
        if(rx > 0)
          {
          m++;
          if(m>30)m=30;
          }
        else
          {
          m--;
          if(m<-30)m=-30;
          if(genparm[AFC_ENABLE]==0 && m<0)m=0;
          }
        bg.wheel_stepn=m;
        sc[SC_SHOW_WHEEL]++;
        make_modepar_file(GRAPHTYPE_BG);
        }
      else
        {
        if(rx > 0)
          {
          step_rx_frequency(1);
          }
        else
          {
          step_rx_frequency(-1);
          }  
        }
      }
    if (button & MOUSE_LEFTBUTTON) 
      {
      new_lbutton_state=1;
      }
    else
      {
      new_lbutton_state=0;
      }
    if (button & MOUSE_RIGHTBUTTON) 
      {
      new_rbutton_state=1;
      }
    else
      {
      new_rbutton_state=0;
      }
    if( thread_status_flag[THREAD_SCREEN] == THRFLAG_SEM_WAIT ||
        thread_status_flag[THREAD_SCREEN] == THRFLAG_ACTIVE ||
        thread_status_flag[THREAD_SCREEN] == THRFLAG_IDLE) 
      {
      if( new_mouse_x!=mouse_x || new_mouse_y!=mouse_y)
        {
        awake_screen();
        }
      check_mouse_actions();
      }
    }
  }
}

