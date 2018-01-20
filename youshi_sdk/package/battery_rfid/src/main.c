
/**
 * @
 *
 * @
 */

#include "main.h"

int rfid_run = 0;
list_t * rfids = NULL;

pthread_t rfid_test_tid;
pthread_mutex_t rfid_test_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t rfid_read_tid;
pthread_mutex_t rfid_read_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t upload_tid;
pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;

void getNowTime(void);

static void* rfid_test_mode(void* arg);

static void* read_mcu_handler(void* arg);

static void* rfid_upload(void *arg);

static void* rfid_read(void* arg);

static void* rfid_worker(void* arg);

static inline void usage(void) 
{
	fprintf(stderr, "Usage:\n%s\n", help_info);
    exit(1);
}

static void sig(int sig)
{
	DEBUG_PRINT("Get SIGINT.\n");

	rfid_run = 0;

	pthread_cancel(rfid_read_tid);
	
	if (NULL != rfids)
	{
		rfid_list_destroy(rfids);
		rfids = NULL;
	}

}

int main(int argc, char* argv[])
{
	char port[32] = {0};
	//char * dev = NULL;
	char dev[64] = {0};
	char wbuf[64] = {0};
	int start_flag = 0, rflag = 0, wflag = 0;
	int status_rflag = 0;
	int c = 0;
	int test_flag = 0;
	int work_flag = 0;

	int port_num = PORT_MAX; //default max = 8
	int frame_size = 8; //default max = 8k

	uint32_t mqtt_port = 0;
	uint8_t mqtt_host[32] = {0};

	struct thread_info tinfo; 
	unsigned long int p = 0;

	memset(&tinfo, 0, sizeof(struct thread_info));

    if ( argc <= 1) {
		usage();
		return -1;
	}

    while ((c = getopt(argc, argv, "hHDvrtSsdk:o:f:p:P:w:")) != -1) {
		switch (c) {
			case 's': // start read in thread (PORT_MAX)
				start_flag = 1;
				break;
			case 'D': // set process to deamon
				//TODO: set process to deamon
				break;
			case 'v': // set precess debug level 
				//TODO: set process debug 
				break;
			case 't': // get system time
				test_flag = 1;
				//getNowTime();
				break;
			case 'h': // help info
			case 'H':
				usage();
				break;
			case 'r': // read datas one time for all ports (PORT_MAX)
				work_flag = 1;
				break;
			case 'S': // read status one time for all ports (PORT_MAX)
				status_rflag = 1;
				break;
			case 'd': // read data from dev
				//malloc ?
				memcpy(dev, optarg, strlen(optarg));
				if (dev[0] == '\0')
					usage();
				break;
			case 'w':// write datas one time for all ports (PORT_MAX)
				wflag = 1;
				memcpy(wbuf, optarg, strlen(optarg));
				if (strlen(optarg) >= 64)
					;//printf too long
				break;
			case 'f':
				frame_size = strtoul(optarg, NULL, 0);
				if (frame_size != 4 && frame_size != 8) {
					printf("frame size error , opton [4|8]. default is 8.");
					break;
				}
				break;
			case 'p':
			case 'P': // port num 
				p = strtoul(optarg, NULL, 0);
				if (p < 1 && p > 8) {
					printf("port num error !");
					break;
				}
				port_num = p;
				break;
			case 'k':// write datas one time for all ports (PORT_MAX)
				mqtt_port = strtoul(optarg, NULL, 0);
				if (frame_size != 4 && frame_size != 8) {
					printf("frame size error , opton [4|8]. default is 8.");
					break;
				}
				break;
			case 'o':
				memcpy(mqtt_host, optarg, strlen(optarg));
				//fprintf(stderr, "para mqtt host:%s , len %d\n", mqtt_host, strlen(mqtt_host));
				if (strlen(optarg) >= 16)
					;//printf too long
				break;
			default:
				usage();
		}
    }

	if (optind < argc) {
		printf("non-option: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");

		usage();

		return -1;
	}
#if 0
	if (status_rflag != 1 && rflag != 1 && wflag != 1 && start_flag != 1) {
		usage();

		fprintf(stderr, "options error !\n");

		return -1;
	}
#endif


	tinfo.port_num = port_num;
	tinfo.frame_size = frame_size;
	tinfo.addr.port = mqtt_port;

	tinfo.addr.ssize = strlen(mqtt_host);

	memcpy(tinfo.addr.saddr, mqtt_host, strlen(mqtt_host));
#if 1
	// setup list
	//list_t * rfids = rfid_list_init();
	rfids = rfid_list_init();

	tinfo.rfids = rfids;
#endif

	if (test_flag == 1) {

		DEBUG_PRINT("Start RFID test console.\n");

		signal(SIGINT, sig);

		if (pthread_mutex_init(&rfid_test_mutex, NULL) < 0) {
			fprintf(stderr, "Init thread mutex error.\n");
			return -1;
		}

		if (pthread_create(&rfid_test_tid, NULL, rfid_test_mode, &tinfo) < 0) {
			fprintf(stderr, "Create rfid test mode thread failed.\n");
			return -1;
		}

		/* wait SIGINT signal to exit */
		pthread_join(rfid_test_tid, NULL);

		printf("rfid console exit.\n");
		pthread_mutex_destroy(&rfid_test_mutex);

		return 0;
	}

	if (work_flag == 1) {

		rfid_run = 1;

		while(rfid_run)
		{

			DEBUG_PRINT("Start RFID read thread.\n");

			signal(SIGINT, sig);
#if 0
			if (pthread_mutex_init(&rfid_read_mutex, NULL) < 0) {
				fprintf(stderr, "Init thread mutex error.\n");
				return -1;
			}
#endif

			// setup rfid read thread
			if (pthread_create(&rfid_read_tid, NULL, rfid_read, &tinfo) < 0) {
				fprintf(stderr, "Create rfid read thread failed.\n");
				return -1;
			}

			// setup upload thread
#if 1
			if (pthread_create(&upload_tid, NULL, rfid_upload, &tinfo) < 0) {
				fprintf(stderr, "Create rfid read thread failed.\n");
				return -1;
			}

#endif
			
			/* wait SIGINT signal to exit */
			pthread_join(rfid_read_tid, NULL);
			DEBUG_MSG("rfid read thread exit.\n");
#if 1
			pthread_cancel(upload_tid);
			DEBUG_MSG("rfid upload thread exit.\n");
#endif

			//pthread_mutex_destroy(&rfid_read_mutex);
		}

		//return 0;
	}
	
	if (NULL != rfids)
	{
		DEBUG_MSG("rfid list destroy.\n");
		rfid_list_destroy(rfids);
	}

	return 0;
}

