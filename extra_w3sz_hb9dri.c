// program extra.c for Linrad
// This demo program displays the following information in the two upper lines
// on the Linrad 'receive mode' screens:
// -the selected frequency
// -the selected mode
// -the selected bandwith
// -the selected input device
// -the selected network-modes
// -the UTC date and time
//
//  Pierre/ON5GN
//
// UPDATE HISTORY
//   7 sept  2004  first version for linrad 01.24
//  23 may   2010  BASEB16 and BASEB24 network-modes added
//                 layout changes
//                 documentation updates
//  27 june  2010  input device information added
//  
// INSTALLATION
//
// To install under Linux for Linux:
//   download lirxx-xx.tbz and extract it to your linrad directory
//   rename extra.c to users_extra.c in your linrad directory 
//   run ./configure and  make xlinrad (or make linrad) from your linrad directory
// 
// To generate a windows version under Linux:
//   install mingw32 from Leif's website
//   download lirxx-xx.tbz and extract it to your linrad directory
//   rename extra.c to users_extra.c in your linrad directory 
//   run ./configure and  make linrad.exe from your linrad directory
//
// To generate a windows version under Windows:
//   Install mingw and nasm from Leif's website in the C:\MinGW  directory
//   Download lirxx-xx.zip and unpack the linrad sources in your linrad
//   directory
//   rename extra.c to users_extra.c in your linrad directory 
//   Start a MS-DOS Command Window on your computer: click the Windows Start
//   menu and enter the "cmd.exe" command into the run box
//   In the MS-DOS Command Window issue a "cd" command to your linrad
//   directory, followed by a "configure.exe" and "make.bat" command
//
//*********************************************************
// 
// Here are the instructions for setting up Linrad so that Slave Instances of Linrad track the Master Linrad.
// The Master and Slave instances of Linrad are compiled differently, and of course must exist in separate directories.
// 
// The software as written assumes that the master and slave are on the same computer.  If they are not, then you must
// set up a drive from one of the computers as a Network Drive on the other, so that it can be referenced with a drive
// letter such as C: or D: or E: or whatever from the remote computer.  I have used the program with a Network Drive,
// so I know that using a Network Drive presents no issues with this software.
// 
// There are two versions of the Master software, one in the directory /lir04-05_Master, and one in the directory
// /lir04-05_Master_Immediate_QSY.
// 
// The software in the directory ...Master works like this:
// 1.  When you type "U" on the master instance of Linrad, it will write the QSO frequency to a file.
// 2.  The slave instance of Linrad will read that file and QSY to that frequency.
// 
// The software in the directory ...Master_Immediate_QSY works like this:
// 1.  When you change frequency, the master instance of Linrad immediately writes the QSO frequency to a file.
// 2.  The slave instance of Linrad will read that file and QSY to that frequency.
// 
// The Master Linrad instance writes file containing the QSO frequency information to the directory named
// \lin2ft\ on the disk it is on. 
// The file name of the file with the QSO frequency information is
// \Radio0file
// So the complete address of the QSO frequency information file is \lin2ft\Radio0file.
// 
// 
// The Slave Linrad instance looks in the Slave Linrad directory for the file freq_location.txt
// That file tells the Slave Linrad where to look for the QSO frequency information file.
// This is necessary and not hardwired into the software because the Disk on which this file
// is located will be different on different systems.  For example, on my system the QSO frequency information file
// in on disk F.  So the file freq_location.txt on my system contains the single line:
// F:\\lin2ft\\Radio0file
// indicating that the QSO information file has the address F:\lin2ft\Radio0file.  However, on your system the same file
// may be located on disk C:, in which case the frequency information file needs to contain the line:
// C:\\lin2ft\Radio0file
// 
// Note that because of the way the C language handles the "\" character, you need to place two backslashes for every backslash needed.
// Thus F:\\lin2ft\\Radio0file must be written into the frequency information file instead of F:\lin2ft\Radio0file
// 
// The Master Linrad file is compiled using the file users_w3sz_hb9dri.C and renaming it wusers_hwaredriver.c 
// if using windows or users_hwaredriver.c if using Linux.
// THE EXTRA_W3SZ FILE IS NOT USED FOR THE MASTER.
// Then "configure" and "make" and Master Linrad should be ready to go.
// 
// The Slave Linrad is compiled using the file extra)w3sz_hb9dri.c and renaming it to users_extra.c
// THE USERS_W3SZ_HB9DRI.C FILE IS NOT USED FOR THE SLAVE.	
// Then "configure" and "make" and Slave Linrad should be ready to go.
// 
// I attached a zip file of the master files and a zip file of the slave files.  Just unzip them and proceed as outlined above.
// Make sure to create a directory \lin2ft\ on the drive on which the Master Linrad is placed.
// Make sure to create the freq_location.txt file in each Slave Linrad directory, and write the location of the
// QSO frequency file in that file as outlined above.
// 
// When running, the command window on the Slave instance of Linrad will first indicate that it found the QSO frequency file
// [assuming that it did find it], and then display frequency information with each "U" QSY (or immediately,
// if you are using the software version from the directory ...Master_Immediate_QSY.
// The information is:
// [a] QSO frequency, [b] number of Hz from the bottom of the window, [c] number of Hz from the center frequency, [[d] the
// result of a calculation that should always equal the QSO frequency, [a].
// [b] - [a] should always equal 48000.  
// Thus this line provides 2 internal checks of the data.
// Below are two examples of the output:
// 
// F:\lir04-05_Slave>linrad
// Found F:\\lin2ft\\Radio0file
//  144170567 93567 45567  144170567
//  144159692 82692 34692  144159692
//  144088255 11255 -36745  144088255
// 
// F:\lir04-05_Slave>linrad
// Found F:\\lin2ft\\Radio0file
//  144088255 11255 -36745  144088255
//  144134755 57755 9755  144134755
//  144098192 21192 -26808  144098192
// 
// Let me know if there are any problems, if you need further info, or if you want me to change the program from what is currently written.
// 
// 
// Roger Rehr
// 1 March, 2015
//***********************************************

