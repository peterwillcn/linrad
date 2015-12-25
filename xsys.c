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


#include <semaphore.h>
#include <pthread.h>
#include <ctype.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <string.h>
#include "globdef.h"
#include "rusage.h"
#include "thrdef.h"
#include "uidef.h"
#include "screendef.h"
#include "lconf.h"
#include "xdef.h"
#include "keyboard_def.h"
#include "vernr.h"
#include "sdrdef.h"
#include "options.h"

#if SHM_INSTALLED == 1
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif 

extern GC xgc;
extern XImage *ximage;
extern Display *xdis;
extern Window xwin;
extern Colormap lir_colormap;


typedef struct {
unsigned short int red;
unsigned short int green;
unsigned short int blue;
unsigned int pixel;
short int flag;
float total;
}PIXINFO;

int first_mempix_copy;
int last_mempix_copy;
char first_mempix_flag;
char last_mempix_flag;

void lir_fix_bug(int bug)
{
XEvent ev;
switch (bug)
  {
  case 0:
  return;
  
  case 1:
  ev.type = Expose;
  ev.xexpose.display = xdis;
  ev.xexpose.window = xwin;
  ev.xexpose.send_event = FALSE;
  ev.xexpose.count=256;
  ev.xexpose.x = 16;
  ev.xexpose.y = 16;
  ev.xexpose.width = 16;
  ev.xexpose.height = 16;
  ev.xexpose.serial = 0;
  if(X11_accesstype==X11_STANDARD)
    {
    XSendEvent(xdis, xwin, FALSE, ExposureMask, &ev);
    }
#if SHM_INSTALLED == 1
  else
    {
    XSendEvent(xdis, xwin, FALSE, ExposureMask, &ev);
    }
#endif
  return;

  default:
  return;
  }
}

void lir_set_title(char *s)
{
char *window_name;
char wn[80];
XTextProperty window_title_property;
// Set the name of our window
// This is the string to be translated into a property. 
// translate the given string into an X property.
#if IA64 == 0
sprintf(wn," 32bit");
#else
sprintf(wn," 64bit");
#endif
if(X11_accesstype==X11_STANDARD)
  {
  sprintf(&wn[6]," %s %s",PROGRAM_NAME,s);
  }
else
  {
  sprintf(&wn[6]," MIT SHM %s %s",PROGRAM_NAME,s);
  }
window_name=wn;
XStringListToTextProperty(&window_name, 1, &window_title_property);
XSetWMName(xdis, xwin, &window_title_property);
XFree(window_title_property.value);
}

void x_global_uiparms(int wn)
{
char s[80],ss[80];
char sr[80],st[80],su[80];
char chr;
#if SHM_INSTALLED != 1
char sv[80];
int i;
#endif
int line, maxprio;
line=2;
if(ui.vga_mode==0)ui.vga_mode=12;
if(ui.mouse_speed <=0 || ui.mouse_speed> 120)ui.mouse_speed=8;
if(ui.vga_mode < 10 || ui.vga_mode > 256)ui.vga_mode=10;
ui.process_priority=1;
if(wn != 0)
  { 
  if(ui.operator_skil == OPERATOR_SKIL_NEWCOMER)
    {
    clear_screen();
    lir_text(0,4,"You are now in NEWCOMER mode.");
    lir_text(0,6,"Press 'Y' to change to NORMAL mode or 'N' to");
    lir_text(0,7,"stay in newcomer mode.");
ask_newco:;
    await_processed_keyboard();
    if(lir_inkey == 'N')goto stay_newco;
    if(lir_inkey != 'Y')goto ask_newco;
    ui.operator_skil=OPERATOR_SKIL_NORMAL;
    }
stay_newco:;
  if(ui.operator_skil == OPERATOR_SKIL_NORMAL)
    {
    clear_screen();
    lir_text(0,7,"You are now in NORMAL mode.");
    lir_text(0,9,"Press 'Y' to change to EXPERT mode or 'N' to");
    lir_text(0,10,"stay in normal mode.");
ask_normal:;
    await_processed_keyboard();
    if(lir_inkey == 'N')goto stay_normal;
    if(lir_inkey != 'Y')goto ask_normal;
    ui.operator_skil=OPERATOR_SKIL_EXPERT;
    }
stay_normal:;
  clear_screen();
  }
if(ui.operator_skil == OPERATOR_SKIL_NEWCOMER)
  {
  ui.font_scale=2;
  wse.parport=0;
  wse.parport_pin=0;
  ui.max_blocked_cpus=0;
  }
else
  {  
  sprintf(s,"Font scale (1 to 5):"); 
  if(wn==0)
    {
fntc:;
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    lir_inkey=to_upper(ss[0]);
    if(lir_inkey < '1' || lir_inkey > '5')goto fntc;
    ui.font_scale=lir_inkey-'0';
    }
  else
    {
    lir_text(0,line,s);
    ui.font_scale=lir_get_integer(21,line,2,1,5);
    if(kill_all_flag) return;
    line++;
    }
#if SHM_INSTALLED == 1
  sprintf(s,"The libraries for MIT-SHM are installed. Use SHM (Y/N) ?");
use_shm:
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,3,stdin)==NULL)lir_sleep(10000);
    lir_inkey=to_upper(ss[0]);
    }
  else
    {
    clear_lines(line,line);
    lir_text(0,line,s);
    to_upper_await_keyboard();
    if(kill_all_flag) return;
    }
  if(lir_inkey == 'Y')
    {
    ui.shm_mode=1;
    }
  else
    {
    if(lir_inkey != 'N')goto use_shm;
    ui.shm_mode=0;
    }
  line++;
