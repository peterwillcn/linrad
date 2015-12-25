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


#include <string.h>
#include "globdef.h"
#include "uidef.h"
#include "screendef.h"
#include "vernr.h"
#include "rusage.h"
#include "thrdef.h"

#define PGW (7*text_width)
#define PGH (PGW+3.5*text_height)  
void make_pol_graph(int clear_old);
int pg_old_y1;
int pg_old_y2;
int pg_old_x1;
int pg_old_x2;

int pol_graph_scro;

  
void check_pg_borders(void)
{
current_graph_minh=PGH;
current_graph_minw=PGW;
check_graph_placement((void*)(&pg));
set_graph_minwidth((void*)(&pg));
}

void help_on_pol_graph(void)
{
int msg_no;
int event_no;
// Set msg invalid in case we are not in any select area.
msg_no=-1;
for(event_no=0; event_no<MAX_PGBUTT; event_no++)
  {
  if( pgbutt[event_no].x1 <= mouse_x && 
      pgbutt[event_no].x2 >= mouse_x &&      
      pgbutt[event_no].y1 <= mouse_y && 
      pgbutt[event_no].y2 >= mouse_y) 
    {
    switch (event_no)
      {
      case PG_TOP:
      case PG_BOTTOM:
      case PG_LEFT:
      case PG_RIGHT:
      msg_no=101;
      break;

      case PG_AUTO:
      msg_no=54;
      break;
  
      case PG_ANGLE:
      msg_no=55;
      break;

      case PG_CIRC:
      msg_no=56;
      break;

      case PG_AVGNUM:
      msg_no=57;
      break;
      }
    }  
  }
help_message(msg_no);
}  




void mouse_continue_pol_graph(void)
{
int i, j;
float t1,t2;
switch (mouse_active_flag-1)
  {
  case PG_TOP:
  if(pg.ytop!=mouse_y)goto pgm;
  break;

  case PG_BOTTOM:
  if(pg.ybottom!=mouse_y)goto pgm;
  break;

  case PG_LEFT:
  if(pg.xleft!=mouse_x)goto pgm;
  break;

  case PG_RIGHT:
  if(pg.xright==mouse_x)break;
pgm:;
  pause_screen_and_hide_mouse();
  graph_borders((void*)&pg,0);
  if(pg_oldx==-10000)
    {
    pg_oldx=mouse_x;
    pg_oldy=mouse_y;
    }
  else
    {
    i=mouse_x-pg_oldx;
    j=mouse_y-pg_oldy;  
    pg_oldx=mouse_x;
    pg_oldy=mouse_y;
    pg.ytop+=j;
    pg.ybottom+=j;      
    pg.xleft+=i;
    pg.xright+=i;
    check_pg_borders();
    }
  graph_borders((void*)&pg,15);
  resume_thread(THREAD_SCREEN);

  break;
    

  default:
  goto await_release;
  }
if(leftpressed == BUTTON_RELEASED)goto finish;
return;
await_release:;
if(leftpressed != BUTTON_RELEASED) return;
switch (mouse_active_flag-1)
  {
  case PG_AUTO:
  pg.adapt^=1;
  break;
  
  case PG_ANGLE:
  t1=mouse_x-pg_x0;
  t2=mouse_y-pg_y0;
  if(fabs(t1)+fabs(t2)>0)
    {
    t1=atan2(-t2,t1)+PI_L*pg.angle/180;
    }
  else
    {
    t1=0;
    }
// Polarization plane is periodic in 180 degrees.
// Place angle in the interval 0 to 180 degrees so sin(angle) is
// always positive.     
  while(t1 < 0)t1+=PI_L;
  if(t1>=PI_L)t1-=PI_L;
  pg.c1=cos(t1);
  t2=sin(t1);
  pg.c2=sin(t1)*cos(pg_b);    
  pg.c3=sin(t1)*sin(pg_b);    
  break;

  case PG_CIRC:
  pg_b=1.2*PI_L*(mouse_x-pg_x0)/(pgbutt[PG_CIRC].x2-pgbutt[PG_CIRC].x1);
  if(pg_b > 0.499*PI_L)pg_b= 0.499*PI_L;
  if(pg_b < -0.499*PI_L)pg_b=-0.499*PI_L;
  if(pg.c1 == 0)pg.c1=0.0001;
  t2=sqrt(1-pg.c1*pg.c1);
  if(pg.c2 < 0)t2*=-1;
  if(t2 == 0)t2=0.0001;
  pg.c2=t2*cos(pg_b);  
  pg.c3=t2*sin(pg_b);  
  break;

  case PG_AVGNUM:
  pg.avg=mouse_x-pg_x0;
  if(pg.avg < 1)pg.avg=1;
  if(pg.avg > pg.xright-pg_x0)pg.avg=pg.xright-pg_x0;
  break;
  }      
finish:;
leftpressed=BUTTON_IDLE;  
mouse_active_flag=0;
make_pol_graph(TRUE);
if(genparm[SECOND_FFT_ENABLE] != 0)new_hg_pol();
pg_oldx=-10000;
}