int get_data(list_t *plist, char *buf, int buf_len)
{
	char tmp[128] = {0};

	if (NULL == plist || NULL == buf || buf_len <= 0)
		goto error;

	memset(tmp, 0, 128);

#if 1
	if(plist->len)
	{
		list_iterator_t *it = list_iterator_new(plist, LIST_HEAD);
		list_node_t *d;

		while(NULL != (d = list_iterator_next(it)))
		{
			rfid_info *p = d->val;

			// data format 
			if (plist->len == 1)
				sprintf(tmp, "%s|%s|%d", p->tag_data, p->temperture, p->ant_num);
			else 
				sprintf(tmp, "%s|%s|%d,", p->tag_data, p->temperture, p->ant_num);


			strncat(buf, tmp, strlen(tmp));

			//printf("list TagData[%s] Temperature[%s] AntNo[%d]\n", p->tag_data, p->temperture, p->ant_num);

		}
	}
#endif
	//printf("\n msg : %s \n", buf);

	return 0;

error:
	return -1;

}

static void* rfid_upload(void *arg)
{
	int i = 0;
	int ret = -1;
	int fd = -1;
	int len = -1;
	char* buf;
	void * handle;
	unsigned char rfid_addr = 0; //read ant 1 ?
	pthread_t tid;

	struct thread_info *tinfo = (struct thread_info *)arg; 

	tid = pthread_self();

	struct timeval tv;

	buf = (char*)malloc(8*1024 * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "rfid upload thread malloc read buffer.\n");
		pthread_exit(NULL);
	}
	memset(buf, 0, (8*1024));
#if 0
	struct mosquitto *mosq = NULL;
	//bool session = true;
	//libmosquitto initial
	mosquitto_lib_init();

	//mosquitto client
	//mosq = mosquitto_new(NULL,session,NULL);
	mosq = mosquitto_new(NULL, true, NULL);
	if(!mosq){
		printf("create client failed..\n");
		mosquitto_lib_cleanup();
		goto error;
	}

#if 0 // for test
	//setup call back
	mosquitto_log_callback_set(mosq, log_callback);
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);
	mosquitto_subscribe_callback_set(mosq, subscribe_callback);
#endif

	//connect mqtt server
	if(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE))
	{
		fprintf(stderr, "Unable to connect.\n");
		goto error;
	}

	//create thread to info from mosquitto_loop() 
	int loop = mosquitto_loop_start(mosq);
	if(loop != MOSQ_ERR_SUCCESS)
	{
		printf("mosquitto loop error\n");
		goto error;
	}
	
