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


#include "globdef.h"
#include "uidef.h"
#include "fft1def.h"
#include "screendef.h"
#include "sigdef.h"
#include "rusage.h"
#include "thrdef.h"
#include "txdef.h"


#if OSNUM == OSNUM_LINUX
#include <unistd.h>

#include "conf.h"
#include "lconf.h"

#include "loadalsa.h"
extern snd_pcm_t *rx_ad_handle, *rx_da_handle;
extern snd_pcm_t *tx_ad_handle, *tx_da_handle;
int alsaread(snd_pcm_t *handle, char *buf, int type);

int zzrw;
#endif


#if OSNUM == OSNUM_WINDOWS
#include "wdef.h"


#endif


void test_rxad(void)
{
int rotor_index, show_rotor, max_show_rotor;
char s[80];
#if OSNUM == OSNUM_LINUX
int err;
#endif
#if OSNUM == OSNUM_WINDOWS
int k;
size_t local_rxadin1_newbuf_ptr;
size_t local_rxadin2_newbuf_ptr;
MMRESULT mmrs;
WAVEHDR *whdr1, *whdr2;
#endif
snd[RXAD].block_frames=ui.rx_ad_speed/50;
make_power_of_two(&snd[RXAD].block_frames);
snd[RXAD].framesize=2*ui.rx_ad_channels;
if( (ui.rx_input_mode&DWORD_INPUT) != 0) snd[RXAD].framesize*=2;
// Allocate a big buffer. The open routine might change the block size.
timf1_bytes=snd[RXAD].framesize*ui.rx_ad_speed;
make_power_of_two(&timf1_bytes);
timf1_bytemask=timf1_bytes-1;
timf1_char=malloc((size_t)timf1_bytes);
if(timf1_char==NULL)
  {
  lirerr(23658);
  return;
  }
snd[RXAD].interrupt_rate=(float)(ui.rx_ad_speed)/(float)snd[RXAD].block_frames;
snd[RXAD].block_bytes=snd[RXAD].framesize*snd[RXAD].block_frames;
timf1p_sdr=0;
timf1p_pa=0;
open_rx_sndin(FALSE);
snd[RXAD].open_flag=CALLBACK_CMD_ACTIVE;
lir_sched_yield();
if(kill_all_flag || rxad_status != LIR_OK)
  {
  lirerr(1418);
  goto rxadin_init_error;
  }
#if OSNUM == OSNUM_WINDOWS
if ( (ui.use_alsa&PORTAUDIO_RX_IN) != 0)
  {
// Do nothing, but clear some pointers to satisfy the compiler.
  local_rxadin1_newbuf_ptr=0;
  local_rxadin2_newbuf_ptr=0;
  }
else
  {      
  EnterCriticalSection(&protect_rxin1);
  local_rxadin1_newbuf_ptr=rxadin1_newbuf_ptr;
  LeaveCriticalSection(&protect_rxin1);
  if(ui.rx_addev_no >= 256)
    {
    EnterCriticalSection(&protect_rxin2);
    local_rxadin2_newbuf_ptr=rxadin2_newbuf_ptr;
    LeaveCriticalSection(&protect_rxin2);
    }
  else
    {
    local_rxadin2_newbuf_ptr=0;
    }
  }
#endif
rotor_index=0;
max_show_rotor=snd[RXAD].interrupt_rate/5;
show_rotor=0;
while(!kill_all_flag && soundcard_test_cmd_flag[RXAD] == THRFLAG_ACTIVE)
  {
  if(show_rotor == 0)
    {
    sprintf(s,"RXAD%c",rotor[rotor_index]);
    rotor_index=(rotor_index+1)&3;
    lir_text(24,screen_last_line,s);
    lir_refresh_screen();
    show_rotor=max_show_rotor;
    }
  show_rotor--;  
  if ( (ui.use_alsa&PORTAUDIO_RX_IN) != 0)
    {
    while (!kill_all_flag && timf1p_sdr == timf1p_pa)
      {
      lir_await_event(EVENT_PORTAUDIO_RXAD_READY);
      }
    }
  else
    {
#if OSNUM == OSNUM_LINUX
    if(ui.rx_addev_no < 256)
      {
      if( (ui.use_alsa&NATIVE_ALSA_USED)!=0)
        {
        err=alsaread(rx_ad_handle, timf1_char,RXAD);
        if(err != TRUE)
          {
          lir_text(0,screen_last_line-1,"ERROR");
          }
        }
      else
        {
        zzrw=read(rx_audio_in, timf1_char, (size_t)snd[RXAD].block_bytes);
        }
      }
    else
      {
      zzrw=read(rx_audio_in, timf1_char, (size_t)(snd[RXAD].block_bytes>>1));
      zzrw=read(rx_audio_in2, timf1_char, (size_t)(snd[RXAD].block_bytes>>1));
      }
    }
  soundcard_test_block_count[RXAD]++;
  }
#endif
#if OSNUM == OSNUM_WINDOWS
    EnterCriticalSection(&protect_rxin1);
    k=rxadin1_newbuf_ptr-local_rxadin1_newbuf_ptr;
    LeaveCriticalSection(&protect_rxin1);
    if(k<0)k+=snd[RXAD].no_of_blocks;
    if(k>=snd[RXAD].no_of_blocks-2)
      {
      no_of_rx_overrun_errors++;
      sprintf(s,"RX%s%d",overrun_error_msg,no_of_rx_overrun_errors);
      wg_error(s,WGERR_RXIN);
      }
    if(k<=1)
      {
next_event1:;    
      WaitForSingleObject(rxin1_bufready, INFINITE);
      }
    whdr1=(WAVEHDR*)rxadin1_newbuf[local_rxadin1_newbuf_ptr];
    if( (whdr1[0].dwFlags & WHDR_DONE) == 0)
      {
      wg_error("SOUND ERROR. PLEASE REPORT!!!!",WGERR_RXIN+1);
      goto next_event1;
      }
    if(ui.rx_addev_no < 256)
      {
      if(local_rxadin1_newbuf_ptr==rxadin1_newbuf_ptr)
        {
        no_of_rx_overrun_errors++;
        sprintf(s,"RX%s%d",overrun_error_msg,no_of_rx_overrun_errors);
        wg_error(s,WGERR_RXIN);
        }
      local_rxadin1_newbuf_ptr++;
      if(local_rxadin1_newbuf_ptr>=(unsigned)snd[RXAD].no_of_blocks)local_rxadin1_newbuf_ptr=0;
      mmrs=waveInAddBuffer(hwav_rxadin1,whdr1, sizeof(WAVEHDR));
      if(mmrs != MMSYSERR_NOERROR)
        {
        lirerr(1226);
        goto rxadin_error;
        }
      }
    else
      {
      EnterCriticalSection(&protect_rxin2);
      k=rxadin2_newbuf_ptr-local_rxadin2_newbuf_ptr;
      LeaveCriticalSection(&protect_rxin2);
      if(k<0)k+=snd[RXAD].no_of_blocks;
      if(k>=snd[RXAD].no_of_blocks-2)
        {
        no_of_rx_overrun_errors++;
        sprintf(s,"RX%s%d",overrun_error_msg,no_of_rx_overrun_errors);
        wg_error(s,WGERR_RXIN);
        }
      if(k<=1)
        {
next_event2:;      
        WaitForSingleObject(rxin2_bufready, INFINITE);
        }
      whdr2=(WAVEHDR*)rxadin2_newbuf[local_rxadin2_newbuf_ptr];
      if( (whdr2[0].dwFlags & WHDR_DONE) == 0)
        {
        wg_error("SOUND ERROR. PLEASE REPORT!!!!",WGERR_RXIN+1);
        goto next_event2;
        }
      if( (ui.rx_input_mode&DWORD_INPUT) == 0)
        {
        k=snd[RXAD].block_bytes>>3;
        }
      else
        {
        k=snd[RXAD].block_bytes>>4;
        }
      local_rxadin1_newbuf_ptr++;
      local_rxadin2_newbuf_ptr++;
      if(local_rxadin1_newbuf_ptr>=(unsigned)snd[RXAD].no_of_blocks)local_rxadin1_newbuf_ptr=0;
      if(local_rxadin2_newbuf_ptr>=(unsigned)snd[RXAD].no_of_blocks)local_rxadin2_newbuf_ptr=0;
      mmrs=waveInAddBuffer(hwav_rxadin1,whdr1, sizeof(WAVEHDR));
      if(mmrs != MMSYSERR_NOERROR)
        {
        lirerr(1232);
        goto rxadin_error;
        }
      mmrs=waveInAddBuffer(hwav_rxadin2,whdr2, sizeof(WAVEHDR));
      if(mmrs != MMSYSERR_NOERROR)
        {
        lirerr(1233);
        goto rxadin_error;
        }
      }
    }
  soundcard_test_block_count[RXAD]++;
  }
