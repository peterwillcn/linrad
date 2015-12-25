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


#include <vga.h>
#include <vgagl.h>
#include <semaphore.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

#include "globdef.h"
#include "lconf.h"
#include "rusage.h"
#include "thrdef.h"
#include "fdef.h"
#include "uidef.h"
#include "screendef.h"
#include "keyboard_def.h"
#include "sdrdef.h"

void graphics_close(void);

void lir_fix_bug(int bug)
{
(void) bug;
}

void lir_set_title(char *s)
{
(void) s;
}

void lin_global_uiparms(int wn)
{
char ss[80];
int line;
char s[80];
int maxprio;
clear_keyboard();
line=0;
if(wn!=0)
  {
  if( ui.operator_skil == OPERATOR_SKIL_NEWCOMER)
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
  if( ui.operator_skil == OPERATOR_SKIL_NORMAL)
    {
    clear_screen();
    lir_text(0,7,"You are now in NORMAL mode.");
    lir_text(0,9," 'Y' to change to EXPERT mode or 'N' to");
    lir_text(0,10,"stay in normal mode.");
ask_normal:;
    await_processed_keyboard();
    if(lir_inkey == 'N')goto stay_normal;
    if(lir_inkey != 'Y')goto ask_normal;
    ui.operator_skil=OPERATOR_SKIL_EXPERT;
    }
stay_normal:;
  }
// *******************************************************
if(wn!=0)
  {
  clear_screen();
  }
if(ui.operator_skil == OPERATOR_SKIL_NEWCOMER)
  {
  ui.font_scale=2;
  }
else
  {
  sprintf(s,"Enter font scale (1 to 5)"); 
fntc:;
  if(wn==0)
    {
    printf("\n%s\n=>",s); 
    await_keyboard();
    if(lir_inkey==0)exit(0);
    }
  else
    {
    lir_text(0,line,s);
    await_keyboard();
    if(kill_all_flag) return;
    line++;
    if(line>=screen_last_line)line=screen_last_line;
    }
  if(lir_inkey < '1' || lir_inkey > '5')goto fntc;
  ui.font_scale=lir_inkey-'0';
  }
if(wn == 0)graphics_init();
init_font(ui.font_scale);
if(kill_all_flag)return;
clear_screen();
settextcolor(15);
lir_text(0,2,"Mouse speed reduction factor:");
ui.mouse_speed=lir_get_integer(30,2,3,1,999);
ui.max_blocked_cpus=0;
ui.process_priority=0;
wse.parport=0;
wse.parport_pin=0;
if(ui.operator_skil != OPERATOR_SKIL_NEWCOMER)
  {
  if(no_of_processors > 1)
    {
    clear_screen();
    sprintf(s,"This system has % d processors.",no_of_processors);
    lir_text(0,3,s);
    lir_text(0,4,"How many do you allow Linrad to block?");
    lir_text(0,5,
        "If you run several instances of Linrad on one multiprocessor");
    lir_text(0,6,"platform it may be a bad idea to allow the total number");
    lir_text(0,7,"of blocked CPUs to be more that the total number less one.");        
    lir_text(5,9,"=>");
    ui.max_blocked_cpus=lir_get_integer(8,9,2,0,no_of_processors-1);
    }    
prio:;
  clear_screen();
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
  lir_text(0,7,s);
  to_upper_await_keyboard();
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
    clear_screen();
    lir_text(0,3,s);
    to_upper_await_keyboard();
    if(lir_inkey=='Y')goto prio_x;
    goto prio;
    }
  if(ui.process_priority==3)
    {
    sprintf(s,"Hmmm, you are the expert so you know what might happen....");
    sprintf(ss,"Are you really sure? (Y/N)");
    clear_screen();
    lir_text(0,3,s);
    lir_text(0,5,ss);
    to_upper_await_keyboard();
    if(lir_inkey=='Y')goto prio_x;
    goto prio;
    }
prio_x:;
  if(ui.operator_skil == OPERATOR_SKIL_EXPERT)
    {
    get_autostart:;    
    clear_screen();
    lir_text(0,3,
             "Set autostart: Z=none,A=WCW,B=CW,C=MS,D=SSB,E=FM,F=AM,G=QRSS");
    to_upper_await_keyboard();
    }
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
if(ui.screen_width_factor <= 0)ui.screen_width_factor=96;
if(ui.screen_height_factor <= 0)ui.screen_height_factor=92;
clear_screen();
lir_text(0,6,
          "Do not forget to save your parameters with 'W' in the main menu");
lir_text(5,8,press_any_key);
await_keyboard();
clear_screen();
settextcolor(7);
}

