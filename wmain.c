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

#define _WIN32_WINNT 0x0502

#include <ctype.h>
#include "globdef.h"
#include "uidef.h"
#include "rusage.h"
#include "thrdef.h"
#include "wdef.h"
#include "screendef.h"
#include "vernr.h"
#include "options.h"
#include "keyboard_def.h"


#include "conf.h"

int mbutton_state;
int newcomer_escflag;
int allow_quit_flag;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
char inpout32_dllname[80];
int i, j, ws, hs, winc, hinc;
char s[80];
static char szAppName[] = PROGRAM_NAME;
MSG msg;
WNDCLASSEX wndclass;
LARGE_INTEGER lrgi;
LPBITMAPINFO lpbi;
LPVOID *m_pBits;
HINSTANCE h_inpout;
DWORD procmask, sysmask;
RECT winrect;
(void)hPrevInstance;
double dt1;
long old;
allow_quit_flag=FALSE;
m_pBits=NULL;
if(DUMPFILE)
  {
  dmp = fopen("dmp", "wc");
  DEB"\n******************************\n");
  }
else
  {
  dmp=NULL;
  }
ui_setup();
first_mempix=0x7fffffff;
last_mempix=0;
os_flag=OS_FLAG_WINDOWS;
// Set up things we need to evaluate CPU load.
CurrentProcess=GetCurrentProcess();
j=GetProcessAffinityMask(CurrentProcess,
                             (PDWORD_PTR)&procmask, (PDWORD_PTR)&sysmask);
if(j==0)
  {
  no_of_processors=1;
  }
else
  {
  no_of_processors=0;
  for(i=0; i<32; i++)
    {
    no_of_processors+=procmask&1;
    procmask/=2;
    }
  }
newcomer_escflag=FALSE;
switch(ui.process_priority)
  {
  case 1:
  SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
  break;
  
  case 2:
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  break;
  
  case 3:
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  break;
  }
serport=INVALID_HANDLE_VALUE;
init_os_independent_globals();
keyboard_buffer_ptr=0;
keyboard_buffer_used=0;
keyboard_buffer=malloc(KEYBOARD_BUFFER_SIZE*sizeof(int));
if(GetWindowRect(GetDesktopWindow(), &desktop_screen) == 0)
  {
  printf("\nGetWindowRect(...) failed");
  exit(0);
  }
wndclass.cbSize        = sizeof (wndclass);
wndclass.style         = CS_HREDRAW | CS_VREDRAW;
wndclass.lpfnWndProc   = WndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = hInstance;
wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
wndclass.hCursor       = LoadCursor (NULL, IDC_CROSS);
wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
wndclass.lpszMenuName  = NULL;
wndclass.lpszClassName = szAppName;
wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);
RegisterClassEx (&wndclass);
ws=desktop_screen.right-desktop_screen.left;
hs=desktop_screen.bottom-desktop_screen.top;
if(ui.screen_width_factor > 0)
  {
  screen_width=(ui.screen_width_factor*ws/100);
  screen_height=ui.screen_height_factor*hs/100;
  }
else
  {
  screen_width=-ui.screen_width_factor;
  screen_height=-ui.screen_height_factor;
  if(screen_width>ws)screen_width=ws;
  if(screen_height>hs)screen_height=hs;
  }
winrect.top=0;
winrect.left=0;
winrect.right=screen_width;
winrect.bottom=screen_height;
AdjustWindowRect(&winrect, WS_OVERLAPPEDWINDOW, FALSE);
winc=winrect.right-winrect.left-screen_width;
hinc=winrect.bottom-winrect.top-screen_height;
if(screen_width+winc > ws)screen_width=ws-winc;
if(screen_height+hinc > hs)screen_height=hs-hinc;
screen_width &= -4;
screen_totpix=screen_width*(screen_height+1);
#if IA64 == 0
sprintf(s," 32bit");
#else
sprintf(s," 64bit");
#endif
sprintf(&s[6]," %s %s",PROGRAM_NAME,szCmdLine);
linrad_hwnd = CreateWindow (szAppName, s,
                     WS_OVERLAPPEDWINDOW,
                     0,0,
                     screen_width+winc,
                     screen_height+hinc,
                     NULL, NULL, hInstance, NULL);
