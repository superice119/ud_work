
/**
 * @
 *
 * @
 */

#include "main.h"
//DvrSsd:w:

static char help[] =    "spi-bridge [option] [para]\n"
						" [read/write/status] stm32 data\n"
						" -r spi-bridge read \n"
						" -w spi-bridge write [string]\n"
						" -S spi-bridge status \n"
						" -s spi-bridge read data in thread \n"
						"  NOTE -- read/write/status value are in string\n";

static inline void usage(void) 
{
	fprintf(stderr, "Usage:\n%s\n", help);
    exit(1);
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

static struct spi_bridge spi_bridge;

static struct spi_bridge spi_bridge_group[16];

struct bridge_group  bridge_group;

static inline bool _is_spi_bridge_status_head_ok(uint8_t status)
{
	return (status & SPI_BRIDGE_STATUS_HEAD_MASK) == SPI_BRIDGE_STATUS_OK;
}

static inline bool _can_spi_bridge_status_read(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_E);
}

static inline bool _can_spi_bridge_status_write(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_F);
}

static inline bool _is_spi_bridge_status_set_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_SET_PARAMETER_ERR);
}

static inline bool _is_spi_bridge_status_check_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_CHECK_ERR);
}

static inline bool _is_spi_bridge_status_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_ERR_MASK);
}
#if 0
static inline uint8_t spi_bridge_calulate_check(unsigned char* data)
{
	int i;
	uint8_t chk = 0;
	unsigned char* ptr = data;
	//for (i = (spi_bridge.len); i > 0; i--) {
	for (i = (SPI_BRIDGE_LEN_224_BYTES); i > 0; i--) {
		chk ^= *ptr++;
	}
	return chk;
}
#endif
static inline uint8_t spi_bridge_calulate_check(unsigned char* data, uint32_t len)
{
	int i;
	uint8_t chk = 0;
	unsigned char* ptr = data;
	//for (i = (SPI_BRIDGE_LEN_224_BYTES); i > 0; i--) {
	for (i = (len); i > 0; i--) {
		chk ^= *ptr++;
	}
	return chk;
}