#else
  for(i=0; i<79; i++)sv[i]='*';
  sv[79]=0;
  sprintf(s,"The libraries for MIT-SHM are not installed.");
  sprintf(ss,"Type ./configure --with-help and look for instructions under");
#if IA64 == 0  
  sprintf(sr,"MISSING: libXext.so (32bit) (or headers)"); 
#else
  sprintf(sr,"MISSING: libXext.so (64bit) (or headers)"); 
#endif
  sprintf(st,
          "Then run ./configure and after that make xlinrad. Finally enable");
  sprintf(su,
        "MIT-SHM in the 'S=Global parms set up' in the main menu of Linrad.");
  if(wn==0)
    {
    printf("\n\n%s",sv); 
    printf("\n%s",s); 
    printf("\n%s",ss); 
    printf("\n%s",sr); 
    printf("\n%s",st); 
    printf("\n%s",su); 
    printf("\n%s\n\n",sv); 
    }
  else
    {
    line++;
    lir_text(0,line,sv);
    line++;
    lir_text(0,line,s);
    line++;
    lir_text(0,line,ss);
    line++;
    lir_text(0,line,sr);
    line++;
    lir_text(0,line,st);
    line++;
    lir_text(0,line,su);
    line++;
    lir_text(0,line,sv);
    line+=2;
    }
#endif
  if(no_of_processors > 1)
    {
    sprintf(s,"This system has % d processors.",no_of_processors);
    sprintf(ss,"How many do you allow Linrad to block?");
    sprintf(sr,
        "If you run several instances of Linrad on one multiprocessor");
    sprintf(st,"platform it may be a bad idea to allow the total number");
    sprintf(su,"of blocked CPUs to be more that the total number less one.");        
    if(wn==0)
      {
      printf("\n%s",s); 
      printf("\n%s",ss); 
      printf("\n%s",sr); 
      printf("\n%s",st); 
      printf("\n%s\n\n=>",su); 
      fflush(NULL);
      while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
      sscanf(ss,"%d", &ui.max_blocked_cpus);
      if(ui.max_blocked_cpus <0)ui.max_blocked_cpus=0;
      if(ui.max_blocked_cpus >=no_of_processors)
                                          ui.max_blocked_cpus=no_of_processors;
      }
    else
      {
      line++;
      lir_text(0,line,s);
      line++;
      lir_text(0,line,ss);
      line++;
      lir_text(0,line,sr);
      line++;
      lir_text(0,line,st);
      line++;
      lir_text(0,line,su);
      line+=2;
      lir_text(0,line,"=>");
      ui.max_blocked_cpus=lir_get_integer(3,line,2,0,no_of_processors-1);
      line+=2;
      }
    }    
  else
    {
    ui.max_blocked_cpus=0;
    }  
  }
sprintf(s,"You can specify the screen size in pixels or as a percentage");
sprintf(sr,"of the entire area of all your screens. Enter Y for sizes in"); 
sprintf(st,"pixels or N for sizes as %% (Y/N)=>");
screen_sel:;
if(wn == 0)
  {
  printf("\n%s",s); 
  printf("\n%s",sr); 
  printf("\n%s",st); 
  fflush(NULL);
  while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
  sscanf(ss,"%c", &chr);
  }
