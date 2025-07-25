
/**
 * @file
 * A simple program that subscribes to a topic.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <mqtt.h>
#include "templates/posix_sockets.h"

	FILE *fp; // File luu macid Gateway

char mac[18];
char re_mac[100] = "";
char *mac_id_gateway = NULL;
char *mac_final = NULL;
char *substr(char *s, int start, int end);
struct mqtt_client client;
int flag_power=0;
int flag_info=0;
int flag_setgroup=0;
int power_fb=0;

/**
 * @brief The function will be called whenever a PUBLISH message is received.
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

int main(int argc, const char *argv[])
{
    const char* addr;
    const char* port;
    const char* topic ="/master/GW-016D5481/power-control";
	const char* topic2 ="/master/GW-016D5481/information";
	const char* topic3 ="/master/GW-016D5481/group-setting";
	const char* username="GW-016D5481";
	const char* password="aabbccddeeff";
	const char* client_id ="GW-016D5481";
    addr = "mq.toptrendvn.xyz";
	//addr = "192.168.1.34";
    port = "1883";

    /* open the non-blocking TCP socket (connecting to the broker) */
    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    
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

    /* subscribe */
    mqtt_subscribe(&client, topic, 0);
	mqtt_subscribe(&client, topic2, 0);
	mqtt_subscribe(&client, topic3, 0);
    /* start publishing the time */
    printf("listening for '%s' messages.\n", topic);
	printf("listening for '%s' messages.\n", topic2);
	printf("listening for '%s' messages.\n", topic3);
    printf("Press CTRL-D to exit.\n\n");

    /* block */
    //while(fgetc(stdin) != EOF); 
	 while(1)
	{
		if (flag_power == 1)
		{
			char message3[1280];
			snprintf(message3, sizeof(message3), "{\"mode\": %d}", power_fb);
			mqtt_publish(&client, "/master/GW-016D5481/status", message3, strlen(message3), MQTT_PUBLISH_QOS_0);
			if (client.error != MQTT_OK) 
			{
				fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
				exit_example(EXIT_FAILURE, sockfd, &client_daemon);
			}
			printf("published feedback: \"%s\"\r\n", message3);
			flag_power = 0;
		}
		if (flag_info == 1)
		{
			char message3[1280];
			snprintf(message3, sizeof(message3), "{\"mode\": %d, \"name\": %s, \"typeconnect\": %d, \"val1\": %d, \"val2\": %d}"
									, power_fb, "\"GW-016D5481\"",1, 5, 1);
			mqtt_publish(&client, "/master/GW-016D5481/status", message3, strlen(message3), MQTT_PUBLISH_QOS_0);
			if (client.error != MQTT_OK) 
			{
				fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
				exit_example(EXIT_FAILURE, sockfd, &client_daemon);
			}
			printf("published feedback: \"%s\"\r\n", message3);
			flag_info = 0;
		}
		
		if (flag_setgroup == 1)
		{
			char message3[1280];
			snprintf(message3, sizeof(message3), "{\"setidtogroup\": %d}", 1);
			mqtt_publish(&client, "/master/GW-016D5481/fb-group-setting", message3, strlen(message3), MQTT_PUBLISH_QOS_0);
			if (client.error != MQTT_OK) 
			{
				fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
				exit_example(EXIT_FAILURE, sockfd, &client_daemon);
			}
			printf("published feedback: \"%s\"\r\n", message3);
			flag_setgroup = 0;
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
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char* topic_name = (char*) malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';

    printf("Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);
	void *data=(const char*) published->application_message;
	if (!strncmp(data, "{\"mode\"", 6))
        {
            printf("reach here %s\r\n", data);
            char *temp = strtok(data, ":");
            temp = strtok(NULL, "\n");
            printf("[%.*s]\n", strlen(temp), (char *)temp);
            struct json_object *new_obj;
            new_obj = json_tokener_parse(temp);
            printf("Parsed is: %s\n", temp);
            printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
            if (!new_obj)
            {
                return 1; // oops, we failed.
            }
			power_fb = atoi(temp);
			flag_power = 1;
		}
	if (!strncmp(data, "{\"reqinfo\"", 10))
        {
            printf("reach here %s\r\n", data);
            char *temp = strtok(data, ":");
            temp = strtok(NULL, "\n");
            printf("[%.*s]\n", strlen(temp), (char *)temp);
            struct json_object *new_obj;
            new_obj = json_tokener_parse(temp);
            printf("Parsed is: %s\n", temp);
            printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
            if (!new_obj)
            {
                return 1; // oops, we failed.
            }
			
			flag_info = 1;
		}
	char *temp = strtok(topic_name, "/");
	temp = strtok(NULL, "\n");
	printf("[%.*s]\n", strlen(temp), (char *)temp);	
	char *toppic = strtok(temp, "/");
	toppic = strtok(NULL, "\n");
	printf("[%.*s]\n", strlen(toppic), (char *)toppic);	
    printf("Parsed toppic is: %s\n", toppic);
	int exists, i, j, k, l;
	if (!strncmp(toppic, "group-setting", 13))
        {
            printf("reach here\r\n");
            struct json_object *new_obj, *tmpQueries, *resultsObj, *tmpResults, *valuesObj, *tmpValues, *tmpSeparateVals, *resultsObjGroup;
            new_obj = json_tokener_parse(data);
            printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
            if (!new_obj)
            {
                return 1; // oops, we failed.
            }
			struct json_object *o;
			if (!json_object_object_get_ex(new_obj, "port1", &o))
            {
                printf("Field %s does not exist\n", "port1");
                return;
            }
			printf( "\n" );
			printf( "Port1:\n\n" );
			 /* Loop through array of queries */
			for ( i = 0; i < json_object_array_length(o); i++ )
			{

				tmpQueries = json_object_array_get_idx( o, i );
				if (!json_object_object_get_ex(tmpQueries, "groupId", &resultsObjGroup))
				{
					printf("Field %s does not exist\n", "groupId");
					return;
				}
				printf( "Group:[%d] = %s \n", i, json_object_to_json_string( resultsObjGroup ) );
				if (!json_object_object_get_ex(tmpQueries, "groupDevice", &resultsObj))
				{
					printf("Field %s does not exist\n", "groupDevice");
					return;
				}
				for ( j = 0; j < json_object_array_length( resultsObj ); j++ )
					{
						tmpResults = json_object_array_get_idx ( resultsObj, j );
						printf( "LampID:[%d] = %s \n", j, json_object_to_json_string( tmpResults ) );
					}
			}
			if (!json_object_object_get_ex(new_obj, "port2", &o))
            {
                printf("Field %s does not exist\n", "port1");
                return;
            }
			printf( "\n" );
			printf( "Port2:\n\n" );
			/* Loop through array of queries */
			for ( i = 0; i < json_object_array_length(o); i++ )
			{

				tmpQueries = json_object_array_get_idx( o, i );
				if (!json_object_object_get_ex(tmpQueries, "groupId", &resultsObjGroup))
				{
					printf("Field %s does not exist\n", "groupId");
					return;
				}
				printf( "Group:[%d] = %s \n", i, json_object_to_json_string( resultsObjGroup ) );
				if (!json_object_object_get_ex(tmpQueries, "groupDevice", &resultsObj))
				{
					printf("Field %s does not exist\n", "groupDevice");
					return;
				}
				for ( j = 0; j < json_object_array_length( resultsObj ); j++ )
					{
						tmpResults = json_object_array_get_idx ( resultsObj, j );
						printf( "LampID:[%d] = %s \n", j, json_object_to_json_string( tmpResults ) );
					}
			}
			flag_setgroup = 1;
		}
	free(topic_name);
	

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

char *substr(char *s, int start, int end)
{
	int indext = 0;

	for (int i = start; i <= end; i++)
	{
		re_mac[indext] = s[i];
		indext++;
	}
	re_mac[indext] = '\0';

	return re_mac;
}