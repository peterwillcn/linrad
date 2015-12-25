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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WUSERHWDR 0
#define WDEPS1 1
#define USEREXTRA 2
#define DEPS3 3
#define WUSERHWDEF 4
#define WDEPS4 5
#define WLINRAD_EXE 6
#define LLINRAD_EXE 7
#define LUSERHWDR 8
#define LUSERHWDEF 9
#define DEPS2 10
#define PERSEUS 11
#define M32 12
#define MAKE_SERVER 13
#define NO_OF_REPLACE 14

char *input_string[NO_OF_REPLACE]={"@WUSERHWDR@",          //0
                                   "@WDEPS1@",             //1
                                   "@USEREXTRA@",          //2
                                   "@DEPS3@",              //3
                                   "@WUSERHWDEF@",         //4
                                   "@WDEPS4@",             //5
                                   "@WLINRAD_EXE@",        //6
                                   "@LLINRAD_EXE@",        //7
                                   "@LUSERHWDR@",          //8
                                   "@LUSERHWDEF@",         //9
                                   "@DEPS2@",              //10
                                   "@PERSEUS@",            //11
                                   "@M32@",                //12
                                   "@MAKE_SERVER@"};       //13
                                   
char *output_string[NO_OF_REPLACE];

void replace(char *s, int *len)
{
int i, j, k;
int m,l1,l2;
for(j=0; j<NO_OF_REPLACE; j++)
  {
  l1=strlen(input_string[j]);
  l2=strlen(output_string[j]);
printf("\n%s -> %s",input_string[j],output_string[j]);
  m=l1-l2;
  i=0;
nxt_i:;  
  while(i <= len[0] && s[i]!=input_string[j][0])i++;
  k=0;
  while( k+i <= len[0] && 
         s[i+k]==input_string[j][k] && 
         input_string[j][k] != 0)k++;
  if(input_string[j][k] == 0)
    {
    len[0]-=m;
    if(m>0)
      {
      k=i;
      while(k < len[0])
        {
        s[k]=s[k+m];
        k++;
        }
      }
    else
      {
      k=len[0];
      while(k > i)
        {
        k--;
        s[k]=s[k+m];
        }
      }
    k=0;  
    while(k<l2)
      {
      s[i]=output_string[j][k];
      i++;
      k++;
      }
    }
  else
    {
    i++;
    }
  if(i<len[0])goto nxt_i;  
  }  
}

int main(int argc, char *argv[])
{
FILE *file;
char *s;
int i, j;
(void) *argv;
(void) argc;
s=malloc(0x40000);
if(s == NULL)
  {
  printf("\nERROR: could not allocate memory");
  exit(0);
  } 
for(j=0; j<NO_OF_REPLACE; j++)
  {
  output_string[j]="";
  }
// Set up the strings we want to replace the input strings with.
file=fopen("wusers_hwaredriver.c","r");
if(file != NULL)
  {
  output_string[WUSERHWDR]="1";
  output_string[WDEPS1]="wusers_hwaredriver.c";
  fclose(file);
  }
file=fopen("users_extra.c","r");
if(file != NULL)
  {
  output_string[USEREXTRA]="1";
  output_string[DEPS3]="users_extra.c";
  fclose(file);
  }
file=fopen("wusers_hwaredef.h","r");
if(file != NULL)
  {
  output_string[WUSERHWDEF]="1";
  output_string[WDEPS4]="wusers_hwaredef.h";
  fclose(file);
  }
output_string[WLINRAD_EXE]="linrad.exe";
output_string[LLINRAD_EXE]="for_linux";  
// ***************************************************
output_string[MAKE_SERVER]="0";
file=fopen("conf.h.in","r");
if(file == NULL)
  {
  printf("Could not open file: conf.h.in\n");
  exit(0);
  }
j=0;
i=1;
while(i != 0 && j< 0x3ffff)
  {  
  i=fread(&s[j],1,1,file);
  j++;
  }
j--;  
printf("\nReading %d bytes from conf.h.in",j);
fclose(file);
replace(s, &j);
file=fopen("conf.h","w");
if(file == NULL)
  {
  printf("\nCould not open conf.h for write");
  exit(0);
  }
i=fwrite(s,j,1,file);
fclose(file);
printf("\n%d bytes written to conf.h",j);
if(i!=1)
  {
  printf("\nERROR Buffer only written in part");
  exit(0);
  }
// ***************************************************
file=fopen("Makefile.in","r");
if(file == NULL)
  {
  printf("\nCould not open file: Makefile.in\n");
  exit(0);
  }
j=0;
i=1;
while(i != 0 && j< 0x3ffff)
  {  
  i=fread(&s[j],1,1,file);
  j++;
  }
j--;  
printf("\nReading %d bytes from Makefile.in",j);
fclose(file);
replace(s, &j);
file=fopen("Makefile","w");
if(file == NULL)
  {
  printf("\nCould not open Makefile for write");
  exit(0);
  }
i=fwrite(s,j,1,file);
fclose(file);
printf("\n%d bytes written to Makefile",j);
if(i!=1)
  {
  printf("\nERROR Buffer only written in part");
  exit(0);
  }
free(s);
return 0;  
}