else
  {
  clear_lines(line,line);
  lir_text(0,line,s);
  line++;
  lir_text(0,line,sr);
  line++;
  lir_text(0,line,st);
  line++;
  await_keyboard();
  chr=lir_inkey;
  if(kill_all_flag) return;
  }
chr=toupper(chr);  
switch (chr)
  {
  case 'N':    
  sprintf(s,"Percentage of screen width to use(25 to 100):"); 
parport_wfac:;
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    sscanf(ss,"%d", &ui.screen_width_factor);
    if(ui.screen_width_factor < 25 ||
       ui.screen_width_factor > 100)goto parport_wfac;
    }
  else
    {
    clear_lines(line,line);
    lir_text(0,line,s);
    ui.screen_width_factor=lir_get_integer(47,line,3,25,100);
    if(kill_all_flag) return;
    line++;
    }
  sprintf(s,"Percentage of screen height to use (25 to 100):"); 
parport_hfac:;
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    sscanf(ss,"%d", &ui.screen_height_factor);
    if(ui.screen_height_factor < 25 ||
       ui.screen_height_factor > 100)goto parport_hfac;
    }
  else
    {
    lir_text(0,line,s);
    ui.screen_height_factor=lir_get_integer(49,line,3,25,100);
    if(kill_all_flag) return;
    clear_screen();
    }
  break;
  
  case 'Y':    
  sprintf(s,"Screen width in pixels (500 to 10000):"); 
parport_wpix:;
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    sscanf(ss,"%d", &ui.screen_width_factor);
    if(ui.screen_width_factor < 500 ||
       ui.screen_width_factor > 10000)goto parport_wpix;
    }
  else
    {
    clear_lines(line,line);
    lir_text(0,line,s);
    ui.screen_width_factor=lir_get_integer(38,line,5,500,10000);
    if(kill_all_flag) return;
    line++;
    }
  ui.screen_width_factor=-ui.screen_width_factor;
  sprintf(s,"Screen height in pixels (400 to 10000):"); 
parport_hpix:;
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    sscanf(ss,"%d", &ui.screen_height_factor);
    if(ui.screen_height_factor < 400 ||
       ui.screen_height_factor > 10000)goto parport_hpix;
    }
  else
    {
    lir_text(0,line,s);
    ui.screen_height_factor=lir_get_integer(39,line,5,400,10000);
    if(kill_all_flag) return;
    clear_screen();
    }
  ui.screen_height_factor=-ui.screen_height_factor;
  break;
  
  default:
  goto screen_sel;
  }
if(ui.operator_skil != OPERATOR_SKIL_NEWCOMER)
  {
  prio:;
  sprintf(s,"Set process priority (0=NORMAL to "); 
  if(ui.operator_skil == OPERATOR_SKIL_EXPERT)
    {
    strcat(s,"3=REALTIME)");
    maxprio=3;
    }
  else  
    {
    strcat(s,"2=HIGH)");
    maxprio=2;
    }
  if(wn==0)
    {
    printf("\n%s, then press Enter: ",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    lir_inkey=to_upper(ss[0]);
    }  
  else
    {
    clear_screen();
    lir_text(0,7,s);
    to_upper_await_keyboard();
    }
  if(lir_inkey >='0' && lir_inkey <= maxprio+'0')
    {
    ui.process_priority=lir_inkey-'0';
    }
  else
    {
    goto prio;
    }
  if(ui.process_priority==2)
    {
    sprintf(s,"Are you sure? (Y/N)");
    if(wn==0)
      {
      printf("\n%s\n=>",s); 
      fflush(NULL);
      while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
      if(to_upper(ss[0])=='Y')goto prio_x;
      goto prio;
      }
    else
      {
      clear_screen();
      lir_text(0,3,s);
      to_upper_await_keyboard();
      if(lir_inkey=='Y')goto prio_xc;
      goto prio;
      }
    }
  if(ui.process_priority==3)
    {
    sprintf(s,"Hmmm, you are the expert so you know what might happen....");
    sprintf(ss,"Are you really sure? (Y/N)");
    if(wn==0)
      {
      printf("\n%s\n=>",s); 
      printf("\n%s\n=>",ss); 
      fflush(NULL);
      while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
      if(to_upper(ss[0])=='Y')goto prio_x;
      goto prio;
      }
    else
      {
      clear_screen();
      lir_text(0,3,s);
      lir_text(0,5,ss);
      to_upper_await_keyboard();
      if(lir_inkey=='Y')goto prio_xc;
      goto prio;
      }
    }
prio_xc:;
  if(wn!=0)clear_screen();
  }
prio_x:;  
if(ui.operator_skil == OPERATOR_SKIL_EXPERT)
  {
  sprintf(s,"Set autostart: Z=none,A=WCW,B=CW,C=MS,D=SSB,E=FM,F=AM,G=QRSS");
get_autostart:;    
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    fflush(NULL);
    while(fgets(ss,8,stdin)==NULL)lir_sleep(10000);
    }         
  else
    {
    lir_text(0,6,s);
    lir_text(0,8,"=>");
    to_upper_await_keyboard();
    sprintf(ss,"%c",lir_inkey);
    lir_text(3,8,ss);
    line=8;
    }
  lir_inkey=to_upper(ss[0]);
  if(lir_inkey == 'Z')
    {
    ui.autostart=0;
    }
  else
    {
    if(lir_inkey < 'A' || lir_inkey > 'G')goto get_autostart;
    ui.autostart=lir_inkey;
    }
  }  
