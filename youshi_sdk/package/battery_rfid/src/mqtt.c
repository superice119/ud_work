/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 ##############################################*/

#include "mqtt.h"

bool session = true;

static void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	int i;
	//TODO: get config and put into config buffer(ringbuffer?)
	if(message->payloadlen){
		printf("%s %s\n", message->topic, message->payload);
#if 1 // for test
		//for(i = 0; i < (message->payloadlen); i++) {
		for(i = 0; i < (32); i++) {
			if (!(i % 8))
				puts("");
			printf("-0x%.2x ", *((unsigned char *)(message->payload + i)));
		}
		puts("");
#endif
	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);
}


static void connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int i;
	if(!result){
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "data:", 2);
	}else{
		fprintf(stderr, "Connect failed\n");
	}
}

static void subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

static void log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	printf("%s\n", str);
}



int mqtt_sub(void * buf, unsigned int len)
{
	int i;
	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mosq = NULL;

	//libmosquitto initial
	mosquitto_lib_init();

	//mosquitto client
	mosq = mosquitto_new(NULL, clean_session, NULL);
	if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	//setup call back
	mosquitto_log_callback_set(mosq, log_callback);
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);
	mosquitto_subscribe_callback_set(mosq, subscribe_callback);

	//connect mqtt server
	if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}

	//loop
	mosquitto_loop_forever(mosq, -1, 1);

	//dead
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
} 


int mqtt_pub(void * buf, unsigned int len, struct saddr * addr )
{
	char i;
	unsigned char buff[MSG_MAX_SIZE];
	unsigned char host[32];
	unsigned int port;
	struct mosquitto *mosq = NULL;

	memset(host, 0, sizeof(host));

	if (buf == NULL || len <= 0) {
		printf("mqtt pub parameters error.\n");
		return 1;
	}

	if (addr == NULL)
	{
		printf("mqtt pub use default addr .\n\n host: %s, port: %d\n", HOST, PORT);
		port = PORT;
		memcpy(host, HOST, strlen(HOST));
		goto start;
	}

	port = addr->port;
	//memcpy(host, addr->saddr, strlen(addr->saddr));
	memcpy(host, addr->saddr, addr->ssize);

	fprintf(stderr, "mqtt port: %d, host: %s.\n", port, host);

	if (port <= 0 || port >= 65535)
	{
		port = PORT;
	}

	int l = strlen(host);

	if (l < 7 || l > 16)
	{
		memcpy(host, HOST, strlen(HOST));
	}

start:
	//libmosquitto initial
	mosquitto_lib_init();

	memset(buff, 0, sizeof(buff));


	//mosquitto client
	mosq = mosquitto_new(NULL,session,NULL);
	if(!mosq){
		printf("create client failed..\n");
		mosquitto_lib_cleanup();
		return 1;
	}

#if 1 // for test
	//setup call back
	mosquitto_log_callback_set(mosq, log_callback);
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);
	mosquitto_subscribe_callback_set(mosq, subscribe_callback);
#endif

	//connect mqtt server
	//if(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE))
	if(mosquitto_connect(mosq, host, port, KEEP_ALIVE))
	{
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}

	//create thread to info from mosquitto_loop() 
	int loop = mosquitto_loop_start(mosq);
	if(loop != MOSQ_ERR_SUCCESS)
	{
		printf("mosquitto loop error\n");
		return 1;
	}
    
#if 0
	while(fgets(buff, MSG_MAX_SIZE, stdin) != NULL)
	{
		/*发布消息*/
		mosquitto_publish(mosq,NULL,"data:",32,buff,0,0);
		memset(buff,0,sizeof(buff));
	}
#else
	if (buf) {
#if 1
		printf("data len %d\n", len);

		for(i = 0; i < len; i++) {
			if (!(i % 8))
				puts("");
			printf(" 0x%.2x", *((unsigned char *)(buf + i)));
		}
		puts("");
#endif
		//memcpy(buff, buf, len);

		/*publish info*/
		mosquitto_publish(mosq, NULL, "data:", (len), buf, 0, 0);


		sleep(1); //wait mqtt client send data

	}
#endif

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}

