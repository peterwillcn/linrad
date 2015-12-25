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
#include "fft2def.h"
#include "screendef.h"
#include "seldef.h"


#define YU 0.0000001

#define RELEASE_FACTOR 1.15
#define MAX_LIMINFO_REG 40
#define SFAC 2.

void selfreq_liminfo(void)
{
int i, k, ia, ib;
float t1, t2;
float *old_liminfo;
old_liminfo=&liminfo[fft1_size];
if(mix1_selfreq[0] < 0)goto selfrx;
ia=mix1_selfreq[0]*fftx_points_per_hz;
k=baseband_bw_fftxpts*.7;
if(genparm[SECOND_FFT_ENABLE] != 0)
  {
  ia/=fft2_to_fft1_ratio;
  k/=fft2_to_fft1_ratio;
  if(k<3)k=3;
  }
ib=ia+k;
ia-=k;
if(ia<0)ia=0;
if(ib>=fft1_size)ib=fft1_size-1;
if(mg.scale_type == MG_SCALE_STON)
  {
  for(i=ia;i<=ib;i++)
    {
    liminfo[i]=-1;
    }
  }
else
  {  
  t1=0;
  t2=2;
  for(i=ia;i<=ib;i++)
    {
    if(liminfo[i]<0)t1=1;
    if(liminfo[i]>0)
      {
      if(t2>liminfo[i])t2=liminfo[i];
      }
    }  
// liminfo[i]  < 0  => bin to timf2_strong with amplitude = 1;
// liminfo[i] == 0  => bin to timf2_weak with amplitude = 1;
// liminfo[i]  > 0  => bin to timf2_strong with amplitude = liminfo[i];
  if(t2 > 1)
    {
// do nothing if liminfo is zero over the entire frequency range.
    if(t1 == 0)goto selfrx;
    t2=1;
    }
  t1=1/t2;
  t1*=sqrt((float)(fft2_size/fft1_size));
// öö removed aug 4 2011 
// öö put back again...
  if(swfloat)
    {
    t2=-1;
    }
  else
    {  
    if(t1 < 0x7fff/genparm[SELLIM_MAXLEVEL])t2=0;
    }
// If the signal is really strong, allow the limiter to work as usual
// with the same gain over the whole baseband.
// Otherwise clear liminfo for the selected passband.
  for(i=ia;i<=ib;i++)
    {
    liminfo[i]=t2;
    }
  }  
selfrx:;
// The blanker evaluates pulse amplitudes based on a reduced bandwidth
// because some frequency points are among strong signals while
// the reference pulse that we subtract is stored assuming the
// full bandwidth is present.
// Make liminfo_amplitude_factor suitable to compensate for the
// loss of amplitude due to the missing parts of the spectrum.
// At half the bandwidth the pulse power is reduced by a factor
// of two. The pulse is also lengthened by a factor of two so
// the amplitude is reduced by a factor of two, not 1.414 as one
// would think from the power ratio.
// If 50% of the frequencies are strong signals, skip the smart blanker.
if(genparm[SECOND_FFT_ENABLE] == 0 )
  {
  liminfo_amplitude_factor=1;
  }
else
  {
  if( (fft1_calibrate_flag&CALAMP)==CALAMP)
    {      
    t1=0;
    for(i=fft1_first_point; i<=fft1_last_point; i++)
      {
      if(liminfo[i]!=0)t1+=fft1_desired[i]*fft1_desired[i];
      }
    liminfo_amplitude_factor=fft1_desired_totsum/(fft1_desired_totsum-t1);  
    }
  else
    {  
    k=0;
    for(i=fft1_first_inband; i<=fft1_last_inband; i++)
      {
      if(liminfo[i]!=0)k++;
      }
    i=fft1_last_inband-fft1_first_inband+1;
    liminfo_amplitude_factor=(float)(i)/(i-k);  
    }
  }
if(liminfo_amplitude_factor > 2)liminfo_amplitude_factor=0;
for(i=0; i<fft1_size; i++) old_liminfo[i]=liminfo[i];    
}

