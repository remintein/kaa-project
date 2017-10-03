#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
int SerialPort;
char result[100];
int i=0;
char CREG[10]="+CREG: 3";
char OK[2]="OK";
int in=0;
char result1[100]="khong can thiet";
char result2[100];
int cpstr(char *needle, char *haystack, int len_haystack)
{
	int j=0;
	int i;
		for (i = 0;; i++)
		{
			if (*(needle + i)=='\0') break;
			if (*(needle + i) == *(haystack + j)) j++;
			if (j == (len_haystack-1))  return 0;
			
		} 
	 return 1;

}
int main()
{

if(cpstr(&result1,&CREG,8))
printf("\nkhong giong");
else
printf("\ntuyet voi");
}