static uint8_t spi_bridge_send_cmd(struct spi_bridge *p, const uint8_t cmd, bool retry)
{
	struct spi_ioc_transfer msg[2];
	int i = 1;
	uint8_t resp;


	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));

	msg[0].tx_buf = (__u32)&cmd;
	msg[0].len = 1;
	msg[0].delay_usecs = SPI_MCU_RESP_DELAY_US;

	msg[1].rx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;
	msg[1].delay_usecs = SPI_MCU_RESP_DELAY_US;

	if (ioctl(p->fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "spi bridge send cmd first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	DEBUG_PRINTF("cmd = %.02X.\n", cmd);
	DEBUG_PRINTF("resp = %.02X.\n", resp);

	if (_is_spi_bridge_status_head_ok(resp))
		return resp;

	if (!retry) {
		fprintf(stderr, "spi bridge send cmd error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	do {
		usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
		DEBUG_PRINTF("spi bridge send cmd retry %d.\n", i);
		if (ioctl(p->fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) { 
			fprintf(stderr, "spi bridge send cmd retry %d error.\n", i);
			return SPI_BRIDGE_STATUS_NULL;
		}

		DEBUG_PRINTF("cmd = %.02X.\n", cmd);
		DEBUG_PRINTF("resp = %.02X.\n", resp);

		if (_is_spi_bridge_status_head_ok(resp))
			return resp;
	} while (i++ <= SPI_BRIDGE_RESP_RETRY_TIMES);

	fprintf(stderr, "spi bridge send cmd error.\n");
	return SPI_BRIDGE_STATUS_NULL;
}

static uint8_t spi_bridge_read_one_frame(struct spi_bridge *p, void* data, bool retry)
{
	struct spi_ioc_transfer msg[2];
	unsigned char buf[p->len + 1];
	uint8_t resp = SPI_BRIDGE_STATUS_OK;
	int i = 0;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));
	msg[0].rx_buf = (__u32)buf;
	msg[0].len = p->len + 1;
	msg[0].delay_usecs = SPI_MCU_CS_HIGH_DELAY_US;//SPI_MCU_RESP_DELAY_US;
	msg[0].cs_change = 1;

	if (ioctl(p->fd, SPI_IOC_MESSAGE(1), msg) < 0) {
		fprintf(stderr, "spi bridge read one frame first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}
#if 0
	int ret;
        for (ret = 0; ret < (p->len + 1); ret++) {
                if (!(ret % 32))
                        puts("");
                printf("%.2X ", (unsigned char)buf[ret]);
        }
        puts("");

	//exit(0);
#endif

	if (buf[p->len] != spi_bridge_calulate_check(buf, p->len)) {
		resp |= SPI_BRIDGE_STATUS_CHECK_ERR;
	}

	msg[1].tx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;

	if (ioctl(p->fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) {
		fprintf(stderr, "spi bridge read one frame send resp error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	if (!_is_spi_bridge_status_check_ok(resp)) {
		fprintf(stderr, "spi bridge read one frame data check error, got 0x%02x, calculated 0x%02x.\n",
			buf[p->len], spi_bridge_calulate_check(buf, p->len));
		return resp;
	}

	memcpy(data, buf, p->len);

	return resp;
}

bool is_valid_fd(int fd)
{
	    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}
#if 0
static uint8_t spi_bridge_write_one_frame(const void* data, bool retry)
{
	struct spi_ioc_transfer msg[2];
	char buf[spi_bridge.len + 1];
	int i = 1;
	uint8_t resp;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));
	memcpy(buf, data, spi_bridge.len);
	buf[spi_bridge.len + 1] = spi_bridge_calulate_check(buf);

	msg[0].tx_buf = (__u32)buf;
	msg[0].len = spi_bridge.len + 1;
	msg[0].delay_usecs = SPI_MCU_RESP_DELAY_US;

	msg[1].rx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;

	if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "spi bridge write one frame first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}
#if 0
	fprintf(stderr, "resp = %.02X.\n", resp);
	exit(0);
#endif
	while (!_is_spi_bridge_status_head_ok(resp) &&
		   (i++ <= SPI_BRIDGE_RESP_RETRY_TIMES)) {
		if (!retry) {
			fprintf(stderr, "spi bridge write one frame response error.\n");
			return SPI_BRIDGE_STATUS_NULL;
		}
		DEBUG_PRINT("spi bridge write one frame retry %d.\n", i);
		if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) {
			fprintf(stderr, "spi bridge write one frame retry %d error.\n", i);
			return SPI_BRIDGE_STATUS_NULL;
		}
	}

	if (_is_spi_bridge_status_head_ok(resp))
		return resp;

	fprintf(stderr, "spi bridge write one frame error.\n");
	return SPI_BRIDGE_STATUS_NULL;
}
#endif

static inline uint8_t spi_bridge_read_status(struct spi_bridge *p )
{
	return spi_bridge_send_cmd(p, SPI_BRIDGE_CMD_GET_STATUS, true);
}

#if 0
static ssize_t spi_bridge_write(const void* data, size_t len)
{
	int writen = len;
	int i = 0;
	char buf[spi_bridge.len];
	const char* ptr = data;
	uint8_t status;

	while (writen && (i < SPI_BRIDGE_WRITE_RETRY_TIMES)) {
		usleep(SPI_MCU_CHECK_STATUS_DELAY_US * 2);
		status = spi_bridge_send_cmd(SPI_BRIDGE_CMD_7688_WRITE_TO_STM32, true);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge write send cmd error.\n");
			return 0;
		}
		if (!_can_spi_bridge_status_write(status)) {
			fprintf(stderr, "spi bridge write stm32 full.\n");
			return len - writen;
		}
		if (writen < spi_bridge.len) {
			memset(buf, 0, spi_bridge.len);
			memcpy(buf, ptr, writen);
			ptr = buf;
		}

		usleep(SPI_MCU_CHECK_STATUS_DELAY_US * 2);

		status = spi_bridge_write_one_frame(ptr, false);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge write write one frame error.\n");
			return len - writen;
		}
		if (!_is_spi_bridge_status_check_ok(status)) {
			i++;
			DEBUG_PRINT("spi bridge write retry %d.\n", i);
			continue;
		}

		ptr += (writen < spi_bridge.len) ? 0 : spi_bridge.len;
		writen -= (writen < spi_bridge.len) ? writen : spi_bridge.len;
		i = 0;
	}

	if (SPI_BRIDGE_WRITE_RETRY_TIMES == i)
		fprintf(stderr, "spi bridge write error.\n");

	return len - writen;
}
#endif