void fft2_update_liminfo(void)
{
int i, j, k, ia, ib;
int nn;
float global_noise_floor, t1, t2, t3;
float *reg_noise, *reg_min;
unsigned wait_n;
reg_noise=&fftf_tmp[fft1_size];
reg_min=&reg_noise[liminfo_groups];
wait_n=1+(1+(fft2_blocktime*wg.waterfall_avgnum))/
                                             (wg.fft_avg1num*fft1_blocktime);
if(wait_n > 255)wait_n=255;
nn=fft2_size/fft1_size;
// Compute the average power of fft2 over the bin bandwidth of fft1
// and store in fftf_tmp
for(i=fft1_first_point; i<fft1_last_point; i++)
  {
  ia=nn*i;
  ib=ia+nn;
  t1=0;
  if(sw_onechan)
    {
    for(j=ia; j<ib; j++) 
      {
      t1+=fft2_powersum_float[j];
      }
    }
  else
    {
    for(j=ia; j<ib; j++) 
      {
      t1+=fft2_xysum[j].x2+fft2_xysum[j].y2;  
      }
    }  
  fftf_tmp[i]=t1*wg_waterf_yfac[i]/nn;
  } 
for(i=0; i<liminfo_groups; i++)
  {
  ia=i*liminfo_group_points;
  ib=ia+liminfo_group_points;
  t1=0;
  for(j=ia; j<ib; j++)t1+=fftf_tmp[j];
  t1/=liminfo_group_points;
// Exclude power levels that are 30 dB below the average 
// and find the smallest one of those that remain.
  k=0;
  t3=BIGFLOAT;
  t1*=0.001F;
  for(j=ia; j<ib; j++)
    {
    if(fftf_tmp[j] > t1 ) 
      {
      k++;
      if(fftf_tmp[j] < t3)t3=fftf_tmp[j];
      }
    }
  if(k < 2)
    {
    reg_min[i]=-1;
    }
  else
    {
    reg_min[i]=t3;
    }
  }
// compute a global noise floor as the average of the
// minimum values while excluding extremes.
t1=0;
for(i=0; i<liminfo_groups; i++)t1+=reg_min[i];
t1/=liminfo_groups;
global_noise_floor=0;
k=0;
for(i=0; i<liminfo_groups; i++)
  {
  if(reg_min[i] > 0.03F*t1 && reg_min[i] < 30.F*t1)
    {
    global_noise_floor+=reg_min[i];
    k++;
    }
  }
if(k < 3)goto fft2updx;
global_noise_floor/=k;    
// Set a limit and compute the average of points
// below the limit with in each group.
t1=5*global_noise_floor;  
for(i=0; i<liminfo_groups; i++)
  {
  ia=i*liminfo_group_points;
  ib=ia+liminfo_group_points;
  t2=0;
  k=0;
  for(j=ia; j<ib; j++)
    {
    if(fftf_tmp[j] < t1 ) 
      {
      k++;
      t2+=fftf_tmp[j];
      }
    }
  if(k > 2)
    {
    reg_noise[i]=t2/k;
    }
  else
    {
    reg_noise[i]=-1;
    }
  }    
t1=0;
k=0;
for(i=0; i<liminfo_groups; i++)
  {
  if(reg_noise[i] > 0)
    {
    k++;
    t1+=reg_noise[i];
    }
  }
if(k < 3)goto fft2updx;
t1/=k;
global_noise_floor=0;
k=0;
for(i=0; i<liminfo_groups; i++)
  {
  if(reg_noise[i] > 0.1F*t1 && reg_noise[i] < 10.F*t1)
    {
    global_noise_floor+=reg_noise[i];
    k++;
    }
  }
if(k < 3)goto fft2updx;
global_noise_floor/=k;
// Set a limit and move points above this limit to strong signals    
t1=0.5*hg.blanker_ston_fft2*global_noise_floor;
// The dumb blanker may generate noise in bins next to points
// flagged as strong signals.
// set such points to the neighbour value.
ia=fft1_first_point;
if(ia < 2)ia=2;
ib=fft1_last_point;
if(ib < fft1_size-2)ib=fft1_size-2;
k=0;
for(i=ia; i<ib; i++)
  {
  if(liminfo[i-1]==0 && liminfo[i]!=0)
    {
    if(fftf_tmp[i-1] > fftf_tmp[i-2])fftf_tmp[i-1]=fftf_tmp[i-2];
    }
  if(liminfo[i+1]==0 && liminfo[i]!=0)
    {
    if(fftf_tmp[i+1] > fftf_tmp[i+2])fftf_tmp[i+1]=fftf_tmp[i+2];
    }
  if(liminfo[i]!=0)k++;
  }
// If 25% or more of the points are strong, clear some of them.
if(k > (ib-ia)/4)
  {
  k=0;
  for(i=ia; i<ib; i++)
    {
    if(liminfo[i] < 0 && fftf_tmp[i] < t1)
      {
      liminfo[i]=0;
      liminfo_wait[i]=0;
      }
    if(liminfo[i] !=0)k++;   
    }  
  if(k > (ib-ia)/4)
    {
    t2=10.F*t1;
    k=0;
    for(i=ia; i<ib; i++)
      {
      if(liminfo[i] < 0 && fftf_tmp[i] < t2)
        {
        liminfo[i]=0;
        liminfo_wait[i]=0;
        }
      }
    }
  }      
// Locate a weak signal region.
for(i=ia; i<ib; i++)
  {
  if(SFAC*fftf_tmp[i-2] > t1 ||
     fftf_tmp[i-1] > t1 ||
     fftf_tmp[i] > t1 ||
     fftf_tmp[i+1] > t1 ||
     SFAC*fftf_tmp[i-2] > t1)
    {
    if(liminfo[i]==0)liminfo[i]=-1;
    liminfo_wait[i]=wait_n;
    }
  }
fft2updx:;
selfreq_liminfo();
}

