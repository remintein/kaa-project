#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
int SerialPort;
int i=0;
int in=0;

char GPS_data_p[500];

char Mau[6]="$GPRMC";
static unsigned short tp_days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};
void get_GPS_sample(void);

void init_module_GPS(void){

	char c;
	int out=0;
	digitalWrite(16,LOW);	
	printf("waiting for the module GPS...\n");
	fflush(stdout);
	while(1)
	{
		digitalWrite(16,HIGH);	
		while(serialDataAvail(SerialPort))
		{
		c=serialGetchar(SerialPort);
		printf("%c",c);
		if(c=='3') out=1;
		fflush(stdout);
		}
		if(out==1) break;
	}
		digitalWrite(16,LOW);
	printf("module GPS ready\n");
	printf("Send AT+GPS=1\n");		
	serialPuts(SerialPort, "AT+GPS=1\r\n");
	serialFlush(SerialPort);
	
	printf("waiting for OK..\n");
	out=0;
	while(1)
	{
		while(serialDataAvail(SerialPort))
		{
			c=serialGetchar(SerialPort);
			if(c=='K') out=1;
			printf("%c",c);			
			fflush(stdout);
		}
		if(out==1) break;
	}
	printf("Receive: OK\n");
	printf("Send AT+GPSRD=1\n");
	serialPuts(SerialPort,"AT+GPSRD=1\r\n");
	serialFlush(SerialPort);
} 

int main(void)
{
	wiringPiSetupGpio();
	pinMode(16,OUTPUT);
	if((SerialPort = serialOpen ("/dev/ttyAMA0",115200)) <0 )
		{
		fprintf(stderr,"Unable to open serial device: %s\n", strerror(errno)) ;
		}
	else	
		{
		serialPuts(SerialPort,"AT+CGDATA\r\n");
		serialFlush(SerialPort);
		char c=serialGetchar(SerialPort);
		printf("%c",c);
		
		init_module_GPS();
		
		}
	while(1)
	{
	get_GPS_sample();
	delay(1000);
	}
	return 1;
}

void get_GPS_sample(void)
{
	int in=0;
	int count=0;
	char c;
	char FaC[100];
	char time[12];
	char La[12];
	char Lo[13];
	char dir1[3];
	char dir2[3];
	char date[8];
	char speed[5];
	int f_index=0;
	int fla=0;
	int j=0;
	int n_data=0;
	for(i=0;i<500;i++) GPS_data_p[i]='\0';
	while(serialDataAvail(SerialPort))
	{
	c=serialGetchar(SerialPort);
	GPS_data_p[in]=c;
	in++;
	if(in==500) break;
	fflush(stdout);
	}
	
	for(i=15;;i++)
	{
		if(GPS_data_p[i]=='\0') break;
		if(GPS_data_p[i]==Mau[j]) j++;
		if(j>=6) fla=1;
		if(fla==1)
		{
		if(GPS_data_p[i]=='$') break;
		FaC[f_index]=GPS_data_p[i];
		f_index++;
		}
	}

	for(i=0;FaC[i]!='\0';i++)
	{
	if(FaC[i]=='V') {n_data=1;}
	if(FaC[i]==',') { count++;in=0;}
	switch(count)
	{
		case 1: {time[in]=FaC[i];
			in++;
			break;}
		case 3: {La[in]=FaC[i];
			in++;
			break;}
		case 4: {dir1[1]=FaC[i];
			break;}
		case 5: {Lo[in]=FaC[i];
			in++;
			break;}
		case 6: {dir2[1]=FaC[i];
			break;}
		case 7: {speed[in]=FaC[i];
			in++;
			break;}
		case 9: {date[in]=FaC[i];
			in++;
			break;}
		default : delay(10);
	}
	}
	time[0]=' ';
	La[0]=' ';
	Lo[0]=' ';
	date[0]=' ';
	speed[0]=' ';
	dir1[0]=' ';
	dir2[0]=' ';
	time[11]='\0';
	La[11]='\0';
	Lo[12]='\0';
	dir1[2]='\0';
	dir2[2]='\0';
	date[7]='\0';
	speed[4]='\0';
	unsigned int time_sample=(int)strtod(time,NULL);
	unsigned int date_sample=(int)strtod(date,NULL);
	int tp_year=date_sample%100+2000;
	int tp_date=date_sample/10000;
	int tp_month=(date_sample-tp_date*10000)/100;
	
	int tp_hour=time_sample/10000;
	int tp_minute=(time_sample-tp_hour*10000)/100;
	int tp_second=time_sample%100;
	printf("Time Sample: %d/%d/%d ; %d:%d:%d",tp_date,tp_month,tp_year,tp_hour,tp_minute,tp_second);
	int inc=0;
	for(in=1970;in<=tp_year;in++)
	{
		if((in%4==0)||((in%100==0)&&(in%400==0)))
			inc++;
	} 
	unsigned long TimeStamp=(((((((tp_year/4)*4-1970)*365+inc-2+tp_days[tp_year%4][tp_month-1]+tp_date)*24)+tp_hour)*60+tp_minute)*60+tp_second);
	double speed_d = strtod(speed,NULL)*1.852;
	double Lati=strtod(La,NULL);
	double Long=strtod(Lo,NULL);
	    int h1 = (int)Lati/ 100;
            int m1 = (int)Lati - h1*100;
            float s1 = Lati - h1*100 - m1;
            s1 *= 60;
            char dataLati[30];
	    snprintf(dataLati,sizeof(dataLati),"%d°%d\'%f\"%s",h1,m1,s1,dir1);

	    int h2 = (int)Long / 100;
            int m2 = (int)Long - h2*100;
            float s2 = Long - h2*100 - m2;
            s2 *= 60;
            char dataLong[30];
	    snprintf(dataLong,sizeof(dataLong),"%d°%d\'%f\"%s",h2,m2,s2,dir2);

	printf("\n");
	if(n_data==0)
	{
	printf("Time Stamp: %lu",TimeStamp);
	printf("\n");
	printf(dataLati);
	printf("\n");
	printf(dataLong);	
	printf("\n");
	printf("speed: ");
	printf("%lf",speed_d);
	printf("\n");
	printf("*********************************************************\n");
	}
	else

	printf("No data GPS");
}