void mouse_on_pol_graph(void)
{
int event_no;
// First find out is we are on a button or border line.
for(event_no=0; event_no<MAX_PGBUTT; event_no++)
  {
  if( pgbutt[event_no].x1 <= mouse_x && 
      pgbutt[event_no].x2 >= mouse_x &&      
      pgbutt[event_no].y1 <= mouse_y && 
      pgbutt[event_no].y2 >= mouse_y) 
    {
    pg_old_y1=pg.ytop;
    pg_old_y2=pg.ybottom;
    pg_old_x1=pg.xleft;
    pg_old_x2=pg.xright;
    mouse_active_flag=1+event_no;
    current_mouse_activity=mouse_continue_pol_graph;
    return;
    }
  }
// Not button or border.
// Do nothing.
current_mouse_activity=mouse_nothing;
mouse_active_flag=1;
}


void make_pol_graph(int clear_old)
{
int i, j;
char *adafix[]={"Adapt","Fixed"};
pause_thread(THREAD_SCREEN);
if(clear_old)
  {
  hide_mouse(pg_old_x1,pg_old_x2,pg_old_y1,pg_old_y2);
  lir_fillbox(pg_old_x1,pg_old_y1,pg_old_x2-pg_old_x1+1,
                                                    pg_old_y2-pg_old_y1+1,0);
  }
check_pg_borders();
clear_button(pgbutt, MAX_PGBUTT);
hide_mouse(pg.xleft,pg.xright,pg.ytop,pg.ybottom);  
scro[pol_graph_scro].no=POL_GRAPH;
scro[pol_graph_scro].x1=pg.xleft;
scro[pol_graph_scro].x2=pg.xright;
scro[pol_graph_scro].y1=pg.ytop;
scro[pol_graph_scro].y2=pg.ybottom;
pgbutt[PG_LEFT].x1=pg.xleft;
pgbutt[PG_LEFT].x2=pg.xleft+2;
pgbutt[PG_LEFT].y1=pg.ytop;
pgbutt[PG_LEFT].y2=pg.ybottom;
pgbutt[PG_RIGHT].x1=pg.xright-2;
pgbutt[PG_RIGHT].x2=pg.xright;
pgbutt[PG_RIGHT].y1=pg.ytop;
pgbutt[PG_RIGHT].y2=pg.ybottom;
pgbutt[PG_TOP].x1=pg.xleft;
pgbutt[PG_TOP].x2=pg.xright;
pgbutt[PG_TOP].y1=pg.ytop;
pgbutt[PG_TOP].y2=pg.ytop+2;
pgbutt[PG_BOTTOM].x1=pg.xleft;
pgbutt[PG_BOTTOM].x2=pg.xright;
pgbutt[PG_BOTTOM].y1=pg.ybottom-2;
pgbutt[PG_BOTTOM].y2=pg.ybottom;
// Draw the border lines
graph_borders((void*)&pg,7);
settextcolor(7);
pg_oldx=-10000;
// Place the Adapt/Fixed button in lower left corner
i=1+text_width/2;
pgbutt[PG_AUTO].x1=pg.xleft+i;
pgbutt[PG_AUTO].x2=pgbutt[PG_AUTO].x1+5*text_width+i;
pgbutt[PG_AUTO].y2=pg.ybottom-2;
pgbutt[PG_AUTO].y1=pgbutt[PG_AUTO].y2-text_width-i;
lir_pixwrite(pgbutt[PG_AUTO].x1+i,pgbutt[PG_AUTO].y1+i/2,adafix[pg.adapt]);
if(kill_all_flag) return;
lir_hline(pgbutt[PG_AUTO].x1,pgbutt[PG_AUTO].y1,pgbutt[PG_AUTO].x2,7);
if(kill_all_flag) return;
lir_hline(pgbutt[PG_AUTO].x1,pgbutt[PG_AUTO].y2,pgbutt[PG_AUTO].x2,7);
if(kill_all_flag) return;
lir_line(pgbutt[PG_AUTO].x1,pgbutt[PG_AUTO].y1,
        pgbutt[PG_AUTO].x1,pgbutt[PG_AUTO].y2,7);
if(kill_all_flag) return;
lir_line(pgbutt[PG_AUTO].x2,pgbutt[PG_AUTO].y1,
        pgbutt[PG_AUTO].x2,pgbutt[PG_AUTO].y2,7);
if(kill_all_flag) return;
// Control for major axis angle
pg_x0=(pg.xleft+pg.xright)/2;
i=pg.xright-pg.xleft-2;
j=pg.ybottom-pg.ytop-3*text_height;
if(i>j)i=j;
i/=2;
pg_y0=pg.ytop+i+2;
pgbutt[PG_ANGLE].x1=pg_x0-i;
pgbutt[PG_ANGLE].x2=pg_x0+i;
pgbutt[PG_ANGLE].y1=pg_y0-i;
pgbutt[PG_ANGLE].y2=pg_y0+i;
// Control for interchannel phase (circular / linear)
pgbutt[PG_CIRC].x1=pgbutt[PG_ANGLE].x1;
pgbutt[PG_CIRC].x2=pgbutt[PG_ANGLE].x2;
pgbutt[PG_CIRC].y1=pgbutt[PG_ANGLE].y2+text_height/2;
pgbutt[PG_CIRC].y2=pgbutt[PG_CIRC].y1+text_height/2;
// Control for averaging parameter.
pgbutt[PG_AVGNUM].x1=pg_x0+1;
pgbutt[PG_AVGNUM].x2=pgbutt[PG_ANGLE].x2;
pgbutt[PG_AVGNUM].y1=pgbutt[PG_CIRC].y2+1;
pgbutt[PG_AVGNUM].y2=pgbutt[PG_CIRC].y2+text_height-2;
if(pg.adapt == 0)
  {
  i=pgbutt[PG_AVGNUM].x2-pgbutt[PG_AVGNUM].x1-1;
  if(pg.avg>i)pg.avg=i;
  i=pgbutt[PG_AVGNUM].x1+pg.avg;
  lir_fillbox(pgbutt[PG_AVGNUM].x1,pgbutt[PG_AVGNUM].y1,
             pgbutt[PG_AVGNUM].x2-pgbutt[PG_AVGNUM].x1,
             pgbutt[PG_AVGNUM].y2-pgbutt[PG_AVGNUM].y1,PC_CONTROL_COLOR);
  lir_line(i,pgbutt[PG_AVGNUM].y1,i,pgbutt[PG_AVGNUM].y2,15);
  }
dpg.xleft=pg.xleft;        
dpg.xright=pg.xright;        
dpg.ytop=pg.ytop;        
dpg.ybottom=pg.ybottom;        
make_modepar_file(GRAPHTYPE_PG);
resume_thread(THREAD_SCREEN);
show_pol_time=current_time();
sc[SC_SHOW_POL]++;
}


