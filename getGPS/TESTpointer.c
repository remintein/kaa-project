#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
char result[100];
int i=0;
char data[100]="khong can thiet";
char *GPS_data_p;
char* get_GPS_sample(void);
int main(void)
{
GPS_data_p=get_GPS_sample();
printf(GPS_data_p);
return 0;
}

char* get_GPS_sample(void)
{
	for(i=0;i<100;i++) result[i]='\0';
	
	for(i=0;;i++)
	{
	if(data[i]=='\0') break;
	result[i]=data[i];
	}
	
    return result;
}