rxadin_error:;
#endif
close_rx_sndin();
rxadin_init_error:
free(timf1_char);
}

void test_rxda(void)
{
char s[80];
int rotor_index, show_rotor,max_show_rotor;
int indicator_sleep_us;
int i;
genparm[DA_OUTPUT_SPEED]=ui.rx_min_da_speed;
if(genparm[DA_OUTPUT_SPEED] < 8000)genparm[DA_OUTPUT_SPEED]=8000;
rx_daout_bytes=ui.rx_max_da_bytes;
rx_daout_channels=ui.rx_max_da_channels;
snd[RXDA].framesize=rx_daout_bytes*rx_daout_channels;
snd[RXDA].block_frames=(int)((float)genparm[DA_OUTPUT_SPEED]*.025F);
make_power_of_two(&snd[RXDA].block_frames);
snd[RXDA].block_bytes=snd[RXDA].block_frames*snd[RXDA].framesize;
min_delay_time=1.F;
if(ui.rx_dadev_no == ui.rx_addev_no)snd[RXDA]=snd[RXAD];
daout=malloc((size_t)(8*snd[RXDA].block_bytes));
daout_pa=0;
daout_px=0;
for(i=0; i<8*snd[RXDA].block_bytes; i++)
  {
  daout[i]=0;
  }
daout_bufmask=8*snd[RXDA].block_bytes-1;
open_rx_sndout();
if(kill_all_flag)return;
snd[RXDA].open_flag=CALLBACK_CMD_ACTIVE;
rotor_index=0;
indicator_sleep_us=30000/snd[RXDA].interrupt_rate;
max_show_rotor=snd[RXDA].interrupt_rate;
show_rotor=0;
while(!kill_all_flag &&
      soundcard_test_cmd_flag[RXDA] == THRFLAG_ACTIVE)
  {
  if(show_rotor == 0)
    {
    sprintf(s,"RXDA%c",rotor[rotor_index]);
    rotor_index=(rotor_index+1)&3;
    lir_text(32,screen_last_line,s);
    lir_refresh_screen();
    show_rotor=max_show_rotor;
    }
  show_rotor--;  
// Write data to the output device.
  lir_rx_dawrite();
  lir_sleep(indicator_sleep_us);
  soundcard_test_block_count[RXDA]++;
  }
close_rx_sndout();
free(daout);
}

