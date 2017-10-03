#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <kaa.h>
#include <platform/kaa_client.h>
#include <kaa_error.h>
#include <kaa_configuration_manager.h>
#include <kaa_logging.h>
#include <gen/kaa_logging_gen.h>
#include <platform/kaa_client.h>
#include <utilities/kaa_log.h>
#include <platform-impl/common/ext_log_upload_strategies.h>
#include <string.h>
#ifdef __cplusplus
 extern "C" {
#endif

#include <wiringPi.h>
#include <wiringSerial.h>
#include <errno.h>

#ifdef __cplusplus
 }
#endif

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
int id=50;
static int32_t sample_period;
static time_t  last_sample_time;
extern kaa_error_t ext_unlimited_log_storage_create(void **log_storage_context_p, kaa_logger_t *logger);
static void init_module_GPS(void){
	
	char c;
	int out=0;
	wiringPiSetupGpio();
	pinMode(16,OUTPUT);
	if((SerialPort = serialOpen ("/dev/ttyAMA0",115200)) <0 )
		{
		fprintf(stderr,"Unable to open serial device: %s\n", strerror(errno)) ;
		}
	else{
	printf("waiting for the module GPS...\n");
	while(1)
	{
		digitalWrite(16,HIGH);	
		while(serialDataAvail(SerialPort))
		{
		c=serialGetchar(SerialPort);
		if(c=='3') out=1;
		fflush(stdout);
		}
		if(out==1) break;
	}
		digitalWrite(16,LOW);
	printf("module GPS ready\n");		
	serialPuts(SerialPort, "AT+GPS=1\r\n");
	serialFlush(SerialPort);
	out=0;
	while(1)
	{
		while(serialDataAvail(SerialPort))
		{
			c=serialGetchar(SerialPort);
			if(c=='K') out=1;		
			fflush(stdout);
		}
		if(out==1) break;
	}
	serialPuts(SerialPort,"AT+GPSRD=1\r\n");
	serialFlush(SerialPort);
	}
}


/* Periodically called by Kaa SDK. */
static void example_callback(void *context)
{
	int in=0;
	int count=0;
	char c;
	char FaC[100];
	char time_d[7];
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

    time_t current_time = time(NULL);
    /* Respect sample period */
    if (difftime(current_time, last_sample_time) >= sample_period) {
	
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
		case 1: {time_d[in]=FaC[i];
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
	time_d[0]=' ';
	La[0]=' ';
	Lo[0]=' ';
	date[0]=' ';
	speed[0]=' ';
	dir1[0]=' ';
	dir2[0]=' ';
	time_d[7]='\0';
	La[11]='\0';
	Lo[12]='\0';
	dir1[2]='\0';
	dir2[2]='\0';
	date[7]='\0';
	speed[4]='\0';
	unsigned int time_sample=(int)strtod(time_d,NULL);
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
	printf("\n*********************************************\n");
	
	}
	else printf("No data GPS");
        last_sample_time = current_time;
        kaa_user_log_record_t *log_record = kaa_logging_data_collection_create();
	log_record->time =TimeStamp;
	log_record->latitude =kaa_string_move_create(dataLati,NULL);
	log_record->longtitude =kaa_string_move_create(dataLong,NULL);
	log_record->speed =speed_d;
	log_record->id =id;
	
        kaa_logging_add_record(kaa_client_get_context(context)->log_collector, log_record, NULL);
    }
}
/* Receives new configuration data. */
static kaa_error_t on_configuration_updated(void *context, const kaa_root_configuration_t *conf)
{
    (void) context;
    printf("Received configuration data. New sample period: %i seconds\n", conf->sample_period);
    sample_period = conf->sample_period;
    return KAA_ERR_NONE;
}
int main(void)
{
	wiringPiSetupGpio();
	pinMode(16,OUTPUT);
	init_module_GPS();
    /* Init random generator used to generate temperature */
    srand(time(NULL));
    /* Prepare Kaa client. */
    kaa_client_t *kaa_client = NULL;
    kaa_error_t error = kaa_client_create(&kaa_client, NULL);
    if (error) {
        return EXIT_FAILURE;
    }
    /* Configure notification manager. */
    kaa_configuration_root_receiver_t receiver = {
        .context = NULL,
        .on_configuration_updated = on_configuration_updated
    };
    error = kaa_configuration_manager_set_root_receiver(
        kaa_client_get_context(kaa_client)->configuration_manager,
        &receiver);
    if (error) {
        return EXIT_FAILURE;
    }
    /* Obtain default configuration shipped within SDK. */
    const kaa_root_configuration_t *dflt = kaa_configuration_manager_get_configuration(
        kaa_client_get_context(kaa_client)->configuration_manager);
    printf("Default sample period: %i seconds\n", dflt->sample_period);
    sample_period = dflt->sample_period;
    
    /* Configure data collection. */
    void *log_storage_context         = NULL;
    void *log_upload_strategy_context = NULL;
    /* The internal memory log storage distributed with Kaa SDK. */
    error = ext_unlimited_log_storage_create(&log_storage_context,
        kaa_client_get_context(kaa_client)->logger);
    if (error) {
        return EXIT_FAILURE;
    }
    /* Create a strategy based on timeout. */
    error = ext_log_upload_strategy_create(
        kaa_client_get_context(kaa_client), &log_upload_strategy_context,
        KAA_LOG_UPLOAD_BY_TIMEOUT_STRATEGY);
    if (error) {
        return EXIT_FAILURE;
    }
    /* Strategy will upload logs every 5 seconds. */
    error = ext_log_upload_strategy_set_upload_timeout(log_upload_strategy_context, 5);
    if (error) {
        return EXIT_FAILURE;
    }
    /* Specify log bucket size constraints. */
    kaa_log_bucket_constraints_t bucket_sizes = {
         .max_bucket_size       = 500,   /* Bucket size in bytes. */
         .max_bucket_log_count  = 2,    /* Maximum log count in one bucket. */
    };
    /* Initialize the log storage and strategy (by default, they are not set). */
    error = kaa_logging_init(kaa_client_get_context(kaa_client)->log_collector,
        log_storage_context, log_upload_strategy_context, &bucket_sizes);
    if (error) {
        return EXIT_FAILURE;
    }
    
    /* Start Kaa SDK's main loop. example_callback is called once per second. */
    error = kaa_client_start(kaa_client, example_callback, kaa_client, 1);
    /* Should get here only after Kaa stops. */
    kaa_client_destroy(kaa_client);
    
    if (error) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
