#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
int main(void)
{
wiringPiSetupGpio();
pinMode(16,OUTPUT);
while(1)
{
digitalWrite(16,HIGH);
delay(1000);
digitalWrite(16,LOW);
delay(1000);
}
}