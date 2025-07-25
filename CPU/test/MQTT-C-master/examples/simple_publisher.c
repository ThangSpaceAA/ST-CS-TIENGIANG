
/**
 * @file
 * A simple program to that publishes the current time whenever ENTER is pressed.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <mqtt.h>
#include "templates/posix_sockets.h"

#define BUFFER_SIZE 1280

#define TEMPLATE_APP_TO_CPU_DIM_ALL "{\"id\": %d,\
                             \"port\": %d,\
                             \"group\": %d,\
                             \"dim\": %d}"

/**
 * @brief The function that would be called whenever a PUBLISH is received.
 *
 * @note This function is not used in this example.
 */
void publish_callback(void** unused, struct mqtt_response_publish *published);

/**
 * @brief The client's refresher. This function triggers back-end routines to
 *        handle ingress/egress traffic to the broker.
 *
 * @note All this function needs to do is call \ref __mqtt_recv and
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that
 *       client ingress/egress traffic will be handled every 100 ms.
 */
void* client_refresher(void* client);

/**
 * @brief Safelty closes the \p sockfd and cancels the \p client_daemon before \c exit.
 */
void exit_example(int status, int sockfd, pthread_t *client_daemon);

/**
 * A simple program to that publishes the current time whenever ENTER is pressed.
 */
int main(int argc, const char *argv[])
{
    const char* addr;
    const char* port;
    const char* topic ="/master/GW-016D5481/group-setting";
	const char* topic2 ="/master/GW-016D5481/power-schedule-setting";
	const char* topic3 ="/master/GW-016D5481/light-schedule-setting";
	const char* username = "GW-016D5481";
	const char* password = "aabbccddeeff";
	const char* client_id = "GW-016D5481";
    //addr = "mq.toptrendvn.xyz";
	addr = "192.168.1.34";
    port = "1883";

    /* open the non-blocking TCP socket (connecting to the broker) */
    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    /* Create an anonymous session */
    
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, username, password, connect_flags, 400);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        exit_example(EXIT_FAILURE, sockfd, NULL);

    }

    /* start publishing the time */
    printf("%s is ready to begin publishing the time.\n", argv[0]);
    printf("Press ENTER to publish the current time.\n");
    printf("Press CTRL-D (or any other key) to exit.\n\n");
    while(fgetc(stdin) == '\n') {
        /* get the current time */
		char message3[BUFFER_SIZE];
		snprintf(message3, sizeof(message3), "{\"mode\": %d}", 0);
		
		char message4[BUFFER_SIZE];
		snprintf(message4, sizeof(message4), "{\"mode\": %d}", 0);
		
		char message5[BUFFER_SIZE];
		snprintf(message5, sizeof(message5), "{\"PKWH\": %0.2f}", 16.88);
		
		#define TEMPLATE_COUNTER_LAMP "CPU-{\"val1\": %d,\
									\"val2\": %d}"
		char message6[BUFFER_SIZE];
		snprintf(message6, sizeof(message6), TEMPLATE_COUNTER_LAMP, 10, 2);	
							
		char message7[BUFFER_SIZE];
		snprintf(message7, sizeof(message7), "{\"port1\":[{\"groupId\":1,\"groupDevice\":[5,4]},{\"groupId\":2,\"groupDevice\":[3,1,2]}],\"port2\":[{\"groupId\":3,\"groupDevice\":[9,8,6]},{\"groupId\":4,\"groupDevice\":[10,7]}]}");					
	
		char message8[BUFFER_SIZE];
		snprintf(message8, sizeof(message8), "[{\"hh_start\":\"00\",\"mm_start\":\"00\",\"hh_end\":\"00\",\"mm_end\":\"00\"},{\"hh_start\":\"8\",\"mm_start\":\"21\",\"hh_end\":\"17\",\"mm_end\":\"01\"},{\"hh_start\":\"11\",\"mm_start\":\"21\",\"hh_end\":\"14\",\"mm_end\":\"01\"}]");					
	
		char message9[BUFFER_SIZE];
		snprintf(message9, sizeof(message8), "{\"group\":\"200\",\"schedule\":[{\"hh_start_dim\":\"00\",\"mm_start_dim\":\"00\",\"dimmer\":\"100\"},{\"hh_start_dim\":\"11\",\"mm_start_dim\":\"22\",\"dimmer\":\"33\"},{\"hh_start_dim\":\"44\",\"mm_start_dim\":\"55\",\"dimmer\":\"66\"}]}");					
						
        printf("published on power: \"%s\"\r\n", message3);
		//printf("published off power: \"%s\"\r\n", message4);
		//printf("published PKWH: \"%s\"\r\n", message5);
		//printf("published count lamp: \"%s\"\r\n", message6);
		printf("published: \"%s\"\r\n", message7);
		printf("published: \"%s\"\r\n", message8);
		printf("published: \"%s\"\r\n", message9);
        /* publish the time */
        mqtt_publish(&client, topic, message3, strlen(message3), MQTT_PUBLISH_QOS_0);
		//mqtt_publish(&client, topic, message4, strlen(message4), MQTT_PUBLISH_QOS_0);
		mqtt_publish(&client, topic, message7, strlen(message7), MQTT_PUBLISH_QOS_0);
		mqtt_publish(&client, topic2, message8, strlen(message8), MQTT_PUBLISH_QOS_0);
		mqtt_publish(&client, topic3, message9, strlen(message9), MQTT_PUBLISH_QOS_0);
		//mqtt_publish(&client, topic, message6, strlen(message6), MQTT_PUBLISH_QOS_1);
        /* check for errors */
        if (client.error != MQTT_OK) {
            fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
            exit_example(EXIT_FAILURE, sockfd, &client_daemon);
        }
    }

    /* disconnect */
    printf("\n%s disconnecting from %s\n", argv[0], addr);
    sleep(1);

    /* exit */
    exit_example(EXIT_SUCCESS, sockfd, &client_daemon);
}

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
    exit(status);
}



void publish_callback(void** unused, struct mqtt_response_publish *published)
{
    /* not used in this example */
}

void* client_refresher(void* client)
{
    while(1)
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}