if(wn == 0)
  {
  printf("\n\nLinrad will now open another window.");
  printf("\nMinimize this window and click on the new window to continue.");
  printf(
      "\n\nDo not forget to save your parameters with 'W' in the main menu");
  fflush(NULL);
  }
else  
  { 
  line+=4;
  lir_text(0,line,"Save the new parameters with W in the main menu");
  line++;
  lir_text(0,line,"Then restart Linrad for changes to take effect.");
  line++;
  lir_text(10,line,press_any_key);
  await_keyboard();
  }
}

void lin_global_uiparms(int wn)
{
(void) wn;
}

// *********************************************************
// Graphics for X11
// *********************************************************

void lir_getpalettecolor(int j, int *r, int *g, int *b)
{
int k;
unsigned short int *ipalette;
switch (color_depth)
  {
  case 24:
  b[0]=(int)(svga_palette[3*j  ])>>2;
  g[0]=(int)(svga_palette[3*j+1])>>2;
  r[0]=(int)(svga_palette[3*j+2])>>2;
  break;

  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  k=ipalette[j];
  k<<=1;
  b[0]=k&0x3f;
  k>>=6;
  g[0]=k&0x3f;
  k>>=5;
  r[0]=k;
  break;

  case 8:
  b[0]=(int)(svga_palette[3*j  ]);
  g[0]=(int)(svga_palette[3*j+1]);
  r[0]=(int)(svga_palette[3*j+2]);
  break;
  }
}

void lir_fillbox(int x, int y,int w, int h, unsigned char c)
{
int i, j, k;
unsigned short int *ipalette;
k=(x+y*screen_width);
if(k<0 || k+(h-1)*screen_width+w > screen_totpix)
  {
  lirerr(1213);
  return;
  }
if(first_mempix > k)
  {
  first_mempix_copy=k;
  first_mempix=k;
  first_mempix_flag=1;
  }
switch (color_depth)
  {
  case 24:
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      mempix_char[4*(k+i)  ]=svga_palette[3*c  ];
      mempix_char[4*(k+i)+1]=svga_palette[3*c+1];
      mempix_char[4*(k+i)+2]=svga_palette[3*c+2];
      }
    k+=screen_width;
    }
  break;
      
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      mempix_shi[k+i]=ipalette[c];
      }
    k+=screen_width;
    }
  break;

  case 8:
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      mempix_char[k+i]=xpalette[c];
      }
    k+=screen_width;
    }
  break;
  }    
k-=screen_width-w;
if(last_mempix < k)
  {
  last_mempix_copy=k;
  last_mempix=k;
  last_mempix_flag=1;
  }
}