#include <time.h>
#include <string.h>
#include "screendef.h"

double users_extra_time;
double users_extra_fast_time;
extern float users_extra_update_interval;
int set_freq_flag;


int w3szfreq; //w3sz
int w3szfreqold=0; //w3sz
int w3sz_map65_flag=0; //w3sz
extern int w3sz_offset_hz; //w3sz
extern int w3sz_offset_hz_old; //w3sz
extern int w3sz_users_flag; //w3sz
char freqlocfile[80]="freq_location.txt";//w3sz
char *w3szfreqfile[80]; //w3sz
FILE *Fp; //w3sz
FILE *Fp1;//w3sz
char w3sz_buffer[80];//w3sz
char w3sz_buffer2[80];//w3sz
char w3sz_trash[80];//w3sz
char freq_loc[80];//w3sz

void init_users_extra(void)
{
char s[80];
// This routine is called just before a receive mode is entered.
// Use it to set your own things from the keyboard.
// Remove the comment statements below for testing purposes 
/*
int line,col;
char s[80];
col=1;
line=2;
sprintf(s,"THIS ROUTINE init_users_extra IS CALLED JUST BEFORE A RECEIVE MODE IS ENTERED");
lir_text(col,line,s);
sprintf(s,"USE IT TO SET YOUR OWN THINGS FROM THE KEYBOARD");
lir_text(col,line+2,s);
sprintf(s,"HIT ANY KEY TO CONTINUE");
lir_text(col,line+4,s);
await_processed_keyboard();
*/
//
// Set users_extra_update_interval to the highest rate at which
// you want your routines called.
// The users_extra routines will be called only when the CPU is idle
// and if the time interval you have specified is exceeded.
users_extra_update_interval=0.25;
Fp=fopen(freqlocfile,"r");//w3sz
if(Fp == NULL)//w3sz
{
	printf("Cannot locate the Master Frequency locator file\n");//w3sz
	*w3szfreqfile="\\lin2ft\\Radio0file";//w3sz
	Fp=fopen(freqlocfile,"w"); //w3sz
	if(Fp == NULL)//w3sz
	{
	clear_screen();
	sprintf(s,"Cannot open to write: %s", freqlocfile);//w3sz
    lir_text(0,5,s);
    lir_text(0,7,
		"Hit escape and create file freq_location.txt in the Linrad directory");//w3sz
    await_processed_keyboard();
	return;//w3sz
	}
	fprintf(Fp,"\\\\lin2ft\\\\Radio0file");//w3sz
	fclose(Fp);//w3sz
}
else//w3sz
{
	fgets(freq_loc,80,Fp);//w3sz
	fclose(Fp);//w3sz
	*w3szfreqfile=freq_loc;//w3sz
	printf("Found %s \n",*w3szfreqfile);//w3sz
	} 
	
users_extra_time=current_time();
users_extra_fast_time=recent_time;
set_freq_flag=FALSE;
}