void x_global_uiparms(int n)
{
(void) n;
// Dummy routine. Not used under Linux svgalib.
}

// *********************************************************
// Mouse routines for svgalib to draw a mouse cross on screen.
// *********************************************************

void restore_behind_mouse(void)
{
int i,imax;
imax=screen_width-mouse_x;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  lir_setpixel(mouse_x+i,mouse_y,behind_mouse[4*i  ]);
  }
imax=mouse_x;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  lir_setpixel(mouse_x-i,mouse_y,behind_mouse[4*i+1]);
  }
imax=screen_height-mouse_y;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  lir_setpixel(mouse_x,mouse_y+i,behind_mouse[4*i+2]);
  }
imax=mouse_y;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  lir_setpixel(mouse_x,mouse_y-i,behind_mouse[4*i+3]);
  }
lir_setpixel(mouse_x,mouse_y,behind_mouse[0]);
mouse_hide_flag=0;
}


void hide_mouse(int x1,int x2,int iy1,int y2)
{
if(mouse_hide_flag ==0)return;
if(mouse_xmax < x1)return;
if(mouse_xmin > x2)return;
if(mouse_ymax < iy1)return;
if(mouse_ymin > y2)return;
restore_behind_mouse();
}

void unconditional_hide_mouse(void)
{
if(mouse_hide_flag ==0)return;
restore_behind_mouse();
}


char lir_getpixel(int x, int y)
{
int ia;
int c;
unsigned short int *ipalette;
c=-1;
ia=x+y*screen_width;
if(ia < 0 || ia >= screen_totpix)
  {
  DEB"\nwrite: x=%d   y=%d",x,y);
  lirerr(1210);
  return 0;
  }
switch (color_depth)
  {
  case 24:
  case 32:
nxt24:
  c++;  
  if(c >= MAX_SVGA_PALETTE)
    {
lirerr(223232);
    return 0;
    }
  if(mempix_char[4*ia  ]!=svga_palette[3*c  ])goto nxt24;
  if(mempix_char[4*ia+1]!=svga_palette[3*c+1])goto nxt24;
  if(mempix_char[4*ia+2]!=svga_palette[3*c+2])goto nxt24;
  break;
  
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
nxt16:
  c++;  
  if(c >= MAX_SVGA_PALETTE)
    {
    lirerr(842318);
    return 0;
    }
  if(mempix_shi[ia]!=ipalette[c])goto nxt16;
  break;

  case 8:
  c=mempix_char[ia];
  break;
  }
return (char)c;  
}




void show_mouse(void)
{
int i,imax;
unsigned char color;
if(mouse_hide_flag == 1)return;
mouse_hide_flag=1;
color=15-leftpressed;
imax=screen_width-mouse_x;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
lir_getpixel(mouse_x+i,mouse_y);
  behind_mouse[4*i  ]=lir_getpixel(mouse_x+i,mouse_y);
  lir_setpixel(mouse_x+i,mouse_y,color);
  }
imax=mouse_x;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  behind_mouse[4*i+1]=lir_getpixel(mouse_x-i,mouse_y);
  lir_setpixel(mouse_x-i,mouse_y,color);
  }
imax=screen_height-mouse_y;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  behind_mouse[4*i+2]=lir_getpixel(mouse_x,mouse_y+i);
  lir_setpixel(mouse_x,mouse_y+i,color);
  }