void lir_getbox(int x, int y, int w, int h, size_t* dp)
{
unsigned char *savmem;
int i, j, k, m;
unsigned char c,c1,c2,c3;
unsigned short int *ipalette;
k=x+y*screen_width;
savmem=(unsigned char*)dp;
m=0;
c=0;
c1=0;
c2=0;
switch (color_depth)
  {
  case 24:
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      if(svga_palette[3*c  ] == mempix_char[4*(k+i)  ] &&
         svga_palette[3*c+1] == mempix_char[4*(k+i)+1] &&
         svga_palette[3*c+2] == mempix_char[4*(k+i)+2])goto store_24;
      if(svga_palette[3*c1  ] == mempix_char[4*(k+i)  ] &&
         svga_palette[3*c1+1] == mempix_char[4*(k+i)+1] &&
         svga_palette[3*c1+2] == mempix_char[4*(k+i)+2])
        {
        c3=c;
        c=c1;
        c1=c;
        goto store_24;
        }
      if(svga_palette[3*c2  ] == mempix_char[4*(k+i)  ] &&
         svga_palette[3*c2+1] == mempix_char[4*(k+i)+1] &&
         svga_palette[3*c2+2] == mempix_char[4*(k+i)+2])
        {
        c3=c;
        c=c2;
        c1=c;
        c2=c3;
        goto store_24;
        }
      c2=c1;
      c1=c;
      while(svga_palette[3*c  ] != mempix_char[4*(k+i)  ] ||
            svga_palette[3*c+1] != mempix_char[4*(k+i)+1] ||
            svga_palette[3*c+2] != mempix_char[4*(k+i)+2])
        {
        c++;
        if(c>=MAX_SVGA_PALETTE)c=0;
        if(c==c1)
          {
          lirerr(1248);
          return;
          }
        }
store_24:;
      savmem[m]=c;
      m++;
      }
    k+=screen_width;
    }
  break;

  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      if(ipalette[c] == mempix_shi[(k+i)]) goto store_16;
      if(ipalette[c1] == mempix_shi[(k+i)])
        {
        c3=c;
        c=c1;
        c1=c;
        goto store_16;
        }
      if(ipalette[c2] == mempix_shi[(k+i)])
        {
        c3=c;
        c=c2;
        c1=c;
        c2=c3;
        goto store_16;
        }
      c2=c1;
      c1=c;
      while(ipalette[c] != mempix_shi[(k+i)])
        {
        c++;
        if(c>=MAX_SVGA_PALETTE)c=0;
        if(c==c1)
          {
          lirerr(1248);
          return;
          }
        }
store_16:;
      savmem[m]=c;
      m++;
      }
    k+=screen_width;
    }
  break;

  case 8:
  for(j=0; j<h; j++)
    {
    for(i=0; i<w; i++)
      {
      if(xpalette[c] == mempix_char[(k+i)]) goto store_8;
      if(xpalette[c1] == mempix_char[(k+i)])
        {
        c3=c;
        c=c1;
        c1=c;
        goto store_8;
        }
      if(xpalette[c2] == mempix_char[(k+i)])
        {
        c3=c;
        c=c2;
        c1=c;
        c2=c3;
        goto store_8;
        }
      c2=c1;
      c1=c;
      while(xpalette[c] != mempix_char[(k+i)])
        {
        c++;
        if(c>=MAX_SVGA_PALETTE)c=0;
        if(c==c1)
          {
          lirerr(1248);
          return;
          }
        }
store_8:;
      savmem[m]=c;
      m++;
      }
    k+=screen_width;
    }
  break;
  }
}

void lir_putbox(int x, int y, int w, int h, size_t* dp)
{
unsigned char *savmem;
int i, j, k, m;
unsigned short int *ipalette;
k=(x+y*screen_width);
if(k<0 || k+(h-1)*screen_width+w-1 >= screen_totpix)
  {
  lirerr(1213);
  return;
  }
if(first_mempix > k)
  {
  first_mempix_copy=k;
  first_mempix=k;
  first_mempix_flag=1;
  }
m=0;
savmem=(unsigned char*)dp;
switch (color_depth)
  {
  case 24:
  for(j=0; j<h; j++)
    {  
    for(i=0; i<w; i++)
      {
      mempix_char[4*(k+i)  ]=svga_palette[3*savmem[m]  ];
      mempix_char[4*(k+i)+1]=svga_palette[3*savmem[m]+1];
      mempix_char[4*(k+i)+2]=svga_palette[3*savmem[m]+2];
      m++;
      }
    k+=screen_width;
    }
  break;
  
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  for(j=0; j<h; j++)
    {  
    for(i=0; i<w; i++)
      {
      mempix_shi[k+i]=ipalette[savmem[m]];
      m++;
      }
    k+=screen_width;
    }
  break;

  case 8:
  for(j=0; j<h; j++)
    {  
    for(i=0; i<w; i++)
      {
      mempix_char[k+i]=xpalette[savmem[m]];
      m++;
      }
    k+=screen_width;
    }
  }  
if(last_mempix < k)
  {
  last_mempix_copy=k;
  last_mempix=k;
  last_mempix_flag=1;
  }
}

