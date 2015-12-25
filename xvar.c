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


#include <pthread.h>
#include <semaphore.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include "globdef.h"
#include "rusage.h"
#include "thrdef.h"
#include "lconf.h"
#include "xdef.h"
#include "ldef.h"

#if SHM_INSTALLED == 1
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
XShmSegmentInfo *shminfo;
#endif
int X11_accesstype;

pthread_t thread_identifier_process_event;
pthread_t thread_identifier_refresh_screen;
unsigned char *mempix_char;
unsigned short int *mempix_shi;
int first_mempix;
int last_mempix;
int process_event_flag;
int expose_event_done;
int shift_key_status;
int alt_key_status;
int color_depth;
unsigned char *xpalette;


GC xgc;
XImage *ximage;
Display *xdis;
Window xwin;
Colormap lir_colormap;