#endif
	while (1)
	{
		/* set timeout value within input loop */
		tv.tv_usec = 0;  /* milliseconds */
		//tv.tv_sec  = TIMEOUT_SECOND_MAX;  /* seconds */
		tv.tv_sec  = 60;  /* seconds */

		int res = select(0, NULL, NULL, NULL, &tv);
		if (res == 0) {

			/* number of file descriptors with input = 0, timeout occurred. */ 
			pthread_mutex_lock(&read_mutex);
			len = get_data(rfids, buf, (8 *1024));   /* I - Serial port file */

			DEBUG_PRINT(" rfid upload thread %u read length = %d.\n",tid, len);
			if (len < 0) {
				fprintf(stderr, "rfid upload thread %u send failed.\n", tid);
				continue; //retry
			}
#if 1
			printf("data: %s \n\n", buf);
			int len = strlen(buf);

			// use default ip and port
			if (len > 0)
				mqtt_pub(buf, len, NULL);
				//mosquitto_publish(mosq, NULL, "data:", (len), buf, 0, 0);

			pthread_mutex_unlock(&read_mutex);
			memset(buf, 0, (8*1024));
#endif
		}

	}

error:
    if (NULL == buf)
		free(buf);
	pthread_exit(NULL);
}

static void* rfid_read(void* arg)
{
	int i = 0;
	int ret = -1;
	int fd = -1;
	int len = -1;
	char* buf;
	void * handle;
	unsigned char rfid_addr = 0; //read ant 1 ?

	struct thread_info *tinfo = (struct thread_info *)arg; 

	struct timeval tv;
	
	struct data data;

	char tmp[512];
	int retry = 0;
	unsigned char time_index = 23; //PollingTime=23,  40+23*20 mS = 500mS

	unsigned char AntSt[4]; 
	AntSt[0] = 0x01;
	AntSt[1] = 0x01; 
	AntSt[2] = 0x01; 
	AntSt[3] = 0x01;

	memset(&data, 0, sizeof(struct data));
	memset(tmp, 0, 512);

	//pthread_t tid;
	
	//open lib
	if((handle = dlopen("./libISO18KAPI.so", RTLD_LAZY))  == NULL)
	{
		fprintf(stderr,"dlopen:%s\n", dlerror());
		goto error;
	}

	//open
	fd = rfid_open("/dev/ttyS1", handle, &rfid_addr);
	if (fd < 0)
	{
		fprintf(stderr, "failed to open RFID COM Port.\n");
		goto error;
	}

	//reset 
	ret = rfid_reset(fd, handle, rfid_addr);
	if (ret < 0)
	{
		fprintf(stderr, "failed to reset  rfid.\n");
		goto error;
	}
#if 1
	fprintf(stderr, "wait for 3s. \n");
	sleep(3);

	fprintf(stderr, "read system status. \n");
	for (i = 0; i < 100; i++)
	{
		ret = rfid_get_sys_status(fd, handle, rfid_addr);
		if (ret < 0)
		{
			fprintf(stderr, "failed to get system status.\n");
			//goto error;
		}
		else
			break;
	}

	if (i >= 100)
		goto error;


	//enable all antennas (4)
	ret = rfid_set_sys_RF_ant_enable(fd, handle, AntSt);
	if (ret < 0)
	{
		fprintf(stderr, "failed to enable antenna.\n");
		goto error;
	}

	//enable antenna auto polling
	ret = rfid_set_sys_RF_ant_auto_poll_on(fd, handle);
	if (ret < 0)
	{
		fprintf(stderr, "failed to enable antenna auto polling.\n");
		goto error;
	}

	//set antenna auto polling time
	ret = rfid_RF_ant_poll_time_set(fd, handle, &time_index);
	if (ret < 0)
	{
		fprintf(stderr, "failed to set antenna auto polling time.\n");
		goto error;
	}
#if 0
	//get antenna auto polling time
	ret = rfid_RF_ant_poll_time_get(fd, handle, &time_index);
	if (ret < 0)
	{
		fprintf(stderr, "failed to set antenna auto polling time.\n");
		goto error;
	}
	fprintf(stderr, "Antenna auto polling time is %d ms\n", (40 + (time_index * 20)));

#endif

#endif


	//polling read
	while(1)
	{

		for (retry = 0; retry < 5; retry++)
		{
			// read data
			//ret = rfid_read_temperature_tag_data(fd, handle, rfid_addr, &data);
			ret = rfid_read_temperature_tag_data(fd, handle, rfid_addr, rfids);
			if (ret < 0)
			{
				fprintf(stderr, "failed to read rfid.\n");

				if (retry >= 5)
					goto error;

				sleep(1); //
				continue;
			}
			else
				break;

		}

		if (retry == 5)
		{
			fprintf(stderr, "read rfid retry : %d\n", retry);
			goto error;
#if 0
			fprintf(stderr, "reset rfid \n", retry);
			//reset 
			ret = rfid_reset(fd, handle, rfid_addr);
			if (ret < 0)
			{
				fprintf(stderr, "failed to reset  rfid.\n");
				goto error;
			}
			sleep(3);

#endif
		}
	}
	

	fprintf(stderr, " \n===========rfid read worker end==============\n");
	pthread_exit(NULL);

error:

	if (fd >=0 )
		rfid_close(fd, handle, rfid_addr);

	if (handle != NULL)
		dlclose(handle);

	fprintf(stderr, " \n===========rfid read worker fail ==============\n");
	pthread_exit(NULL);
}
static void* rfid_test_mode(void* arg)
{
	char* buf;

	struct thread_info *tinfo = (struct thread_info *)arg; 

	struct timeval tv;

	//pthread_t tid;
	
	test_mode();

#if 0
	//send to mqtt server
	if (len > 0)
		//mqtt_pub(data_in, len);
		mqtt_pub(data_in, len, &(tinfo->addr));
#endif

	fprintf(stderr, " \n===========rfid test  mode end==============\n");
	pthread_exit(NULL);
}

