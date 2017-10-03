#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <time.h>

int in=0;
int day=27;
int month=2;
int year=1995;
int hour=0;
int minute=0;
int second=0;
int inc=0;
static unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};



int main ()
{
  for(in=1970;in<=year;in++)
	{
		if((in%4==0)||((in%100==0)&&(in%400==0)))
			inc++;
	} 
// inc: so nam nhuan <=> so ngay cong them
  unsigned long TimeStamp=(((((((year/4)*4-1970)*365+inc-2+days[year%4][month-1]+day)*24)+hour)*60+minute)*60+second);
  printf("Time Stamp: %lu",TimeStamp);
  return 0;
}