void test_txda(void)
{
char s[80];
int rotor_index, show_rotor,max_show_rotor;
int indicator_sleep_us;
int i;
snd[TXDA].framesize=ui.tx_da_bytes*ui.tx_da_channels;
snd[TXDA].block_frames=(int)((float)ui.tx_da_speed*0.025F);
make_power_of_two(&snd[TXDA].block_frames);
snd[TXDA].block_bytes=snd[TXDA].block_frames*snd[TXDA].framesize;
min_delay_time=1.F;
snd[TXDA].interrupt_rate=(float)ui.tx_da_speed/
                                       (float)snd[TXDA].block_frames;
open_tx_sndout();
if(txda_status != LIR_OK)
  {
  lirerr(1407);
  }
if(kill_all_flag)return;
txout_bufsiz=8*snd[TXDA].block_bytes;
txout_mask=txout_bufsiz-1;
txout_px=0;
txout=malloc((size_t)txout_bufsiz*sizeof(float));
txout_sizhalf=snd[TXDA].block_bytes/snd[TXDA].framesize;
txout_fftsize=2*txout_sizhalf;
rotor_index=0;
indicator_sleep_us=30000/snd[TXDA].interrupt_rate;
max_show_rotor=snd[TXDA].interrupt_rate;
show_rotor=0;
if(txout == NULL)
  {
  lirerr(1245231);
  return;
  }
for(i=0; i<snd[TXDA].block_bytes; i++)
  {
  txout[i]=0;
  }
snd[TXDA].open_flag=CALLBACK_CMD_ACTIVE;
while(!kill_all_flag &&
      soundcard_test_cmd_flag[TXDA] == THRFLAG_ACTIVE)
  {
  if(show_rotor == 0)
    {
    sprintf(s,"TXDA%c",rotor[rotor_index]);
    rotor_index=(rotor_index+1)&3;
    lir_text(40,screen_last_line,s);
    lir_refresh_screen();
    show_rotor=max_show_rotor;
    }
  show_rotor--;  
// Write data to the output device.
  lir_tx_dawrite((char*)txout);
  lir_sleep(indicator_sleep_us);
  soundcard_test_block_count[TXDA]++;
  }
close_tx_sndout();
free(txout);
}

int get_mic_speed(int *line, int min, int max)
{
int i,mm;
char s[80];
line[0]++;
sprintf(s,"Select microphone A/D speed %d to %d",min, max);
lir_text(5,line[0],s);
i=0;
while(s[i] != 0)i++;
mm=lir_get_integer(i+7, line[0], 6, min, max);
line[0]++;
return mm;
}

void get_tx_enable(void)
{
int line;
if( ((ui.rx_addev_no >=0 && ui.rx_addev_no < SPECIFIC_DEVICE_CODES) &&
                      (ui.network_flag&NET_RX_INPUT) == 0) ||
     (ui.rx_dadev_no >=0 && ui.rx_dadev_no < SPECIFIC_DEVICE_CODES))
  {
  line=5;
get_enable:;
  clear_screen();
  if(kill_all_flag)return;
  lir_text(4,line,"Enable Tx while running receive modes ? (Y/N) =>");
  await_processed_keyboard();
  if(lir_inkey == 'Y')
    {
    ui.tx_enable=1;
    line+=2;
    lir_text(0,line,"Parameters for speech processor, keying, etc. have to");
    line++;
    lir_text(0,line,"be set up and saved before Tx becomes enabled in Rx modes.");
    line++;
    }
  else
    {
    if(lir_inkey != 'N')goto get_enable;
    ui.tx_enable=0;
    }
  }
else
  {
  ui.tx_enable=0;
  }
}