void select_pol_default(void)
{
char s[80];
int iang,isign,iaut;
float t1;
char sgn[]={'+','-'};
char *amode[]={"Auto","Man"};
iang=dpg.angle;
isign=0;
if(iang > 135)
  {
  iang-=180;
  isign=1;
  }
iaut=dpg.adapt;
t1=180*atan2(dpg.c2,dpg.c1)/PI_L;
t1=fabs(t1-dpg.angle);
pol_menu:;
clear_screen();
settextcolor(14);
lir_text(5,5,"INIT POLARISATION PARAMETERS FOR TWO CHANNEL RADIO");
settextcolor(7);
sprintf(s,"A => Angle for channel 1 (0=Hor, 90=vert)  %d",iang);
lir_text(5,8,s); 
sprintf(s,"B => Toggle sign for channel 2             %c",sgn[isign]);
lir_text(5,9,s); 
sprintf(s,"C => Toggle mode                           %s",amode[iaut]);
lir_text(5,10,s); 
sprintf(s,"D => Set start polarization                %d",dpg.startpol);
lir_text(5,11,s); 
sprintf(s,"E => Initial averaging                     %d",dpg.avg);
lir_text(5,12,s); 
lir_text(5,14,"S => Save to disk and exit");
await_processed_keyboard();
if(kill_all_flag) return;
switch (lir_inkey)
  {
  case 'A':
  lir_text(10,17,"Angle:");
  iang=lir_get_integer(16, 17, 4, 0, 90);
  if(kill_all_flag) return;
  break;

  case 'B':
  isign^=1;
  break;

  case 'C':
  iaut^=1;
  break;

  case 'D':
  lir_text(10,17,"Start polarization:");
  dpg.startpol=lir_get_integer(29, 17, 4, 0, 179);
  if(kill_all_flag) return;
  break;
  
  case 'E':
  lir_text(10,17,"Averages:");
  dpg.avg=lir_get_integer(19, 17, 4, 1, pg.xright-pg_x0);
  if(kill_all_flag) return;
  break;
  
  case 'S':
  dpg.angle=iang;
  if(isign != 0)dpg.angle+=180;
  dpg.adapt=iaut;
// Get polarization coefficients for start polarization    
  t1=dpg.angle+dpg.startpol;
  t1*=PI_L/180;
  dpg.c1=cos(t1);
  dpg.c2=sin(t1);
  dpg.c3=0;
  dpg.check=PG_VERNR;
  make_modepar_file(GRAPHTYPE_PG);
  return;
  }
goto pol_menu;    
}