static ssize_t spi_bridge_read(struct spi_bridge *p, void* data, size_t len)
{
	int readn = len;
	int i = 0;
	char buf[p->len];
	char* ptr = data;
	uint8_t status;

	while (readn && (i < SPI_BRIDGE_READ_RETRY_TIMES)) {
		//usleep(SPI_MCU_CHECK_STATUS_DELAY_US * 1);
		usleep(SPI_MCU_READ_DELAY_US *10); //Modify by cheny 20171206
		status = spi_bridge_send_cmd(p, SPI_BRIDGE_CMD_7688_READ_FROM_STM32, true);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge read send cmd error.\n");
			return 0;
		}
		if (!_can_spi_bridge_status_read(status)) {
			if (len != readn)
				break;
			fprintf(stderr, "spi bridge read stm32 empty.\n");
			return len - readn;
		}

		fprintf(stderr, "spi bridge read delay 50ms.\n");

		status = spi_bridge_read_one_frame(p, buf, false);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge read read one frame error.\n");
			return len - readn;
		}
		if (!_is_spi_bridge_status_check_ok(status)) {
			i++;
			DEBUG_PRINT("spi bridge read retry %d.\n", i);
			continue;
		}


		memcpy(ptr, buf, (readn < p->len) ? readn : (p->len));
		ptr += (readn < (p->len)) ? 0 : (p->len);
		readn -= (readn < (p->len)) ? readn : (p->len);
		i = 0;
	}

	if (SPI_BRIDGE_READ_RETRY_TIMES == i)
		fprintf(stderr, "spi bridge read error.\n");

	return len - readn;
}

static void* read_mcu_handler(void* arg)
{
	char* buf;
	struct spi_bridge *p;

	struct timeval tv;

	//pthread_t tid;

	buf = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "spi bridge failed to malloc spi read buffer.\n");
		pthread_exit(NULL);
	}

	while (1) {
		for (int i = 0; i < PORT_MAX; i++) {
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

				len = spi_bridge_read(p, data_in, SPI_BRIDGE_MAX_DATA_LEN);
				//pthread_mutex_unlock(&spi_bridge.spi_mutex);
				if (len < 0) {
					fprintf(stderr, "spi bridge read failed.\n");
					break;
				}
				if (SPI_BRIDGE_MAX_DATA_LEN != len)
					data_in[len] = '\0';
#if 1
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
			}
		}
	}

	free(buf);
	fprintf(stderr, " \n===========read mcu end==============\n");
	//pthread_cancel(spi_bridge.read_stdin_tidp);
	pthread_exit(NULL);
}
#if 0
static char* spi_bridge_fgets(char* str, int size, FILE* stream)
{
	char* ptr = str;
	char ch;
	int len;

	while (size--) {
		len = scanf("%c", &ch);
		if (EOF == len) {
			*ptr = '\0';
			if (feof(stream))
				break;
			if (ferror(stream)) {
				fprintf(stderr, "spi bridge read stdin error.\n");
				return NULL;
			}
		}
		*ptr++ = (char)ch;
		if ((ch == '\n') ||
			(ch == '\t')) {
			*ptr = '\0';
			break;
		}
	}
	return str;
}

static void* read_stdin_handler(void* arg)
{
	char* buf;

	buf = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "spi bridge failed to malloc spi write buffer.\n");
		pthread_exit(NULL);
	}

	/* turn off stdin buffer */
	if (setvbuf(stdin, NULL, _IONBF, 0) < 0) {
		fprintf(stderr, "spi bridge failed to turn off stdin buffer.\n");
		pthread_exit(NULL);
	}

	while (1) {
		char* data_in;
		uint8_t status;

		data_in = buf;
		data_in = spi_bridge_fgets(data_in, SPI_BRIDGE_MAX_DATA_LEN, stdin);
		if (NULL != data_in) {
			if (strncmp(data_in, "exit", 4) == 0)
				break;
			do {
				pthread_mutex_lock(&spi_bridge.spi_mutex);
				status = spi_bridge_read_status();
				DEBUG_PRINT("read_stdin_handler read status = 0x%x\n", status);
				if (_is_spi_bridge_status_head_ok(status)
					&& _can_spi_bridge_status_write(status)) {
					break;
				}
				pthread_mutex_unlock(&spi_bridge.spi_mutex);
				usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
			} while (1);
			if (spi_bridge_write(data_in, strlen(data_in)) < 0) {
				fprintf(stderr, "spi bridge write failed.\n");
				pthread_mutex_unlock(&spi_bridge.spi_mutex);
				break;
			}
			pthread_mutex_unlock(&spi_bridge.spi_mutex);
			DEBUG_PRINT("spi bridge write %s\n", data_in);
		} else {
			fprintf(stderr, "fgets error.\n");
			break;
		}
	}

	free(buf);
	pthread_cancel(spi_bridge.read_mcu_tidp);
	pthread_exit(NULL);
}
#endif