imax=mouse_y;
if(imax > mouse_cursize) imax=mouse_cursize;
for(i=1; i<imax; i++)
  {
  behind_mouse[4*i+3]=lir_getpixel(mouse_x,mouse_y-i);
  lir_setpixel(mouse_x,mouse_y-i,color);
  }
behind_mouse[0]=lir_getpixel(mouse_x,mouse_y);
lir_setpixel(mouse_x,mouse_y,color);
}

void lir_move_mouse_cursor(void)
{
if(mouse_hide_flag !=0)restore_behind_mouse();
set_button_coordinates();
mouse_xmax=mouse_x+mouse_cursize;
mouse_xmin=mouse_x-mouse_cursize;
mouse_ymax=mouse_y+mouse_cursize;
mouse_ymin=mouse_y-mouse_cursize;
}

// *********************************************************
// Graphics for Linux framebuffer 
// *********************************************************

void lir_getpalettecolor(int j, int *r, int *g, int *b)
{
int k;
unsigned short int *ipalette;
switch (color_depth)
  {
  case 24:
  case 32:
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
switch (color_depth)
  {
  case 24:
  case 32:
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
      mempix_char[k+i]=c;
      }
    k+=screen_width;  
    }
  break;
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
  case 32:
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
//          lirerr(1248);
c=0;
goto store_24;


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
      savmem[m]=mempix_char[k+i];
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
m=0;
savmem=(unsigned char*)dp;
switch (color_depth)
  {
  case 24:
  case 32:
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
      mempix_char[k+i]=savmem[m];
      m++;
      }
    k+=screen_width;
    }
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
  case 32:
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
    mempix_char[i]=c;
    }
  break;
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
    case 32:
    while(i!=x2)
      {
      i+=k;
      j=t1+delt;
      ia=i+j*(screen_width);
      mempix_char[4*ia  ]=svga_palette[3*c  ];
      mempix_char[4*ia+1]=svga_palette[3*c+1];
      mempix_char[4*ia+2]=svga_palette[3*c+2];
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
      t1+=t2;
      }
    break;
    
    case 8:
    while(i!=x2)
      {
      i+=k;
      j=t1+delt;
      ia=i+j*(screen_width);
      mempix_char[ia  ]=c;
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
    case 32:
    while(i!=y2)
      {
      i+=k;
      j=t1+delt;
      ia=j+i*(screen_width);
      mempix_char[4*ia  ]=svga_palette[3*c  ];
      mempix_char[4*ia+1]=svga_palette[3*c+1];
      mempix_char[4*ia+2]=svga_palette[3*c+2];
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
      t1+=t2;
      }
    break;

    case 8:
    while(i!=y2)
      {
      i+=k;
      j=t1+delt;
      ia=j+i*(screen_width);
      mempix_char[ia  ]=c;
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
  case 32:
  mempix_char[4*ia  ]=svga_palette[3*c  ];
  mempix_char[4*ia+1]=svga_palette[3*c+1];
  mempix_char[4*ia+2]=svga_palette[3*c+2];
  break;
  
  case 16:
  ipalette=(unsigned short int*)(&svga_palette[0]);
  mempix_shi[ia]=ipalette[c];
  break;

  case 8:
  mempix_char[ia]=c;
  break;
  }
}


void clear_screen(void)
{
int i, k;
switch (color_depth)
  {
  case 24:
  case 32:
  k=4*screen_width*screen_height;
  for(i=0; i<k; i+=4)
    {
    mempix_char[i]=0;
    mempix_char[i+1]=0;
    mempix_char[i+2]=0;
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
    mempix_char[i]=0;
    }
  }
lir_refresh_entire_screen();
lir_refresh_screen();
}


void lir_refresh_screen(void)
{
// This is used by Windows and X11 to force a copy from memory
// to the screen. svgalib is fast enough to allow
// direct screen updates. (SetPixel under Windows is extremely slow)
}

void lir_refresh_entire_screen(void)
{
// This is used by X11 to force a copy from memory
}

