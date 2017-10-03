#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <time.h>

int in=0;
int inc=0;


int main ()
{
  
  //unsigned long TimeStamp=(((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
  //printf("Time Stamp: %lu",TimeStamp);
	for(in=1970;in<=2017;in++)
	{
		if((in%4==0)||((in%100==0)&&(in%400==0)))
			inc++;
	}
	printf("days: %d   %d",47*365+inc,inc);
  return 0;
}