static void spi_console_exit(int sig)
{
	DEBUG_PRINT("Get SIGINT.\n");
	pthread_cancel(spi_bridge.read_mcu_tidp);
	//pthread_cancel(spi_bridge.read_stdin_tidp);
}

static int open_spi_device(const char* dev)
{
	int fd;
	int mode = SPI_MODE;
	int word_len = SPI_WORD_LEN;
	int hz = SPI_HZ;

	fd = open(dev, O_RDWR);
	if (fd <= 0) {
		fprintf(stderr, "Can not open spi device(%s).\n", dev);
		return -1;
	}
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
		fprintf(stderr, "Can not set spidev mode to %d.\n", mode);
		goto err_out;
	}
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &word_len) < 0) {
		fprintf(stderr, "Can not set spidev word len to %d.\n", word_len);
		goto err_out;
	}
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &hz) < 0) {
		fprintf(stderr, "Can not set spidev speed to %d.\n", hz);
		goto err_out;
	}
	return fd;

err_out:
	close(fd);
	return -1;
}

static int spi_bridge_init(void)
{
	struct spi_bridge *p;
	char port[32] = {0};

	for (int i = 0; i < PORT_MAX; i++) {
		sprintf(port, "%s%d", SPI_DEVICE_PRE, i+2 );

		p = &(spi_bridge_group[i]);

		memset(p, 0, sizeof(spi_bridge));

		p->fd = open_spi_device(port);
		if (p->fd <= 0) {
			fprintf(stderr, "Can not open spi port %d device!\n", i);
			return -1;
		}

		rt_ringbuffer_init(&(p->read_buf), p->read_buf_pool,
				SPI_BRIDGE_MAX_DATA_LEN);
		rt_ringbuffer_init(&(p->write_buf), p->write_buf_pool,
				SPI_BRIDGE_MAX_DATA_LEN);

		//p->len = SPI_BRIDGE_LEN_16_BYTES;
		p->len = SPI_BRIDGE_LEN_224_BYTES;

		memset(port, 0, 32);
	}

	return 0;
}

static int read_status(void)
{
#if 1
	char port[32] = {0};
	for (int i = 2; i < PORT_MAX+2; i++ )
	{
		struct spi_bridge *p = &spi_bridge_group[i-2];

		sprintf(port, "%s%d", SPI_DEVICE_PRE, i );

		p->fd = open_spi_device(port);
		if (p->fd <= 0) {
			fprintf(stderr, "Can not open spi port %d device!\n", i);
			return -1;
		}

		fprintf(stderr, "port %d device %s: \n", i, port);

		memset(port, 0, 32);

		p->status = spi_bridge_read_status(p);

		//DEBUG_PRINT("spi bridge status = 0x%x\n", spi_bridge.status);

		if (!_is_spi_bridge_status_head_ok(p->status)) {
			printf("spi bridge read status error.\n");
			close(p->fd);
			return -1;
		}
		if (!_can_spi_bridge_status_read(p->status))
			printf("Can not read. STM32 read buf empty.\n");

		if (!_can_spi_bridge_status_write(p->status))
			printf("Can not write. STM32 write buf full.\n");

		if (!_is_spi_bridge_status_set_ok(p->status))
			printf("Set stm32 parameter error.\n");

		if (!_is_spi_bridge_status_check_ok(p->status))
			printf("Transfer data error.\n");

		if (_is_spi_bridge_status_ok(p->status))
			printf("OK\n");

		close(p->fd);

	}
#endif
	return 0;
}

static int read_data(void)
{
#if 1	
	char* data;
	char port[32] = {0};
	int len;

	for (int i = 2; i < PORT_MAX+2; i++ )
	{
		struct spi_bridge *p = &spi_bridge_group[i-2];

		sprintf(port, "%s%d", SPI_DEVICE_PRE, i );

		p->fd = open_spi_device(port);
		if (p->fd <= 0) {
			fprintf(stderr, "Can not open spi port %d device!\n", i);
			return -1;
		}

		memset(port, 0, 32);

		do {
			p->status = spi_bridge_read_status(p);
			DEBUG_PRINT("spi bridge status = 0x%x\n", p->status);
			if (_is_spi_bridge_status_head_ok(p->status)
					&& _can_spi_bridge_status_read(p->status)) {
				break;
			}
			usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
		} while (1);

		//usleep(SPI_MCU_CHECK_STATUS_DELAY_US*2);

		if (NULL == (data = (uint8_t*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(uint8_t)))) {
			fprintf(stderr, "read data malloc error!\n");
			close(p->fd);
			return -1;
		}

		len = spi_bridge_read(p, data, SPI_BRIDGE_MAX_DATA_LEN);

#if 1
		int ret;
		for (ret = 0; ret < SPI_BRIDGE_MAX_DATA_LEN; ret++) {
			if (!(ret % 32))
				puts("");
			printf("%x ", (unsigned char)data[ret]);
		}
		puts("");

#endif

		printf("mqtt payload data len : %d \n", len);

		//send to mqtt server
		mqtt_pub(data, len);

		free(data);
		close(p->fd);
	}
#endif
	return 0;
}