void lir_hline(int x1, int y, int x2, unsigned char c)
{
int i, ia, ib;
unsigned short int *ipalette;
ia=y*screen_width;
if(x1 <= x2)
  {
  ib=ia+x2;
  ia+=x1;
  }
else
  {
  ib=ia+x1;
  ia+=x2;
  }
if(ia < 0 || ib >= screen_totpix)
  {
  lirerr(1214);
  return;
  }
switch (color_depth)
  {
  case 24:
  for(i=ia; i<=ib; i++)
    {
    mempix_char[4*i  ]=svga_palette[3*c  ];
    mempix_char[4*i+1]=svga_palette[3*c+1];
    mempix_char[4*i+2]=svga_palette[3*c+2];
    }
  break;
  
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  for(i=ia; i<=ib; i++)
    {
    mempix_shi[i]=ipalette[c];
    }
  break;

  case 8:
  for(i=ia; i<=ib; i++)
    {
    mempix_char[i]=xpalette[c];
    }
  break;
  }
if(first_mempix > ia)
  {
  first_mempix_copy=ia;
  first_mempix=ia;
  first_mempix_flag=1;
  }
if(last_mempix < ib)
  {
  last_mempix_copy=ib;
  last_mempix=ib;
  last_mempix_flag=1;
  }
}

