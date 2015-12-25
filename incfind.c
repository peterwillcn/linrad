
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
FILE *file;
int i,j,k;
if(argc < 3)
  {
nothing:;  
  return 0;
  }
i=0;
next:;
while(argv[2][i] != ':' && argv[2][i] != 0)i++;
if(argv[2][i] == 0)
  {
  return 0;
  }
k=i;
i++;
if(argv[2][i] == ' ' && 
   argv[2][i+1] == 'E' && 
   argv[2][i+2] == 'L' && 
   argv[2][i+3] == 'F' && 
   argv[2][i+4] == ' ')
  {  
  while(argv[2][k] != '/' && k>0)k--;
  if(k<=3)return 0;
  j=k-3;
  sprintf(&argv[2][j],"include");
  while(j>0 && argv[2][j-1] != 10)
    {
    j--;
    }
  if(argv[2][i+5] == '3' && 
     argv[2][i+6] == '2')
    {
    if(argv[1][0] == '1')
      {
      printf("-I%s",&argv[2][j]);
      return;
      }
    }
  else
    {
    if(argv[1][0] == '2')
      {
      printf("-I%s",&argv[2][j]);
      return;
      }
    }
  }  
goto next; 
}