void init_pol_graph(void)
{
float t1;
if (read_modepar_file(GRAPHTYPE_PG) == 0)
  {
pg_default:;
  dpg.xleft=27*text_width;
  dpg.xright=dpg.xleft+PGW;
  dpg.ytop=0.72*screen_height;
  dpg.ybottom=dpg.ytop+PGH;
  lir_status=LIR_NEW_POL;
  dpg.startpol=0;
  dpg.avg=8;
  dpg.angle=0;
  dpg.adapt=0;
  dpg.c1=1;
  dpg.c2=0;
  dpg.c3=0;
  return;
  }
if(dpg.check != PG_VERNR)goto pg_default;
if(dpg.avg < 1 || dpg.avg > 1000)goto pg_default;
t1=dpg.c1*dpg.c1+dpg.c2*dpg.c2+dpg.c3*dpg.c3;
if(t1 < 0.98 || t1 > 1.02)goto pg_default;
if(dpg.angle < 0 || dpg.angle > 180.00001)goto pg_default;
if( (dpg.adapt&-2) != 0)goto pg_default;
if(dpg.startpol < 0 || dpg.startpol > 179)goto pg_default;
pg=dpg;
pg_oldx=-10000;
pg_flag=1;
pol_graph_scro=no_of_scro;
make_pol_graph(FALSE);
no_of_scro++;
if(no_of_scro >= MAX_SCRO)lirerr(89);
}