if(linrad_hwnd == NULL)
  {
  printf("\nCreateWindow(...) failed");
  exit(0);
  }
ShowWindow (linrad_hwnd, iCmdShow);

UpdateWindow (linrad_hwnd);
screen_hdc=GetDC(linrad_hwnd);
rxad_mme_open_flag1=0;
rxad_mme_open_flag2=0;
txad_mme_open_flag=0;
// ****************************************************************
// Try to install inpout32.dll to try to get access to the
// parallel port for hardware control.
SetDllDirectory(DLLDIR);
#if IA64 == 0
sprintf(inpout32_dllname,"%sinpout32.dll",DLLDIR);
#else
sprintf(inpout32_dllname,"%sinpout64.dll",DLLDIR);
#endif
h_inpout=LoadLibraryEx(inpout32_dllname, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
if(h_inpout == NULL)
  {
  parport_installed=FALSE;
  }
else
  {
  parport_installed=TRUE;
// get the addresses of the functions
  inp32 = (inpfuncPtr) GetProcAddress(h_inpout, "Inp32");
  if(inp32 == NULL)
    {
    lirerr(1197);
    goto wmain_error;
    }
  oup32 = (oupfuncPtr) GetProcAddress(h_inpout, "Out32");
  if (oup32 == NULL)
    {
    lirerr(1197);
    goto wmain_error;
    }
  }
// *****************************************************************
// Set the variables Linrad uses to access the screen.
init_font(ui.font_scale);
if(lir_errcod != 0)goto wmain_error;
// ******************************************************************
// Rearrange the palette. It was designed for svgalib under Linux
for(i=0;i<3*256;i++)
  {
  svga_palette[i]<<=2;
  }
for(i=0;i<3*256;i+=3)
  {
  j=svga_palette[i];
  svga_palette[i]=svga_palette[i+2];
  svga_palette[i+2]=j;
  }
// ***************************************************************
// Create a BITMAP in our own memory space. The SetPixel function
// of WinAPI os hopelessly slow.
lpbi = malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
lpbi->bmiHeader.biWidth = screen_width;
lpbi->bmiHeader.biHeight = screen_height+1;
lpbi->bmiHeader.biPlanes = 1;
lpbi->bmiHeader.biBitCount = 8;
lpbi->bmiHeader.biCompression = BI_RGB;
lpbi->bmiHeader.biSizeImage = 0;
lpbi->bmiHeader.biXPelsPerMeter = 0;
lpbi->bmiHeader.biYPelsPerMeter = 0;
lpbi->bmiHeader.biClrUsed = 0;
lpbi->bmiHeader.biClrImportant = 0;
for (i=0; i<255; i++)
  {
  lpbi->bmiColors[i].rgbRed=svga_palette[3*i];
  lpbi->bmiColors[i].rgbGreen=svga_palette[3*i+1];
  lpbi->bmiColors[i].rgbBlue=svga_palette[3*i+2];
  lpbi->bmiColors[i].rgbReserved=0;
  }
memory_hbm= CreateDIBSection(screen_hdc, lpbi, DIB_RGB_COLORS,
                               m_pBits, NULL, 0 );
free(lpbi);
// ****************************************************************
// Create a device context for the memory bitmap.
memory_hdc=CreateCompatibleDC(screen_hdc);
SelectObject(memory_hdc, memory_hbm);
// ***************************************************************
// Initiate our timer
if(QueryPerformanceFrequency(&lrgi)==0)lirerr(1215);
internal_clock_frequency=
     (double)(0x10000)*(double)(0x10000)*lrgi.HighPart+lrgi.LowPart;
if(QueryPerformanceCounter(&lrgi)==0)lirerr(1216);
internal_clock_offset=lrgi.HighPart;
mouse_cursize=0;
mbutton_state=0;
lir_init_event(EVENT_KILL_ALL);
rxin1_bufready=NULL;
thread_handle_kill_all = CreateThread(NULL, 0, winthread_kill_all,
                                      NULL,0, &thread_id_kill_all);
if(thread_handle_kill_all == NULL)
  {
  lirerr(1201);
  goto wmain_error;
  }
lir_init_event(EVENT_REFRESH_SCREEN);
thread_handle_refresh_screen = CreateThread(NULL, 0, winthread_refresh_screen,
                                      NULL,0, &thread_id_refresh_screen);
expose_time=current_time();  
i=check_mmx();
mmx_present=i&1;
if(mmx_present != 0)simd_present=i/2; else simd_present=0;
GetObject(memory_hbm,sizeof(memory_bm),&memory_bm);
if(memory_bm.bmType != 0)lirerr(10003);
mempix=memory_bm.bmBits;
lir_status=LIR_OK;
// Create a thread for the main menu.
lir_init_event(EVENT_KEYBOARD);
lir_init_event(EVENT_SYSCALL);
lir_init_event(EVENT_MANAGE_EXTIO);
users_open_devices();
if(kill_all_flag) goto wmain_error;
thread_handle_main_menu = CreateThread(NULL, 0, winthread_main_menu,
                                      NULL,0, &thread_id_main_menu);
if(thread_handle_main_menu == NULL)lirerr(1201);
lir_init_event(EVENT_MOUSE);
thread_handle_mouse = CreateThread(NULL, 0, winthread_mouse,
                                     NULL,0, &thread_id_mouse);
if(thread_handle_mouse == NULL)lirerr(1201);
translate_loop:;
while ( GetMessage (&msg, NULL, 0, 0) )
  {
  if(msg.message == WM_USER)
    {
    switch(msg.wParam)
      {
      case EXTIO_COMMAND_LOAD_DLL:
      extio_error=load_extio_library();
      break;
    
      case EXTIO_COMMAND_UNLOAD_DLL:
      if(!kill_all_flag)
        {  
        unload_extio_library();
        }
      break;
        
      case EXTIO_COMMAND_START:
      start_extio();
      lir_sched_yield();
      if(kill_all_flag)
        {
        lir_refresh_screen();
        lir_sleep(4000000);
        }
      break;
    
      case EXTIO_COMMAND_STOP:
      stop_extio();
      break;

      case EXTIO_COMMAND_KILL_ALL:
      goto kill;
      }  
    extio_command_flag=EXTIO_COMMAND_DONE;
    }
  else
    {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
    }
  }
lir_sched_yield();
if(!allow_quit_flag)goto translate_loop;  
kill:;
if(extio_running != 0)stop_extio();
if(extio_handle != NULL)unload_extio_library();
ReleaseSemaphore(rxin1_bufready,1,&old);
while(!kill_all_flag) lir_sleep(3000);
ReleaseSemaphore(rxin1_bufready,1,&old);
store_in_kbdbuf(0);
dt1=current_time();
while(threads_running && current_time()-dt1 < 1) lir_sleep(3000);
if( current_time()-dt1  < 0.99)
  {
  WaitForSingleObject(&thread_id_kill_all,INFINITE);
  }
WaitForSingleObject(thread_handle_main_menu, 0);
refresh_screen_flag=FALSE;
lir_sched_yield();
lir_refresh_screen();
WaitForSingleObject(thread_handle_refresh_screen,INFINITE);
lir_remove_mouse_thread();
WaitForSingleObject(thread_handle_mouse,INFINITE);
wmain_error:;
users_close_devices();
lir_close_serport();
ReleaseDC (linrad_hwnd, screen_hdc);
if(dmp!=NULL)fclose(dmp);
if(parport_installed)
  {
  FreeLibrary(h_inpout);
  }
lir_close_event(EVENT_KILL_ALL);
lir_close_event(EVENT_SYSCALL);
lir_close_event(EVENT_KEYBOARD);
lir_close_event(EVENT_MANAGE_EXTIO);
lir_close_event(EVENT_MOUSE);
lir_close_event(EVENT_REFRESH_SCREEN);
fprintf( stderr,
 "\nIf your system hangs here, use the X on this cmd window to stop Linrad");
return lir_errcod;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
int i, cc;
HDC hdc;
PAINTSTRUCT  ps;
int keydown_type;
int k, m;
int mx, my;
switch (iMsg)
  {
  case WM_CREATE:
// We could initiate things here rather than in the main program (???).
  return 0;

  case WM_SIZE:
  first_mempix=0;
  last_mempix=screen_totpix-1;
  break;

  case WM_PAINT:
  expose_time=recent_time;
  hdc=BeginPaint (linrad_hwnd, &ps);
  if( thread_status_flag[THREAD_SCREEN] != THRFLAG_SEM_WAIT &&
      thread_status_flag[THREAD_SCREEN] != THRFLAG_ACTIVE &&
      thread_status_flag[THREAD_SCREEN] != THRFLAG_IDLE)
      {
      BitBlt(hdc, 0, 0, screen_width, screen_height, memory_hdc, 0, 0, SRCCOPY);
      }
  EndPaint (linrad_hwnd, &ps);
  return 0;

  case WM_CHAR:
  if(newcomer_escflag)
    {
    cc=to_upper((int)wParam);
    if(cc=='Y')goto escexit;
    if(cc!='N')break;
    newcomer_escpress(1);
    newcomer_escflag=FALSE;
    break;
    }
  if( wParam==27 && ui.operator_skil == OPERATOR_SKIL_NEWCOMER  && !kill_all_flag)
    {
    newcomer_escpress(0);
    newcomer_escflag=TRUE;
    break;
    }
  if( wParam==27)
    {
escexit:;
    if(kill_all_flag)
      {
      if(lir_inkey==1)store_in_kbdbuf(2);
      return 0;
      }
    lir_set_event(EVENT_KILL_ALL);
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
    allow_quit_flag=TRUE;
    PostQuitMessage(0);
    return 0;
    }
  store_in_kbdbuf((int)wParam);
  break;

  case WM_SYSCHAR:
  i=toupper((int)wParam)-'A'+ALT_A_KEY;
  store_in_kbdbuf(i);
  break;

  case WM_KEYUP:
  keydown_type=(lParam>>31)&1;
  if(keydown_type == 1 && wParam==FUNCTION_SHIFT)shift_key_status=0;
  break;

  case WM_KEYDOWN:
  keydown_type=lParam>>24;
  cc=0;
  if((keydown_type&1)!=0)
    {
    switch(wParam)
      {
      case NUMBER_HOME:
      cc=HOME_KEY;
      break;

      case NUMBER_UP:
      cc=ARROW_UP_KEY;
      break;

      case NUMBER_PGUP:
      cc=PAGE_UP_KEY;
      break;

      case NUMBER_LEFT:
      cc=ARROW_LEFT_KEY;
      break;

      case NUMBER_RIGHT:
      cc=ARROW_RIGHT_KEY;
      break;

      case NUMBER_END:
      cc=END_KEY;
      break;

      case NUMBER_DOWN:
      cc=ARROW_DOWN_KEY;
      break;

      case NUMBER_PGDN:
      cc=PAGE_DOWN_KEY;
      break;

      case NUMBER_INSERT:
      cc=INSERT_KEY;
      break;

      case NUMBER_DELETE:
      cc=DELETE_KEY;
      break;
      }
    }
  else
    {
    if((keydown_type&32)==0)
      {
      if(wParam == FUNCTION_SHIFT)
        {
        shift_key_status=1;
        return 0;
        }
      if(wParam >=96 && wParam <=105)return 0;
      switch(wParam)
        {
        case NUMERIC_0:
        cc='0';
        break;

        case NUMERIC_1:
        cc='1';
        break;

        case NUMERIC_2:
        cc='2';
        break;

        case NUMERIC_3:
        cc='3';
        break;

        case NUMERIC_4:
        cc='4';
        break;

        case NUMERIC_5:
        cc='5';
        break;

        case NUMERIC_6:
        cc='6';
        break;

        case NUMERIC_7:
        cc='7';
        break;

        case NUMERIC_8:
        cc='8';
        break;

        case NUMERIC_9:
        cc='9';
        break;
        }
      if(shift_key_status == 0)
        {
        switch(wParam)
          {
          case FUNCTION_F1:
          cc=F1_KEY;
          break;

          case FUNCTION_F2:
          cc=F2_KEY;
          break;

          case FUNCTION_F3:
          cc=F3_KEY;
          break;

          case FUNCTION_F4:
          cc=F4_KEY;
          break;

          case FUNCTION_F5:
          cc=F5_KEY;
          break;

          case FUNCTION_F6:
          cc=F6_KEY;
          break;

          case FUNCTION_F7:
          cc=F7_KEY;
          break;

          case FUNCTION_F8:
          cc=F8_KEY;
          break;

          case FUNCTION_F9:
          cc=F9_KEY;
          break;

          case FUNCTION_F11:
          cc=F11_KEY;
          break;

          case FUNCTION_F12:
          cc=F12_KEY;
          break;
          }
        }
      else
        {
        switch(wParam)
          {
          case FUNCTION_F1:
          cc=SHIFT_F1_KEY;
          break;

          case FUNCTION_F2:
          cc=SHIFT_F2_KEY;
          break;

          case FUNCTION_F3:
          cc=SHIFT_F3_KEY;
          break;

          case FUNCTION_F4:
          cc=SHIFT_F4_KEY;
          break;

          case FUNCTION_F5:
          cc=SHIFT_F5_KEY;
          break;

          case FUNCTION_F6:
          cc=SHIFT_F6_KEY;
          break;

          case FUNCTION_F7:
          cc=SHIFT_F7_KEY;
          break;

          case FUNCTION_F8:
          cc=SHIFT_F8_KEY;
          break;
          }
        }
      }
    }
  if(cc != 0)store_in_kbdbuf(cc);
  break;

  case WM_MOUSEWHEEL:
  k=GET_WHEEL_DELTA_WPARAM(wParam);
  if(mbutton_state==0)
    {
    if(k<0)
      {
      step_rx_frequency(1);
      }
    else
      {
      step_rx_frequency(-1);
      }
    }
  else
    {
    m=bg.wheel_stepn;
    if (k<0)
      {
      m++;
      if(m>30)m=30;
      }
    else
      {
      m=bg.wheel_stepn;
      m--;
      if(m<-30)m=-30;
      if(genparm[AFC_ENABLE]==0 && m<0)m=0;
      }
    bg.wheel_stepn=m;
    sc[SC_SHOW_WHEEL]++;
    make_modepar_file(GRAPHTYPE_BG);
    }
  break;

  case WM_MBUTTONDOWN:
  mbutton_state=1;
  break;

  case WM_MBUTTONUP:
  mbutton_state=0;
  break;

  case WM_LBUTTONDOWN:
  new_lbutton_state=1;
  goto mousepost; 

  case WM_LBUTTONUP:
  new_lbutton_state=0;
  goto mousepost;

  case WM_RBUTTONDOWN:
  new_rbutton_state=1;
  goto mousepost;

  case WM_RBUTTONUP:
  new_rbutton_state=0;
  goto mousepost;

  case WM_MOUSEMOVE:
  mx=new_mouse_x;
  my=new_mouse_y;
  new_mouse_x= LOWORD (lParam);
  new_mouse_y= HIWORD (lParam);
  if(new_mouse_x >= screen_width)new_mouse_x=screen_width-1;
  if(new_mouse_y >= screen_height)new_mouse_y=screen_height-1;
  if(new_mouse_x < 0)new_mouse_x=0;
  if(new_mouse_y < 0)new_mouse_y=0;
  if(  mx == new_mouse_x &&   my==new_mouse_y)break;
  if( (mx == new_mouse_x && new_mouse_x == screen_width-1) || 
      (my == new_mouse_y && new_mouse_y == screen_height-1) ||
      (mx == new_mouse_x && new_mouse_x == 0) || 
      (my == new_mouse_y && new_mouse_y == 0))break;
mousepost:;
  lir_set_event(EVENT_MOUSE);
  break;

  case WM_DESTROY:
  goto escexit;
  }
return DefWindowProc (hwnd, iMsg, wParam, lParam);
}