void getNowTime(void)
{
#if 0
	time_t timer;//time_t就是long int 类型
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);
	printf("Local time is: %s\n", asctime(tblock));

	//Local time is: Mon Feb 16 11:29:26 2009
#endif
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);

	struct tm nowTime;

	localtime_r(&time.tv_sec, &nowTime);

	char current[1024];

	sprintf(current, "%04d %02d %02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon+1, nowTime.tm_mday, 
			nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);

	fprintf(stderr, "time:%s\n", current);
}

static void* read_mcu_handler(void* arg)
{
	char* buf;
	struct spi_bridge *p;
	struct thread_info *tinfo = (struct thread_info *)arg; 

	struct timeval tv;

	//pthread_t tid;

	//buf = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char));
	buf = (char*)malloc((tinfo->frame_size * 1024) * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "spi bridge failed to malloc spi read buffer.\n");
		pthread_exit(NULL);
	}

	while (1) {
		for (int i = 0; i < tinfo->port_num; i++) {
			char* data_in;
			uint8_t status;
			ssize_t len;
			p = &spi_bridge_group[i];
#if 0
			if (is_valid_fd(p->fd))
				continue;
#endif
			fprintf(stderr, "read port %d \n", i);

			//fprintf(stderr, " \n  +++++++++++++++ read mcu %d++++++++++++++++++++ \n", j);

			data_in = buf;

			/* set timeout value within input loop */
#if 0
			tv.tv_usec = 0;  /* milliseconds */
			tv.tv_sec  = TIMEOUT_SECOND_MAX;  /* seconds */
#else
			tv.tv_usec = TIMEOUT_MSEC_MAX;  /* milliseconds */
			tv.tv_sec  = 0;  /* seconds */
#endif

			int res = select(0, NULL, NULL, NULL, &tv);
			if (res == 0) {
				/* number of file descriptors with input = 0, timeout occurred. */ 

				do {
					//pthread_mutex_lock(&spi_bridge.spi_mutex);
					status = spi_bridge_read_status(p);
					if (status != 0x51)
						DEBUG_PRINTF("read_mcu_handler read status = 0x%x\n", status);
					if (_is_spi_bridge_status_head_ok(status)
							&& _can_spi_bridge_status_read(status)) {
						break;
					}
					//pthread_mutex_unlock(&spi_bridge.spi_mutex);
					usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
				} while (1);

				//len = spi_bridge_read(p, data_in, SPI_BRIDGE_MAX_DATA_LEN);
				len = spi_bridge_read(p, data_in, (tinfo->frame_size * 1024));
				//pthread_mutex_unlock(&spi_bridge.spi_mutex);
				if (len < 0) {
					fprintf(stderr, "spi bridge read failed.\n");
					break;
				}
				if ((tinfo->frame_size * 1024) != len)
					data_in[len] = '\0';
#if 0
				int ret;
				for (ret = 0; ret < SPI_BRIDGE_MAX_DATA_LEN; ret++) {
					if (!(ret % 32))
						puts("");
					printf("%x ", (unsigned char)data_in[ret]);
				}
				puts("");

				//fprintf(stdout, "%s", data_in);
#endif
				fflush(stdout);

				printf("mqtt payload data len : %d \n", len);
#if 1
				//send to mqtt server
				if (len > 0)
					//mqtt_pub(data_in, len);
					mqtt_pub(data_in, len, &(tinfo->addr));
#endif
			}
		}
	}

	free(buf);
	fprintf(stderr, " \n===========read mcu end==============\n");
	//pthread_cancel(spi_bridge.read_stdin_tidp);
	pthread_exit(NULL);
}


