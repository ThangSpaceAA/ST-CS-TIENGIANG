
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
    const char* topic;

    /* get address (argv[1] if present) */
    if (argc > 1) {
        addr = argv[1];
    } else {
        addr = "test.mosquitto.org";
    }

    /* get port number (argv[2] if present) */
    if (argc > 2) {
        port = argv[2];
    } else {
        port = "1883";
    }

    /* get the topic name to publish */
    if (argc > 3) {
        topic = argv[3];
    } else {
        topic = "datetime";
    }

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
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);

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
			snprintf(message3, BUFFER_SIZE, "qtApp-{\"CMD\": %d,\
									\"dim1\": %d,\
									\"dim2\": %d,\
									\"dim3\": %d,\
									\"dim4\": %d,\
									\"dim5\": %d,\
									\"dim6\": %d,\
									\"dim7\": %d,\
									\"dim8\": %d,\
									\"dim9\": %d,\
									\"hh_start_dim1\": %d,\
									\"mm_start_dim1\": %d,\
									\"hh_start_dim2\": %d,\
									\"mm_start_dim2\": %d,\
									\"hh_start_dim3\": %d,\
									\"mm_start_dim3\": %d,\
									\"hh_start_dim4\": %d,\
									\"mm_start_dim4\": %d,\
									\"hh_start_dim5\": %d,\
									\"mm_start_dim5\": %d,\
									\"hh_start_dim6\": %d,\
									\"mm_start_dim6\": %d,\
									\"hh_start_dim7\": %d,\
									\"mm_start_dim7\": %d,\
									\"hh_start_dim8\": %d,\
									\"mm_start_dim8\": %d,\
									\"hh_start_dim9\": %d,\
									\"mm_start_dim9\": %d}"	,1, 2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9);


        printf("%s published : \"%s\"", argv[0], message3);

        /* publish the time */
        mqtt_publish(&client, topic, message3, strlen(message3) + 1, MQTT_PUBLISH_QOS_0);

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