void fft1_update_liminfo(void)
{
unsigned int wait_n;
int i,j,k,ia,ib,ja,jb,ix,iy;
float limit,maxval;
float t1,t2,t3,noise_floor;
float *old_liminfo;
old_liminfo=&liminfo[fft1_size];
fft1_sumsq_tot+=wg.fft_avg1num;
if(fft1_sumsq_tot > wg.spek_avgnum)fft1_sumsq_tot = wg.spek_avgnum;
// This routine is called if second fft is enabled.
// liminfo is an array used to split the fft1 signal into two
// signals: timf2_weak and timf2_strong.
// timf2_strong contains all strong narrow band signals while
// timf2_weak contains weak narrow band signals, noise, pulses and
// all other wide band interference.
// The timf2 signals are calculated by a back transformation from
// fft1.
// Each frequency is processed as follows: 
// liminfo[i]  < 0  => bin to timf2_strong with amplitude = 1;
// liminfo[i] == 0  => bin to timf2_weak with amplitude = 1;
// liminfo[i]  > 0  => bin to timf2_strong with amplitude = liminfo[i];
// We try to set liminfo equal over the full bandwidth of each strong
// signal.
// The gain reduction caused by positive liminfo values acts as an AGC
// that sets gain independently for each signal.
// We want the strong signal to fit in 16 bit, so it should be limited
// to below +/- 32656.
// To have some margin, set the limit at limit.
// In the worst case all energy is in one channel out of four (two arrays
// with complex data). 
// Then fft1_sumsq becomes limit*limit* wg.fft_avg1num
// We get the attenuation required as the square root 
// of  limit*limit*wg.fft_avg1num/fft1_sumsq[i] 
//
// Wait 1 second before making weak once a bin is classified as strong.
k=1+1/(wg.fft_avg1num*fft1_blocktime);
if(k < 255)wait_n=k; else wait_n=255;
//
//
// fft1_sumsq holds wg.fft_avg1num summed power spectra at the
// current location when we arrive here called by get fft1.
// If fft2_size is bigger, narrowband signals grow more!!
t1=genparm[SELLIM_MAXLEVEL];
limit=t1*t1*wg.fft_avg1num*ui.rx_rf_channels;
limit*=fft1_size;
limit/=fft2_size;
ia=fft1_sumsq_pa+fft1_first_point;
ix=ia;
iy=fft1_sumsq_pa+fft1_last_point-1;
limloop:;
if(fft1_sumsq[ia] > limit)
  {
  maxval=fft1_sumsq[ia];
  ib=ia+1;
  while(fft1_sumsq[ib] > limit && ib<=iy)
    {
    if(fft1_sumsq[ib] > maxval) maxval=fft1_sumsq[ib];
    ib++;
    }
  while(ia>ix && fft1_sumsq[ia-1]/fft1_sumsq[ia] < 0.3)ia--;
  while(ib<iy && fft1_sumsq[ib+1]/fft1_sumsq[ib] < 0.3)ib++;
  ja=ia-fft1_sumsq_pa;
  jb=ib-fft1_sumsq_pa;
  t1=liminfo[ja];
  for(j=ja+1; j<=jb; j++)
    {
    if(liminfo[j] > 0 && liminfo[j] < t1)t1=liminfo[j];
    }
  t2=sqrt(limit/maxval);
  if(t1/t2 > 0.1 && t1/t2 <10)
    {
    t2=0.8*t1+0.2*t1;
    }
  for(j=ja; j<=jb; j++)liminfo[j]=t2;
  t1=t2;
  j=1+(ib-ia)/4; 
  while(ia>ix && j>0)
    {
    j--;
    ia--;
    ja=ia-fft1_sumsq_pa;
    t1=pow(t1,0.9);
    if( liminfo[ja] <= 0 || liminfo[ja] > t1)
      {
      liminfo[ja]=t1;
      }
    else
      {  
      goto low_px;
      }
    }
low_px:;   
  j=1+(ib-ia)/4; 
  while(ib<iy && j>0)
    {
    j--;
    ib++;
    jb=ib-fft1_sumsq_pa;
    t2=pow(t2,0.9);
    if( liminfo[jb] <= 0 || liminfo[jb] > t1)
      {
      liminfo[jb]=t2;
      }
    else
      {  
      goto high_px;
      }
    }  
high_px:;
  ia=ib;
  }
else
  {
  liminfo[ia-fft1_sumsq_pa]=0;
  }  
ia++;
if( ia <iy )goto limloop;
if(fft1_sumsq_tot < wg.spek_avgnum)goto fft1updx;
// In order to get a good noise blanker function, it is nessecary
// to remove all sinewave like components all the way down to near the
// noise floor.
// This is trivial on VHF bands where there is a well defined noise
// floor of essentially white noise.
// Use the full dynamic range spectrum, the wide graph data stored
// in fft1_slowsum to find a noise floor for the current situation.
// Do that by dividing the spectrum into liminfo_groups segments and 
// find the minimum within each of them.
// Store these minimum values in liminfo_group_min.
ja=fft1_first_inband/liminfo_group_points;
jb=1+fft1_last_inband/liminfo_group_points;
if( (jb-ja)*liminfo_group_points > fft1_size)jb--;
ia=ja*liminfo_group_points;
ib=ia+liminfo_group_points;
for(j=ja; j<jb; j++)
  {
  t1=BIGFLOAT;
  t2=BIGFLOAT;
  t3=BIGFLOAT;
  for(i=ia; i<ib; i++)
    {
    fftt_tmp[i]=wg_waterf_yfac[i]*fft1_slowsum[i];
    if(fftt_tmp[i]<=t3)
      {
      if(fftt_tmp[i]<=t1)
        {
        t3=t2;
        t2=t1;
        t1=fftt_tmp[i];
        }
      else
        {
        if(fftt_tmp[i]<=t2)
          {
          t3=t2;
          t2=fftt_tmp[i];
          }
        else
          {
          t3=fftt_tmp[i];
          }
        }
      }      
    }
  liminfo_group_min[j]=0.3333333*(t1+t2+t3);
  ia+=liminfo_group_points;
  ib+=liminfo_group_points;
  }
// Get the average of the min values.
t1=0;
for(j=ja; j<jb; j++)t1+=liminfo_group_min[j];
t1/=jb-ja;
// Get the average of the points below twice the average of min values
// if avgnum is large.  
// If avgnum is small include larger values in the average.
k=0;
noise_floor=0;
t1*=2*(1+2./wg.spek_avgnum);
for(j=ja; j<jb; j++)
  {
  if(liminfo_group_min[j]<t1)
    {
    noise_floor+=liminfo_group_min[j];
    k++;
    }
  }
if(noise_floor < 0.0001)noise_floor=0.0001;
if(k!=0)
  {
// noise_floor is the noise floor if wg_spek_avgnum is very large.
// Picking the minimum as the average of the 3 smallest points
// underestimates the minimum noise floor by about 3 times (in power)
// if wg_spek_avgnum=1
  noise_floor*=(1+2./wg.spek_avgnum)/k;  
// We may want to know where the fft1 noise floor is at some later time.
// Calculate it as a RMS voltage expressed in bits.
// The power at each frequency comes from the sum of 2*ui.rx_rf_channels
// amplitudes. Get the amplitude for each one of them.
  fft1_noise_floor=noise_floor/(wg_waterf_yfac[0]*wg.spek_avgnum*
                                                       2*ui.rx_rf_channels);
  noise_floor*=hg.blanker_ston_fft1;
// Anything that is more than genparm[SELLIM_STON] times above 
// the noise floor is a signal.
// Set liminfo negative for the corresponding data points so they will
// be processed together with the strong signals.
  ia=0;
  while(ia < fft1_first_point || ia < 2)
    {
    if(liminfo[ia]==0)liminfo[ia]=-1;
    ia++;
    }
  while(fftt_tmp[ia]>noise_floor && ia<fft1_size)
    {
    if(liminfo[ia]==0)liminfo[ia]=-1;
    ia++;
    }
  while(4*fftt_tmp[ia+1]<fftt_tmp[ia] && ia<fft1_size)
    {
    ia++;
    if(liminfo[ia]==0)liminfo[ia]=-1;
    }
get_above:;
  while(fftt_tmp[ia]<=noise_floor && ia<fft1_last_point)ia++;
  if(ia<fft1_last_point)
    {
    ib=ia;
    if(liminfo[ia]==0)liminfo[ia]=-1;
    while( (SFAC*fftt_tmp[ib-1] < fftt_tmp[ib]|| 
               SFAC*SFAC*fftt_tmp[ib-2]
                                       <fftt_tmp[ib]) && ib>fft1_first_point)
      {
      ib--;
      if(liminfo[ib]==0)liminfo[ib]=-1;
      }
    while(fftt_tmp[ia+1]>noise_floor && ia<fft1_last_point)
      {
      ia++;
      if(liminfo[ia]==0)liminfo[ia]=-1;
      }
    if(ia!=fft1_last_point)
      {
      while( (SFAC*fftt_tmp[ia+1] < fftt_tmp[ia] || 
               SFAC*SFAC*fftt_tmp[ia+2] 
                                       < fftt_tmp[ia]) && ia<fft1_last_point)
        {
        ia++;
        if(liminfo[ia]==0)liminfo[ia]=-1;
        }
      ia++; 
      }
    if(ia<fft1_last_point)goto get_above;
    }
  }
if(ia > fft1_size-2)ia=fft1_size-2;
while(ia < fft1_size)
  {
  liminfo[ia]=-1;
  ia++;
  }
// In case we recently attenuated a frequency, do not allow
// the gain to grow too quickly. When two back transforms are
// joined there will be a transcient if the gain was different
// for a strong signal.
// We could solve this by making a gradual transfer between back
// transforms as is done with sin squared windows but it is a good
// idea anyway to keep the gain down for a little while at the
// frequency of a very strong signal. It may have been just a key up.
for(i=0; i<fft1_size; i++)
  {
  if(liminfo[i] != 0)
    {
    liminfo_wait[i]=wait_n;
    }
  else
    {  
    if(liminfo_wait[i] > 0)
      {
      liminfo_wait[i]--;
      }
    if(liminfo_wait[i] >0)liminfo[i]=-1;
    }
  if(old_liminfo[i]>0)
    {
    t1=old_liminfo[i]*RELEASE_FACTOR;
    if(t1 < 1)
      {
      if(liminfo[i]>0 && liminfo[i] > t1)liminfo[i]=t1;
      }
    }
  }          
//for(i=fft1_size/4;i<3*fft1_size/4;i++)liminfo[i]=-1;
fft1updx:;
selfreq_liminfo();
//for(i=0;i<fft1_size;i++)liminfo[i]=0;
//for(i=0;i<8192;i++)liminfo[i]=-1;
//for(i=4900;i<4950;i++)liminfo[i]=0;
liminfo[0]=0;
liminfo[1]=0;
liminfo[fft1_size-2]=0;
liminfo[fft1_size-1]=0;
}
