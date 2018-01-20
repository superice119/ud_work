
/**
 * @
 *
 * @
 */

#include "spi_bridge.h"

int open_spi_device(const char* dev)
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


int spi_bridge_init(int port_num, int frame_size)
{
	struct spi_bridge *p;
	char port[32] = {0};

	for (int i = 0; i < port_num; i++) {
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
		//p->len = SPI_BRIDGE_LEN_224_BYTES;
		//p->len = SPI_BRIDGE_LEN_4064_BYTES;
		//p->len = SPI_BRIDGE_LEN_2048_BYTES;
		if (frame_size == 4)
			p->len = SPI_BRIDGE_LEN_4096_BYTES;
		//p->len = SPI_BRIDGE_LEN_8192_BYTES;
		if (frame_size == 8)
			p->len = SPI_BRIDGE_LEN_8160_BYTES;

		memset(port, 0, 32);
	}

	return 0;
}


uint8_t spi_bridge_send_cmd(struct spi_bridge *p, const uint8_t cmd, bool retry)
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

uint8_t spi_bridge_read_one_frame(struct spi_bridge *p, void* data, bool retry)
{
	struct spi_ioc_transfer msg[2];
	unsigned char buf[p->len + 1];
	uint8_t resp = SPI_BRIDGE_STATUS_OK;
	int i = 0;
	int ret = 0;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));
	msg[0].rx_buf = (__u32)buf;
	msg[0].len = p->len + 1;
	msg[0].delay_usecs = SPI_MCU_CS_HIGH_DELAY_US;//SPI_MCU_RESP_DELAY_US;
	msg[0].cs_change = 1;

	if ((ret = ioctl(p->fd, SPI_IOC_MESSAGE(1), msg)) < 0) {
		perror("error");
		fprintf(stderr, "spi bridge read one frame first send error. errno %d\n", errno);
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


ssize_t spi_bridge_read(struct spi_bridge *p, void* data, size_t len)
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
		//usleep(SPI_MCU_READ_DELAY_US * 5); //Modify by cheny 20171206

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

char* spi_bridge_fgets(char* str, int size, FILE* stream)
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


#if 0
uint8_t spi_bridge_write_one_frame(const void* data, bool retry)
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
ssize_t spi_bridge_write(const void* data, size_t len)
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