void lir_line(int x1, int yy1, int x2,int y2, unsigned char c)
{
int ia;
int i,j,k;
int xd, yd;
float t1,t2,delt;
unsigned short int *ipalette;
if(x1 < 0)x1=0;
if(x1 >= screen_width)x1=screen_width-1;
if(x2 < 0)x2=0;
if(x2 >= screen_width)x2=screen_width-1;
if(yy1 < 0)yy1=0;
if(yy1 >= screen_height)yy1=screen_height-1;
if(y2 < 0)y2=0;
if(y2 >= screen_height)y2=screen_height-1;
xd=x2-x1;
yd=y2-yy1;
if(yd==0)
  {
  if(xd==0)
    {
    lir_setpixel(x1,yy1,c);
    }
  else
    {
    lir_hline(x1,yy1,x2,c);
    }
  return;
  }
if(abs(xd)>=abs(yd))
  {
  if(xd>=0)
    {
    k=1;
    }
  else
    {
    k=-1;
    }  
  if(yd >= 0)
    {
    delt=0.5;
    }
  else
    {  
    delt=-0.5;
    }
  t1=yy1;
  t2=(float)(yd)/abs(xd);
  i=x1-k;
  switch (color_depth)
    {
    case 24:
    while(i!=x2)
      {
      i+=k;
      j=t1+delt;
      ia=i+j*(screen_width);
      mempix_char[4*ia  ]=svga_palette[3*c  ];
      mempix_char[4*ia+1]=svga_palette[3*c+1];
      mempix_char[4*ia+2]=svga_palette[3*c+2];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;
    
    case 16:
    ipalette=(unsigned short int*)(&svga_palette[0]);
    while(i!=x2)
      {
      i+=k;
      j=t1+delt;
      ia=i+j*(screen_width);
      mempix_shi[ia  ]=ipalette[c  ];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;
    
    case 8:
    while(i!=x2)
      {
      i+=k;
      j=t1+delt;
      ia=i+j*(screen_width);
      mempix_char[ia  ]=xpalette[c  ];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;
        
    }  
  }  
else
  {
  if(yd>=0)
    {
    k=1;
    }
  else
    {
    k=-1;
    } 
  if(xd >= 0)
    {
    delt=0.5;
    }
  else
    {  
    delt=-0.5;
    }
  t1=x1;
  t2=(float)(xd)/abs(yd);
  i=yy1-k;
  switch (color_depth)
    {
    case 24:
    while(i!=y2)
      {
      i+=k;
      j=t1+delt;
      ia=j+i*(screen_width);
      mempix_char[4*ia  ]=svga_palette[3*c  ];
      mempix_char[4*ia+1]=svga_palette[3*c+1];
      mempix_char[4*ia+2]=svga_palette[3*c+2];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;
    
    case 16:
    ipalette=(unsigned short int*)(&svga_palette[0]);
    while(i!=y2)
      {
      i+=k;
      j=t1+delt;
      ia=j+i*(screen_width);
      mempix_shi[ia  ]=ipalette[c  ];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;

    case 8:
    while(i!=y2)
      {
      i+=k;
      j=t1+delt;
      ia=j+i*(screen_width);
      mempix_char[ia  ]=xpalette[c  ];
      if(first_mempix > ia)
        {
        first_mempix_copy=ia;
        first_mempix=ia;
        first_mempix_flag=1;
        }
      if(last_mempix < ia)
        {
        last_mempix_copy=ia;
        last_mempix=ia;
        last_mempix_flag=1;
        }
      t1+=t2;
      }
    break;
    }    
  }  
}

void lir_setpixel(int x, int y, unsigned char c)
{
int ia;
unsigned short int *ipalette;
ia=x+y*screen_width;
if(ia < 0 || ia >= screen_totpix)
  {
  DEB"\nwrite: x=%d   y=%d",x,y);
  lirerr(1210);
  return;
  }
switch (color_depth)
  {
  case 24:
  mempix_char[4*ia  ]=svga_palette[3*c  ];
  mempix_char[4*ia+1]=svga_palette[3*c+1];
  mempix_char[4*ia+2]=svga_palette[3*c+2];
  break;
  
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  mempix_shi[ia]=ipalette[c];
  break;

  case 8:
  mempix_char[ia]=xpalette[c];
  break;
  }
if(first_mempix > ia)
  {
  first_mempix_copy=ia;
  first_mempix=ia;
  first_mempix_flag=1;
  }
if(last_mempix < ia)
  {
  last_mempix_copy=ia;
  last_mempix=ia;
  last_mempix_flag=1;
  }
}

void clear_screen(void)
{
int i, k;
switch (color_depth)
  {
  case 24:
  k=4*screen_width*screen_height;
  for(i=0; i<k; i+=4)
    {
    mempix_char[i]=svga_palette[3*CLRSCR_COLOR];
    mempix_char[i+1]=svga_palette[3*CLRSCR_COLOR+1];
    mempix_char[i+2]=svga_palette[3*CLRSCR_COLOR+2];
    mempix_char[i+3]=255;
    }
  break;

  case 16:
  k=screen_width*screen_height;
  for(i=0; i<k; i++)
    {
    mempix_shi[i]=0;
    }
  break;
  
  case 8:
  k=screen_width*screen_height;
  for(i=0; i<k; i++)
    {
    mempix_char[i]=xpalette[0];
    }
  }
lir_refresh_entire_screen();
lir_refresh_screen();
}

void lir_refresh_entire_screen(void)
{
first_mempix_copy=0;
first_mempix=0;
first_mempix_flag=1;
last_mempix_copy=screen_totpix-1;
last_mempix=screen_totpix-1;
last_mempix_flag=1;
}

void lir_refresh_screen(void)
{
lir_set_event(EVENT_REFRESH_SCREEN);
}

void thread_mouse(void)
{
wxmouse();
}

void thread_refresh_screen(void)
{
int l1, h, h1, n;
while(refresh_screen_flag)
  {
  lir_await_event(EVENT_REFRESH_SCREEN);
  if(last_mempix >= first_mempix )
    {
    first_mempix_flag=0;
    last_mempix_flag=0;
    l1=first_mempix/screen_width;
    h=last_mempix/screen_width-l1+1;
    if(h>screen_height)h=screen_height;
    first_mempix=0x7fffffff;
    last_mempix=0;
    lir_sched_yield();
    if(first_mempix_flag != 0)first_mempix=first_mempix_copy;
    if(last_mempix_flag != 0)last_mempix=last_mempix_copy;
    while(h>0)
      {
      n=(5*h)/screen_height;
      if(n > 1)
        {
        h1=h/n;
        }
      else
        {
        h1=h;
        }
      if(X11_accesstype==X11_STANDARD)
        {
        XPutImage(xdis, xwin, xgc, ximage, 0, l1, 0, l1, screen_width, h1);
        }
#if SHM_INSTALLED == 1
      else
        {
        XShmPutImage(xdis, xwin, xgc, ximage, 0, l1, 0, l1, screen_width, h1,FALSE);
        }
#endif
      l1+=h1;
      h-=h1;
      if(h > 0)lir_sched_yield();
      }
#if AVOID_XFLUSH == TRUE    
#if USE_XFLUSH == 1
    XFlush(xdis);
#endif
#else
    XFlush(xdis);
#endif
    }
  }
}  