void ui_setup(void)
{
FILE *file;
int i, j, k;
char s[10];
char chr;
int *uiparm;
char *parinfo;
uiparm=(int*)(&ui);
parinfo=NULL;
file = fopen(userint_filename, "rb");
if (file == NULL)
  {
  printf("\n\nWELCOME TO LINRAD");
  printf("\nThis message is not an error, but an indication that setup");
  printf("\nhas not yet been done.");
  printf("\n\nSetup file %s missing.",userint_filename);
full_setup:;
  for(i=0; i<MAX_UIPARM; i++) uiparm[i]=0;
  welcome_msg();
  while(fgets(s,8,stdin)==NULL);
  chr=to_upper(s[0]);
  if(chr != 'S' && chr != 'N' && chr != 'E') exit(0);
  if(chr == 'N')
    {
    ui.operator_skil=OPERATOR_SKIL_NEWCOMER;
    }
  else
    {
    if(chr == 'S')
      {
      ui.operator_skil=OPERATOR_SKIL_NORMAL;
      }
    else
      {
      ui.operator_skil=OPERATOR_SKIL_EXPERT;
      }
    }
  win_global_uiparms(0);
  if(kill_all_flag) exit(0);
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
  if(parinfo==NULL)
    {
    parinfo=malloc(4096);
    if(parinfo==NULL)
      {
      lirerr(1078);
      fclose(file);
      return;
      }
    }
  i=fread(parinfo,1,4095,file);
  fclose(file);
  file=NULL;
  if(i >= 4095)
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
    while(parinfo[k]!='[')k++;
    sscanf(&parinfo[k],"[%d]",&uiparm[i]);
    while(parinfo[k]!='\n')k++;
    }
  if(ui.screen_width_factor*ui.screen_height_factor <=0)goto go_full_setup;
  if(ui.screen_width_factor > 0)
    {
    if(ui.screen_width_factor < 25 || 
      ui.screen_width_factor > 100 ||
      ui.screen_height_factor <25 || 
      ui.screen_height_factor > 100)goto go_full_setup;
    }
  else
    {
    if(ui.screen_width_factor < -10000 || 
      ui.screen_width_factor > -500 ||
      ui.screen_height_factor <-10000 || 
      ui.screen_height_factor > -400)goto go_full_setup;
    }
  if( ui.font_scale < 1 || 
      ui.font_scale >5 ||
      ui.process_priority < 0 || 
      ui.process_priority > 3 ||
      ui.check != UI_VERNR ||
      ui.operator_skil < OPERATOR_SKIL_NEWCOMER ||
      ui.operator_skil > OPERATOR_SKIL_EXPERT ||
      ui.rx_soundcard_radio < RX_SOUNDCARD_RADIO_UNDEFINED ||
      ui.rx_soundcard_radio >= MAX_RX_SOUNDCARD_RADIO)
    {
go_full_setup:;    
    printf("\n\nSetup file %s has errors",userint_filename);
    goto full_setup;
    }
  uiparm_change_flag=FALSE;
  }
free(parinfo);
}