static int read_wr(void)
{
#if 0
	if (0 == strcmp(argv[1], RT2880_SPI_WRITE_STR)) {
		char* data = argv[2];
		char* tx;

		spi_bridge.fd = open_spi_device(SPI_DEVICE);
		if (spi_bridge.fd <= 0) {
			fprintf(stderr, "Can not open spi device!\n");
			return -1;
		}

		if (NULL == data) {
			tx = (char*)malloc(2 * sizeof(char));
			if (NULL == tx) {
				fprintf(stderr, "failed to malloc spi write tx buffer!\n");
				return -1;
			}
			strcpy(tx, "\n");
		} else {
			tx = (char*)malloc((strlen(data) + 2) * sizeof(char));
			if (NULL == tx) {
				fprintf(stderr, "failed to malloc spi write tx buffer!\n");
				return -1;
			}
			strcat(strcpy(tx, data), "\n");
		}

		do {
			spi_bridge.status = spi_bridge_read_status();
			DEBUG_PRINT("spi bridge status = 0x%x\n", spi_bridge.status);
			if (_is_spi_bridge_status_head_ok(spi_bridge.status)
					&& _can_spi_bridge_status_write(spi_bridge.status)) {
				break;
			}
			usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
		} while (1);

		spi_bridge_write(tx, strlen(tx));
		DEBUG_PRINT("spi bridge write %s\n", data);
		free(tx);
		close(spi_bridge.fd);
	} else {
		fprintf(stderr, "Usage:\n%s\n", usage);
		return -1;
	}
	break;
#endif
	return 0;
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

    if ( argc <= 1) {
		usage();
		return -1;
	}

    while ((c = getopt(argc, argv, "hHDvrtSsd:w:")) != -1) {
		switch (c) {
			case 's':
				start_flag = 1;
				break;
			case 'D':
				//TODO: set process to deamon
				break;
			case 'v':
				usage();
				break;
			case 't':
				getNowTime();
				break;
			case 'h':
			case 'H':
				usage();
				break;
			case 'r':
				rflag = 1;
				break;
			case 'S':
				status_rflag = 1;
				break;
			case 'd':
				//malloc ?
				memcpy(dev, optarg, strlen(optarg));
				if (dev[0] == '\0')
					usage();
				break;
			case 'w':
				wflag = 1;
				memcpy(wbuf, optarg, strlen(optarg));
				if (strlen(optarg) >= 64)
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
    argc -= optind;
    argv += optind;

    if ( argc > 1) {
		usage();
		return -1;
	}
#endif

    //if (argc > 0)
	//	interface = argv[0];

	spi_bridge_init();

	if (start_flag == 1) {

		DEBUG_PRINT("Start SPI console.\n");

		signal(SIGINT, spi_console_exit);

		if (pthread_mutex_init(&spi_bridge.spi_mutex, NULL) < 0) {
			fprintf(stderr, "Init thread mutex error.\n");
			return -1;
		}
		if (pthread_create(&spi_bridge.read_mcu_tidp, NULL, read_mcu_handler, NULL) < 0) {
			fprintf(stderr, "Create read mcu thread failed.\n");
			return -1;
		}
#if 0
		if (pthread_create(&spi_bridge.read_stdin_tidp, NULL, read_stdin_handler, NULL) < 0) {
			fprintf(stderr, "Create write mcu thread failed.\n");
			return -1;
		}
#endif
		/* wait SIGINT signal to exit */
		pthread_join(spi_bridge.read_mcu_tidp, NULL);
#if 0
		pthread_join(spi_bridge.read_stdin_tidp, NULL);
#endif
		printf("SPI console exit.\n");
		pthread_mutex_destroy(&spi_bridge.spi_mutex);

		close(spi_bridge.fd);

		return 0;
	}

	//TODO: status 
	if (status_rflag == 1) {
		read_status();
	}

	//TODO: read
	if (rflag == 1) {
		read_data();
	}
	
	//TODO: write
	if (wflag == 1) {
		read_wr();
	}

	return 0;
}