void users_extra(void)
{
//w3sz the addition below checks the file w3szfreqfile and if it has changed, it changes main display cursor frequency
char s[120];

if (w3sz_users_flag==0) //w3sz
	{
		w3sz_offset_hz =0; //w3sz
		w3sz_offset_hz_old=0; //w3sz
	}


Fp1=fopen(*w3szfreqfile,"r"); //w3sz
if(Fp1 == NULL)//w3sz
{
printf("Cannot locate %s\n",*w3szfreqfile);//w3sz
Fp1=fopen(*w3szfreqfile,"w"); //w3sz
if(Fp1 == NULL)//w3sz
{
sprintf(s,"Cannot open to write: %s", *w3szfreqfile);//w3sz
clear_screen();
lir_text(0,5,s);
sprintf(s,"Hit escape and make sure %s is correctly written",*w3szfreqfile);//w3sz
lir_text(0,7,s);
sprintf(s,"in the file %s \n",freqlocfile);//w3sz
    lir_text(0,8,s);
    await_processed_keyboard();
    return;
}
fprintf(Fp1,"Radio0file was missing\n");//w3sz
fprintf(Fp1,"Radio0file was missing\n");//w3sz
fprintf(Fp1,"Radio0file was missing\n");//w3sz
fprintf(Fp1,"Radio0file was missing\n");//w3sz
fprintf(Fp1,"%.0f fQSO\n",remainder(fg.passband_center,1)*1000);//w3sz
fprintf(Fp1,"fQSO2\n");//w3sz
fclose(Fp1);//w3sz
}
else //w3sz
{
	
	fgets(w3sz_buffer,80,Fp1);//w3sz
	
	if  (w3sz_buffer !=NULL)//w3sz
	{
	w3szfreq=atoi(w3sz_buffer);//w3sz
	}
	fclose(Fp1);//w3sz
  set_freq_flag=TRUE;
}
}



void users_extra_fast(void)
{
int post;
float t1;
int w3sz_diff=-96000;//w3sz
if((w3szfreq != w3szfreqold) || (w3sz_offset_hz != w3sz_offset_hz_old)) //w3sz
{
	new_mix1_curx[0]=-1;//w3sz

	w3sz_diff = w3szfreq - fg.passband_center; //w3sz
		if (w3sz_diff >= -48000 && w3sz_diff <= 48000) //w3sz
			{
			t1=w3sz_diff + 48000 + w3sz_offset_hz; //w3sz  w3sz_offset_hz is offset in Hz from users_w3sz.c file parameter box
			printf(" %d %.0f %.3f  %.0f %d\n",w3szfreq, t1, fg.passband_center, t1-48000+fg.passband_center*1000000,w3sz_offset_hz); //w3sz added
			}
		else
			{
			printf("Selected Frequency is out of Range\n");//w3sz
			printf("%d\n",w3szfreq);//w3sz
			printf(" %d %.3f %d\n",w3szfreq, fg.passband_center,w3sz_offset_hz); //w3sz added
			t1=48000;//w3sz
			}

w3szfreqold = w3szfreq; //w3sz
w3sz_offset_hz_old = w3sz_offset_hz;//w3sz
set_freq_flag=TRUE;

post=make_new_signal(0, t1);//w3sz

if(post)
  {
  sc[SC_FREQ_READOUT]++;
  if(genparm[SECOND_FFT_ENABLE] != 0)sc[SC_HG_FQ_SCALE]++;
  if(genparm[AFC_ENABLE]==0)sc[SC_BG_FQ_SCALE]++;
  }
if(leftpressed != BUTTON_RELEASED)
  {
  if(post)awake_screen();
  return;
  }
if(post)
  {  
  if(genparm[AFC_ENABLE]!=0)sc[SC_BG_FQ_SCALE]++;
  awake_screen();
  }
  
leftpressed=BUTTON_IDLE;  
baseb_reset_counter++;
mouse_active_flag=0;
set_freq_flag=FALSE;
}
}

