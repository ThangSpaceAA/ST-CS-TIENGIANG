#define _GNU_SOURCE
#include <stdio.h>
#include "main.h"
#include "Parameter_CPU.h"
// Bu#ild: gcc -Wall -lev -luwsc -o Cpu main.c Parameter_CPU.c function_system.c gps.c -lwiringPi -lpthread -lmodbus -ljson-c -lev -luwsc -I.

#pragma region BIEN TOAN CUC
const char *url = "ws://localhost:8080";
char urlmqtt[100];
struct ev_signal signal_watcher;
int ping_interval = 10; /* second */
struct uwsc_client *cl;
int opt;
int sockfd_bk = -1;
int sockfd;

int flag_onoffstatus = 0;
int flag_inforstatus = 0;
int flag_dimstatus = 0;
pthread_t client_daemon;
uint8_t tx_buffer[1024] = {0};
int power_fb = 0;
int check_group = 0;
int check_port = 0;
int check_onoff = 0;
int size_array_on_off_schedule = 0;
int count_lamp_rf = 0;
struct mqtt_client client;
parameter_MFM384_t parameter_MFM384;

void *client_refresher(void *client);
void serial_get_buffer(void);
void get_data_finish(void);
void publish_callback(void **unused, struct mqtt_response_publish *published);
void exit_example(int status, int sockfd, pthread_t *client_daemon);
int checkconnect = 0;

char topicsub[50] = "/power-control";
char topicsub2[50] = "/information";
char topicsub3[50] = "/group-setting";
char topicsub4[50] = "/power-schedule-setting";
char topicsub5[50] = "/light-schedule-setting";
char topicsub6[50] = "/onlyone-schedule-setting";
char topicsub7[50] = "/group-schedule-setting";
char topicsub8[50] = "/request-power-schedule";
char topicsub9[50] = "/request-light-schedule";
char topicsub10[50] = "/request-group-schedule";
char topicsub11[50] = "/sync-time";
char topicsub12[50] = "/dim-information";

int flag_chart = 0;

char topic[255] = "";
char topic2[255] = "";
char topic3[255] = "";
char topic4[255] = "";
char topic5[255] = "";
char topic6[255] = "";
char topic7[255] = "";
char topic8[255] = "";
char topic9[255] = "";
char topic10[255] = "";
char topic11[255] = "";
char topic12[255] = "";

char topicrawpub[50] = "/status";
char topicrawpub2[50] = "/fb-group-setting";
char topicrawpub3[50] = "/fb-power-schedule-setting";
char topicrawpub4[50] = "/fb-light-schedule-setting";
char topicrawpub5[50] = "/fb-onlyone-schedule-setting";
char topicrawpub6[50] = "/fb-group-schedule-setting";
char topicrawpub7[50] = "/fb-request-power-schedule";
char topicrawpub8[50] = "/fb-request-light-schedule";
char topicrawpub9[50] = "/fb-request-group-schedule";
char topicrawpub10[50] = "/fb-sync-time";
char topicrawpub11[50] = "/fb-dim-information";
char topicrawpub12[50] = "/report";

char topicpub[255] = "";
char topicpub2[255] = "";
char topicpub3[255] = "";
char topicpub4[255] = "";
char topicpub5[255] = "";
char topicpub6[255] = "";
char topicpub7[255] = "";
char topicpub8[255] = "";
char topicpub9[255] = "";
char topicpub10[255] = "";
char topicpub11[255] = "";
char topicpub12[255] = "";

// type_flag_system_t cpu_flag_system;

typedef struct __attribute__((packed))
{
    char dim_fb[250];
    uint8_t lampport;
    uint8_t group[250];
    uint8_t id[250];
} fb_dim_t;
fb_dim_t fb_dim_port1;
fb_dim_t fb_dim_port2;

char mac[18];
char time_update[18];
char date_update[18];

char re_mac[100] = "";
char *mac_id_gateway = NULL;
char *mac_final = NULL;
char *substr(char *s, int start, int end);
char topicraw[50] = "/master/";

void sort_up(uint8_t number[], int n)
{
    uint8_t temp;
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (number[i] > number[j])
            {
                temp = number[i];
                number[i] = number[j];
                number[j] = temp;
            }
        }
    }
}

void sort_down(uint8_t a[], int n)
{
    for (int i = 0; i <= n - 1; i++)
        for (int j = i; j <= n; j++)
        {
            if (a[i] < a[j])
            {
                uint8_t temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
}

void insert_queue(uint8_t insert_item)
{
    bool enable_save_queue = true;
    int i;
    if (queue.rear == Size_Queue_Array - 1)
    {
        printf("Overflow\r\n");
    }
    else
    {
        if (queue.front == -1)
        {
            queue.front = 0;
        }
        for (i = 0; i <= queue.rear; i++)
        {
            if (insert_item == queue.inp_arr[i])
            {
                enable_save_queue = false;
                break;
            }
        }
        if (enable_save_queue)
        {
            printf("Element to be inserted in the Queue:\r\n");
            queue.rear = queue.rear + 1;
            queue.inp_arr[queue.rear] = insert_item;
        }
        else
        {
            printf("Exist element in the Queue!!!!!\r\n");
        }
    }
}

void descrease_queue(void)
{
    if (queue.front == -1 || queue.front > queue.rear)
    {
        printf("Underflow \n");
        queue.front = -1;
        queue.rear = -1;
    }
    else
    {
        printf("Element deleted from the Queue: %d\n", queue.inp_arr[queue.front]);
        queue.front = queue.front + 1;
        printf("gia tri queue front: %d\r\n", queue.front);
        if (queue.front > queue.rear)
        {

            queue.front = -1;
            queue.rear = -1;
        }
    }
}

void show_queue(void)
{
    if (queue.front == -1)
    {
        printf("Empty Queue \n");
    }
    else
    {
        printf("Queue: \n");
        for (int i = queue.front; i <= queue.rear; i++)
            printf("%d ", queue.inp_arr[i]); //
        printf("\n");
    }
}

void insert_queue_lamp_port(uint8_t insert_item, int port)
{
    bool enable_save_queue = true;
    int i;
    if (port == 1)
    {
        if (queue_lamps_port1.rear == Size_Queue_Array - 1)
        {
            printf("Overflow\r\n");
        }
        else
        {
            if (queue_lamps_port1.front == -1)
            {
                queue_lamps_port1.front = 0;
            }
            for (i = 0; i <= queue_lamps_port1.rear; i++)
            {
                if (insert_item == queue_lamps_port1.inp_arr[i])
                {
                    enable_save_queue = false;
                    break;
                }
            }
            if (enable_save_queue)
            {
                printf("Element to be inserted lamp port1 rs485 in the Queue:\r\n");
                queue_lamps_port1.rear = queue_lamps_port1.rear + 1;
                queue_lamps_port1.inp_arr[queue_lamps_port1.rear] = insert_item;
            }
            else
            {
                printf("Exist element in the Queue!!!!!\r\n");
            }
        }
    }
    else if (port == 2)
    {
        if (queue_lamps_port2.rear == Size_Queue_Array - 1)
        {
            printf("Overflow\r\n");
        }
        else
        {
            if (queue_lamps_port2.front == -1)
            {
                queue_lamps_port2.front = 0;
            }
            for (i = 0; i <= queue_lamps_port2.rear; i++)
            {
                if (insert_item == queue_lamps_port2.inp_arr[i])
                {
                    enable_save_queue = false;
                    break;
                }
            }
            if (enable_save_queue)
            {
                printf("Element to be inserted lamp port2 rs485 in the Queue:\r\n");
                queue_lamps_port2.rear = queue_lamps_port2.rear + 1;
                queue_lamps_port2.inp_arr[queue_lamps_port2.rear] = insert_item;
            }
            else
            {
                printf("Exist element in the Queue!!!!!\r\n");
            }
        }
    }
}

void descrease_queue_lamp_port(int port)
{
    if (port == 1)
    {
        if (queue_lamps_port1.front == -1 || queue_lamps_port1.front > queue_lamps_port1.rear)
        {
            printf("Underflow \n");
            queue_lamps_port1.front = -1;
            queue_lamps_port1.rear = -1;
        }
        else
        {
            printf("Element deleted from the Queue lamp port 1: %d\n", queue.inp_arr[queue_lamps_port1.front]);
            queue_lamps_port1.front = queue_lamps_port1.front + 1;
            printf("gia tri queue front lamp port 1: %d\r\n", queue_lamps_port1.front);
            if (queue_lamps_port1.front > queue_lamps_port1.rear)
            {

                queue_lamps_port1.front = -1;
                queue_lamps_port1.rear = -1;
            }
        }
    }
    else if (port == 2)
    {
        if (queue_lamps_port2.front == -1 || queue_lamps_port2.front > queue_lamps_port2.rear)
        {
            printf("Underflow \n");
            queue_lamps_port2.front = -1;
            queue_lamps_port2.rear = -1;
        }
        else
        {
            printf("Element deleted from the Queue lamp port 2: %d\n", queue.inp_arr[queue_lamps_port2.front]);
            queue_lamps_port2.front = queue_lamps_port2.front + 1;
            printf("gia tri queue front lamp port 2: %d\r\n", queue_lamps_port2.front);
            if (queue_lamps_port2.front > queue_lamps_port2.rear)
            {

                queue_lamps_port2.front = -1;
                queue_lamps_port2.rear = -1;
            }
        }
    }
}

void show_queue_lamp_port(int port)
{
    if (port == 1)
    {
        if (queue_lamps_port1.front == -1)
        {
            printf("Empty queue_lamps_port1 \n");
        }
        else
        {
            printf("queue_lamps_port1: \n");
            for (int i = queue_lamps_port1.front; i <= queue_lamps_port1.rear; i++)
                printf("%d ", queue_lamps_port1.inp_arr[i]); //
            printf("\n");
        }
    }
    else if (port == 2)
    {
        if (queue_lamps_port2.front == -1)
        {
            printf("Empty queue_lamps_port2 \n");
        }
        else
        {
            printf("queue_lamps_port2: \n");
            for (int i = queue_lamps_port2.front; i <= queue_lamps_port2.rear; i++)
                printf("%d ", queue_lamps_port2.inp_arr[i]); //
            printf("\n");
        }
    }
}

void send_data_cpu_to_qt(uint8_t arr[], int rear, const char *template, const char cmd)
{

    char data3[1000] = {0};
    char tmp_message1[200] = {0};
    char tmp_message2[200] = {0};
    sort_up(arr, rear);
    for (int i = 0; i <= rear; i++)
    {
        if (arr[i] != 0)
        {
            snprintf(tmp_message2, sizeof(tmp_message2) + 2, "%s %d", tmp_message1, (uint8_t)arr[i]);
            snprintf(tmp_message1, sizeof(tmp_message1), "%s", tmp_message2);
        }
    }
    printf("feedback message: %s\r\n", tmp_message1);
    int data_len3 = sprintf(data3, template, cmd, tmp_message1);

    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len3, data3);
    printf("Da send den qt ok %s\n", data3);
    memset(data3, 0x00, 1000);
}

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1)
        close(sockfd);
    if (client_daemon != NULL)
        pthread_cancel(*client_daemon);
    exit(status);
}
#pragma endregion
type_lightAddConfig_t type_lightAddConfig;
void *Serial(void *threadArgs)
{
    while (1)
    {
        delay(1);
        serial_get_buffer();
    }
}

void *Mqtt(void *threadArgs)
{
    const char *username = mac_final;
    const char *password = "aabbccddeeff";
    const char *client_id = mac_final;
    // const char* addr = "mq.sitechlighting.one";
    char *add = NULL;
    add = setting.urlmqtt;
    char urlserver[100];
    const char *addr =  add;
    sprintf(urlserver, "%s.%s", "app", add);
    // addr = "app.sitech.com.vn";
    const char *port = "1883";
    sprintf(topic, "%s%s%s", topicraw, mac_final, topicsub);
    sprintf(topic2, "%s%s%s", topicraw, mac_final, topicsub2);
    sprintf(topic3, "%s%s%s", topicraw, mac_final, topicsub3);
    sprintf(topic4, "%s%s%s", topicraw, mac_final, topicsub4);
    sprintf(topic5, "%s%s%s", topicraw, mac_final, topicsub5);
    sprintf(topic6, "%s%s%s", topicraw, mac_final, topicsub6);
    sprintf(topic7, "%s%s%s", topicraw, mac_final, topicsub7);
    sprintf(topic8, "%s%s%s", topicraw, mac_final, topicsub8);
    sprintf(topic9, "%s%s%s", topicraw, mac_final, topicsub9);
    sprintf(topic10, "%s%s%s", topicraw, mac_final, topicsub10);
    sprintf(topic11, "%s%s%s", topicraw, mac_final, topicsub11);
    sprintf(topic12, "%s%s%s", topicraw, mac_final, topicsub12);

    sprintf(topicpub, "%s%s%s", topicraw, mac_final, topicrawpub);
    sprintf(topicpub2, "%s%s%s", topicraw, mac_final, topicrawpub2);
    sprintf(topicpub3, "%s%s%s", topicraw, mac_final, topicrawpub3);
    sprintf(topicpub4, "%s%s%s", topicraw, mac_final, topicrawpub4);
    sprintf(topicpub5, "%s%s%s", topicraw, mac_final, topicrawpub5);
    sprintf(topicpub6, "%s%s%s", topicraw, mac_final, topicrawpub6);
    sprintf(topicpub7, "%s%s%s", topicraw, mac_final, topicrawpub7);
    sprintf(topicpub8, "%s%s%s", topicraw, mac_final, topicrawpub8);
    sprintf(topicpub9, "%s%s%s", topicraw, mac_final, topicrawpub9);
    sprintf(topicpub10, "%s%s%s", topicraw, mac_final, topicrawpub10);
    sprintf(topicpub11, "%s%s%s", topicraw, mac_final, topicrawpub11);
    sprintf(topicpub12, "%s%s%s", topicraw, mac_final, topicrawpub12);
    delay(3000);
    printf("dia chi addr: %s\r\n", urlserver);
    printf("thong tin port: %s\r\n", port);
    delay(3000);
    sockfd = open_nb_socket(urlserver, port);
    // printf("sockfd: %d\n", sockfd);
    if (sockfd == -1)
    {
        printf("Failed to open socket\n");
        perror("Failed to open socket: ");
        checkconnect = 0;
        sockfd_bk = -1;
        // exit_example(EXIT_FAILURE, sockfd, NULL);
    }
    else
    {
        checkconnect = 1;
        sockfd_bk = 0;
    }

    uint8_t sendbuf[20480]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[10240]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    /* Create an anonymous session */
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, username, password, connect_flags, 400);
    /* check that we don't have any errors */
    if (client.error != MQTT_OK)
    {
        printf("Failed %s\n", mqtt_error_str(client.error));
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        // exit_example(EXIT_FAILURE, sockfd, NULL);

        pthread_exit(NULL);
    }
    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    if (pthread_create(&client_daemon, NULL, client_refresher, &client))
    {
        fprintf(stderr, "Failed to start client daemon.\n");
        printf("Failed to start client daemon.\n");
        // exit_example(EXIT_FAILURE, sockfd, NULL);
        pthread_exit(NULL);
    }

    /* subscribe */
    mqtt_subscribe(&client, topic, 0);
    mqtt_subscribe(&client, topic2, 0);
    mqtt_subscribe(&client, topic3, 0);
    mqtt_subscribe(&client, topic4, 0);
    mqtt_subscribe(&client, topic5, 0);
    mqtt_subscribe(&client, topic6, 0);
    mqtt_subscribe(&client, topic7, 0);
    mqtt_subscribe(&client, topic8, 0);
    mqtt_subscribe(&client, topic9, 0);
    mqtt_subscribe(&client, topic10, 0);
    mqtt_subscribe(&client, topic11, 0);
    mqtt_subscribe(&client, topic12, 0);
/* start publishing the time */
#ifdef DEBUG_MQTT
    printf("listening for '%s' messages.\n", topic);
    printf("listening for '%s' messages.\n", topic2);
    printf("listening for '%s' messages.\n", topic3);
    printf("listening for '%s' messages.\n", topic4);
    printf("listening for '%s' messages.\n", topic5);
    printf("listening for '%s' messages.\n", topic6);
    printf("listening for '%s' messages.\n", topic7);
    printf("listening for '%s' messages.\n", topic8);
    printf("listening for '%s' messages.\n", topic9);
    printf("listening for '%s' messages.\n", topic10);
    printf("listening for '%s' messages.\n", topic11);
    printf("listening for '%s' messages.\n", topic12);
#endif
    char data_mode_active[1000] = {0};
    int data_mode_active_len = sprintf(data_mode_active, "CPU-{\"CMD\": %d,\"connect\":%d}", CMD_CPU_TO_QT_CONNECTION, checkconnect);
    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
    memset(data_mode_active, 0, 1000);

    while (1)
    {
        if (sockfd_bk == -1)
        {
            sockfd = open_nb_socket(urlserver, port);
            printf("sockfd: %d\n", sockfd);
            if (sockfd == -1)
            {
                printf("Failed to open socket\n");
                perror("Failed to open socket: ");
                checkconnect = 0;
                sockfd_bk = -1;
                // exit_example(EXIT_FAILURE, sockfd, NULL);
                pthread_exit(NULL);
            }
            else
            {
                checkconnect = 1;
                sockfd_bk = 0;
            }

            uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
            uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
            mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
            /* Create an anonymous session */
            /* Ensure we have a clean session */
            uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
            /* Send connection request to the broker. */
            mqtt_connect(&client, client_id, NULL, NULL, 0, username, password, connect_flags, 400);
            printf("sockfd: %d\n", MQTT_OK);
            /* check that we don't have any errors */
            if (client.error != MQTT_OK)
            {
                printf("Failed %s\n", mqtt_error_str(client.error));
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, NULL);
            }

            /* start a thread to refresh the client (handle egress and ingree client traffic) */

            if (pthread_create(&client_daemon, NULL, client_refresher, &client))
            {
                fprintf(stderr, "Failed to start client daemon.\n");
                printf("Failed to start client daemon.\n");
                // exit_example(EXIT_FAILURE, sockfd, NULL);
            }
            /* subscribe */
            mqtt_subscribe(&client, topic, 0);
            mqtt_subscribe(&client, topic2, 0);
            mqtt_subscribe(&client, topic3, 0);
            mqtt_subscribe(&client, topic4, 0);
            mqtt_subscribe(&client, topic5, 0);
            mqtt_subscribe(&client, topic6, 0);
            mqtt_subscribe(&client, topic7, 0);
            mqtt_subscribe(&client, topic8, 0);
            mqtt_subscribe(&client, topic9, 0);
            mqtt_subscribe(&client, topic10, 0);
            mqtt_subscribe(&client, topic11, 0);
            mqtt_subscribe(&client, topic12, 0);
/* start publishing the time */
#ifdef DEBUG_MQTT

            printf("listening for '%s' messages.\n", topic);
            printf("listening for '%s' messages.\n", topic2);
            printf("listening for '%s' messages.\n", topic3);
            printf("listening for '%s' messages.\n", topic4);
            printf("listening for '%s' messages.\n", topic5);
            printf("listening for '%s' messages.\n", topic6);
            printf("listening for '%s' messages.\n", topic7);
            printf("listening for '%s' messages.\n", topic8);
            printf("listening for '%s' messages.\n", topic9);
            printf("listening for '%s' messages.\n", topic10);
            printf("listening for '%s' messages.\n", topic11);
            printf("listening for '%s' messages.\n", topic12);
#endif
            char data_mode_active[1000] = {0};
            int data_mode_active_len = sprintf(data_mode_active, "CPU-{\"CMD\": %d,\"connect\":%d}", CMD_CPU_TO_QT_CONNECTION, checkconnect);
            cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
            memset(data_mode_active, 0, 1000);
        }

        if (flag_control.flag_power == 1 || flag_onoffstatus == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"mode\": %d}", power_fb);
            mqtt_publish(&client, topicpub, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_power = 0;
            flag_onoffstatus = 0;
        }
        if (flag_control.flag_info == 1 || flag_inforstatus == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"mode\": %d, \"name\": \"%s\", \"typeconnect\": %d, \"val1\": %d, \"val2\": %d, \"pkwh\": %0.2f}", power_fb, mac_final, 1, fb_dim_port1.lampport, fb_dim_port2.lampport, 0.29);
            //, power_fb, mac_final,1, counters_package_p2.val1, counters_package.val1, 0.29);
            //, power_fb, mac_final,1, 6, 7, 0.29);
            mqtt_publish(&client, topicpub, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));

                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_info = 0;
            flag_inforstatus = 0;
        }

        if (flag_control.flag_setgroup == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"setidtogroup\": %d}", 1);
            mqtt_publish(&client, topicpub2, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_setgroup = 0;
        }

        if (flag_control.flag_setgroupschedule == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"setgroupschedule\": %d}", 1);
            mqtt_publish(&client, topicpub3, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_setgroupschedule = 0;
        }
        if (flag_control.flag_lightschedulesetting == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"lightschedulesetting\": %d}", 1);
            mqtt_publish(&client, topicpub4, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_lightschedulesetting = 0;
        }
        if (flag_control.flag_dimid == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"onlyoneschedulesetting\": %d}", 1);
            mqtt_publish(&client, topicpub5, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_dimid = 0;
        }
        if (flag_control.flag_groupschedulesetting == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"groupschedulesetting\": %d}", 1);
            mqtt_publish(&client, topicpub6, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_groupschedulesetting = 0;
        }
        if (flag_control.flag_requestpowerschedule == 1)
        {
            char message1[1280] = {0};
            char message2[1280] = {0};
            char message4[2056] = {0};
            if (time_on_off.size_array_on_off_schedule == 1)
            {
                snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                         time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
            }
            else
            {
                for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
                {
                    if (i == 0)
                    {
                        snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                 time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                    }
                    else
                    {
                        snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                 time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                    }
                    strcat(message1, message2);
                    printf("message1: %s\r\n", message1);
                }
            }
            snprintf(message4, sizeof(message4), "[%s]", message1);
            mqtt_publish(&client, topicpub7, message4, strlen(message4), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message4);
#endif
            // memset(message4, 0, 1280);
            flag_control.flag_requestpowerschedule = 0;
        }
        if (flag_control.flag_requestlightschedule == 1)
        {
            dim_schedule_active = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_active.txt", dim_schedule_active);
            char message3[1280];
            snprintf(message3, sizeof(message3), "{\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                     200,
                     dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1, dim_schedule_active.dim1,
                     dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2, dim_schedule_active.dim2,
                     dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3, dim_schedule_active.dim3,
                     dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4, dim_schedule_active.dim4,
                     dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5, dim_schedule_active.dim5,
                     dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6, dim_schedule_active.dim6,
                     dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7, dim_schedule_active.dim7,
                     dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8, dim_schedule_active.dim8,
                     dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9, dim_schedule_active.dim9);
            mqtt_publish(&client, topicpub8, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif
            flag_control.flag_requestlightschedule = 0;
        }
        if (flag_control.flag_requestgroupschedule == 1)
        {
            char message4[1280] = {0};
            char message5[1280] = {0};
            char message6[1280] = {0};
            char message7[10000] = {0};
            dim_schedule_group_1 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_1.txt", dim_schedule_group_1);
            dim_schedule_group_2 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
            dim_schedule_group_3 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
            if (dim_schedule_group_1.group == 1)
            {
                // char message4[1280];
                snprintf(message4, sizeof(message4), "{\"onOff\":1,\"port\":1,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         1,
                         dim_schedule_group_1.hh_start_dim1, dim_schedule_group_1.mm_start_dim1, dim_schedule_group_1.dim1,
                         dim_schedule_group_1.hh_start_dim2, dim_schedule_group_1.mm_start_dim2, dim_schedule_group_1.dim2,
                         dim_schedule_group_1.hh_start_dim3, dim_schedule_group_1.mm_start_dim3, dim_schedule_group_1.dim3,
                         dim_schedule_group_1.hh_start_dim4, dim_schedule_group_1.mm_start_dim4, dim_schedule_group_1.dim4,
                         dim_schedule_group_1.hh_start_dim5, dim_schedule_group_1.mm_start_dim5, dim_schedule_group_1.dim5,
                         dim_schedule_group_1.hh_start_dim6, dim_schedule_group_1.mm_start_dim6, dim_schedule_group_1.dim6,
                         dim_schedule_group_1.hh_start_dim7, dim_schedule_group_1.mm_start_dim7, dim_schedule_group_1.dim7,
                         dim_schedule_group_1.hh_start_dim8, dim_schedule_group_1.mm_start_dim8, dim_schedule_group_1.dim8,
                         dim_schedule_group_1.hh_start_dim9, dim_schedule_group_1.mm_start_dim9, dim_schedule_group_1.dim9);
            }
            if (dim_schedule_group_1.group == 99)
            {
                // char message4[1280];
                snprintf(message4, sizeof(message4), "{\"onOff\":0,\"port\":1,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         1,
                         dim_schedule_group_1.hh_start_dim1, dim_schedule_group_1.mm_start_dim1, dim_schedule_group_1.dim1,
                         dim_schedule_group_1.hh_start_dim2, dim_schedule_group_1.mm_start_dim2, dim_schedule_group_1.dim2,
                         dim_schedule_group_1.hh_start_dim3, dim_schedule_group_1.mm_start_dim3, dim_schedule_group_1.dim3,
                         dim_schedule_group_1.hh_start_dim4, dim_schedule_group_1.mm_start_dim4, dim_schedule_group_1.dim4,
                         dim_schedule_group_1.hh_start_dim5, dim_schedule_group_1.mm_start_dim5, dim_schedule_group_1.dim5,
                         dim_schedule_group_1.hh_start_dim6, dim_schedule_group_1.mm_start_dim6, dim_schedule_group_1.dim6,
                         dim_schedule_group_1.hh_start_dim7, dim_schedule_group_1.mm_start_dim7, dim_schedule_group_1.dim7,
                         dim_schedule_group_1.hh_start_dim8, dim_schedule_group_1.mm_start_dim8, dim_schedule_group_1.dim8,
                         dim_schedule_group_1.hh_start_dim9, dim_schedule_group_1.mm_start_dim9, dim_schedule_group_1.dim9);
            }
            if (dim_schedule_group_2.group == 2)
            {
                // char message5[1280];
                snprintf(message5, sizeof(message5), "{\"onOff\":1,\"port\":1,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         2,
                         dim_schedule_group_2.hh_start_dim1, dim_schedule_group_2.mm_start_dim1, dim_schedule_group_2.dim1,
                         dim_schedule_group_2.hh_start_dim2, dim_schedule_group_2.mm_start_dim2, dim_schedule_group_2.dim2,
                         dim_schedule_group_2.hh_start_dim3, dim_schedule_group_2.mm_start_dim3, dim_schedule_group_2.dim3,
                         dim_schedule_group_2.hh_start_dim4, dim_schedule_group_2.mm_start_dim4, dim_schedule_group_2.dim4,
                         dim_schedule_group_2.hh_start_dim5, dim_schedule_group_2.mm_start_dim5, dim_schedule_group_2.dim5,
                         dim_schedule_group_2.hh_start_dim6, dim_schedule_group_2.mm_start_dim6, dim_schedule_group_2.dim6,
                         dim_schedule_group_2.hh_start_dim7, dim_schedule_group_2.mm_start_dim7, dim_schedule_group_2.dim7,
                         dim_schedule_group_2.hh_start_dim8, dim_schedule_group_2.mm_start_dim8, dim_schedule_group_2.dim8,
                         dim_schedule_group_2.hh_start_dim9, dim_schedule_group_2.mm_start_dim9, dim_schedule_group_2.dim9);
            }
            if (dim_schedule_group_2.group == 98)
            {
                // char message5[1280];
                snprintf(message5, sizeof(message5), "{\"onOff\":0,\"port\":1,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         2,
                         dim_schedule_group_2.hh_start_dim1, dim_schedule_group_2.mm_start_dim1, dim_schedule_group_2.dim1,
                         dim_schedule_group_2.hh_start_dim2, dim_schedule_group_2.mm_start_dim2, dim_schedule_group_2.dim2,
                         dim_schedule_group_2.hh_start_dim3, dim_schedule_group_2.mm_start_dim3, dim_schedule_group_2.dim3,
                         dim_schedule_group_2.hh_start_dim4, dim_schedule_group_2.mm_start_dim4, dim_schedule_group_2.dim4,
                         dim_schedule_group_2.hh_start_dim5, dim_schedule_group_2.mm_start_dim5, dim_schedule_group_2.dim5,
                         dim_schedule_group_2.hh_start_dim6, dim_schedule_group_2.mm_start_dim6, dim_schedule_group_2.dim6,
                         dim_schedule_group_2.hh_start_dim7, dim_schedule_group_2.mm_start_dim7, dim_schedule_group_2.dim7,
                         dim_schedule_group_2.hh_start_dim8, dim_schedule_group_2.mm_start_dim8, dim_schedule_group_2.dim8,
                         dim_schedule_group_2.hh_start_dim9, dim_schedule_group_2.mm_start_dim9, dim_schedule_group_2.dim9);
            }

            if (dim_schedule_group_3.group == 3)
            {
                // char message6[1280];
                snprintf(message6, sizeof(message6), "{\"onOff\":1,\"port\":2,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         3,
                         dim_schedule_group_3.hh_start_dim1, dim_schedule_group_3.mm_start_dim1, dim_schedule_group_3.dim1,
                         dim_schedule_group_3.hh_start_dim2, dim_schedule_group_3.mm_start_dim2, dim_schedule_group_3.dim2,
                         dim_schedule_group_3.hh_start_dim3, dim_schedule_group_3.mm_start_dim3, dim_schedule_group_3.dim3,
                         dim_schedule_group_3.hh_start_dim4, dim_schedule_group_3.mm_start_dim4, dim_schedule_group_3.dim4,
                         dim_schedule_group_3.hh_start_dim5, dim_schedule_group_3.mm_start_dim5, dim_schedule_group_3.dim5,
                         dim_schedule_group_3.hh_start_dim6, dim_schedule_group_3.mm_start_dim6, dim_schedule_group_3.dim6,
                         dim_schedule_group_3.hh_start_dim7, dim_schedule_group_3.mm_start_dim7, dim_schedule_group_3.dim7,
                         dim_schedule_group_3.hh_start_dim8, dim_schedule_group_3.mm_start_dim8, dim_schedule_group_3.dim8,
                         dim_schedule_group_3.hh_start_dim9, dim_schedule_group_3.mm_start_dim9, dim_schedule_group_3.dim9);
            }
            if (dim_schedule_group_3.group == 97)
            {
                // char message6[1280];
                snprintf(message6, sizeof(message6), "{\"onOff\":0,\"port\":2,\"group\":%d,\"schedule\":[{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d},{\"hh_start_dim\":%d,\"mm_start_dim\":%d,\"dimmer\":%d}]}",
                         3,
                         dim_schedule_group_3.hh_start_dim1, dim_schedule_group_3.mm_start_dim1, dim_schedule_group_3.dim1,
                         dim_schedule_group_3.hh_start_dim2, dim_schedule_group_3.mm_start_dim2, dim_schedule_group_3.dim2,
                         dim_schedule_group_3.hh_start_dim3, dim_schedule_group_3.mm_start_dim3, dim_schedule_group_3.dim3,
                         dim_schedule_group_3.hh_start_dim4, dim_schedule_group_3.mm_start_dim4, dim_schedule_group_3.dim4,
                         dim_schedule_group_3.hh_start_dim5, dim_schedule_group_3.mm_start_dim5, dim_schedule_group_3.dim5,
                         dim_schedule_group_3.hh_start_dim6, dim_schedule_group_3.mm_start_dim6, dim_schedule_group_3.dim6,
                         dim_schedule_group_3.hh_start_dim7, dim_schedule_group_3.mm_start_dim7, dim_schedule_group_3.dim7,
                         dim_schedule_group_3.hh_start_dim8, dim_schedule_group_3.mm_start_dim8, dim_schedule_group_3.dim8,
                         dim_schedule_group_3.hh_start_dim9, dim_schedule_group_3.mm_start_dim9, dim_schedule_group_3.dim9);
            }
            snprintf(message7, sizeof(message7), "[%s,%s,%s]", message4, message5, message6);
            mqtt_publish(&client, topicpub9, message7, strlen(message7), MQTT_PUBLISH_QOS_1);
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message7);
#endif
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
            flag_control.flag_requestgroupschedule = 0;
        }
        
        if (flag_control.flag_synctime == 1)
        {
            type_date_time_t type_date_time_update_rtc;
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"synctime\": %d}", 1);
            mqtt_publish(&client, topicpub10, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif

            // dang thu de dong bo thoi gian tu app xuong master
            delay(500);
            struct timeval curTime;
            struct tm *my_date_time;

            gettimeofday(&curTime, NULL);
            my_date_time = localtime(&curTime.tv_sec);
            printf("%d gio %d phut %d giay\r\n", my_date_time->tm_hour, my_date_time->tm_min, my_date_time->tm_sec);
            printf("ngay %d thang %d nam %d\r\n", my_date_time->tm_mday, my_date_time->tm_mon + 1, my_date_time->tm_year - 100);

            type_date_time_update_rtc.year = (my_date_time->tm_year - 100) + 2000;
            type_date_time_update_rtc.month = my_date_time->tm_mon + 1;
            type_date_time_update_rtc.date = my_date_time->tm_mday;
            type_date_time_update_rtc.hour = my_date_time->tm_hour;
            type_date_time_update_rtc.minute = my_date_time->tm_min;
            type_date_time_update_rtc.seconds = my_date_time->tm_sec;
            send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&type_date_time_update_rtc, sizeof(type_date_time_t));
            
            flag_control.flag_synctime = 0;
        }

        if (flag_control.flag_diminformation == 1 || flag_dimstatus == 1)
        {
            char message3[1500] = {0};
            char message4[1500] = {0};
            char message5[1300] = {0};
            char message6[1400] = {0};
            char message7[1300] = {0};
            char message8[1400] = {0};
            char message9[4096] = {0};
            // Lỗi khi nhập 1 đèn
            snprintf(message5, sizeof(message5), "%d,", fb_dim_port1.dim_fb[1]);
            snprintf(message7, sizeof(message7), "%d,", fb_dim_port2.dim_fb[1]);
            for (int i = 2; i <= fb_dim_port1.lampport; i++)
            {
                // if (i == 1)
                // snprintf(message5, sizeof(message5), "%d,",fb_dim_port1.dim_fb[1]);
                if (i == fb_dim_port1.lampport)
                {
                    snprintf(message6, sizeof(message6), "%s%d", message5, fb_dim_port1.dim_fb[i]);
                    snprintf(message3, sizeof(message3), "{\"port\": %d, \"dim\":[%s]}", 1, message6);
                }
                else
                {
                    snprintf(message6, sizeof(message6), "%s%d,", message5, fb_dim_port1.dim_fb[i]);
                    strcpy(message5, message6);
                }
            }

            for (int i = 2; i <= fb_dim_port2.lampport; i++)
            {
                // if (i == 1)
                // snprintf(message7, sizeof(message7), "%d,",fb_dim_port2.dim_fb[1]);
                if (i == fb_dim_port2.lampport)
                {
                    snprintf(message8, sizeof(message8), "%s%d", message7, fb_dim_port2.dim_fb[i]);
                    snprintf(message4, sizeof(message4), ",{\"port\": %d, \"dim\":[%s]}", 2, message8);
                }
                else
                {
                    snprintf(message8, sizeof(message8), "%s%d,", message7, fb_dim_port2.dim_fb[i]);
                    strcpy(message7, message8);
                }
            }
            snprintf(message9, sizeof(message9), "[%s%s]", message3, message4);
            mqtt_publish(&client, topicpub11, message9, strlen(message9), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message9);
#endif
            flag_control.flag_diminformation = 0;
            flag_dimstatus = 0;
        }
        /// Chart
        if (flag_chart == 1)
        {
            char message3[1280] = {0};
            snprintf(message3, sizeof(message3), "{\"pkw\": %.02f, \"pkwh\": %.02f, \"val1\": %d, \"val2\": %d, \"V1\": %.02f, \"V2\": %.02f, \"V3\": %.02f, \"I1\": %.02f, \"I2\": %.02f, \"I3\": %.02f, \"P1\": %.02f, \"P2\": %.02f, \"P3\": %.02f, \"Pf1\": %.02f, \"Pf2\": %.02f, \"Pf3\": %.02f}", parameter_MFM384.P_Total_MFM384, parameter_MFM384.P_Consumtion_kWH,
                     fb_dim_port1.lampport, fb_dim_port2.lampport,
                     // counters_package_p2.val1, counters_package.val1,
                     parameter_MFM384.V1_MFM384,
                     parameter_MFM384.V2_MFM384,
                     parameter_MFM384.V3_MFM384,
                     parameter_MFM384.I1_MFM384,
                     parameter_MFM384.I2_MFM384,
                     parameter_MFM384.I3_MFM384,
                     parameter_MFM384.P1_MFM384,
                     parameter_MFM384.P2_MFM384,
                     parameter_MFM384.P3_MFM384,
                     fabs(parameter_MFM384.PF1_MFM384),
                     fabs(parameter_MFM384.PF2_MFM384),
                     fabs(parameter_MFM384.PF3_MFM384));
            printf("topicpub12: \"%s\"\r\n", topicpub12);
            mqtt_publish(&client, topicpub12, message3, strlen(message3), MQTT_PUBLISH_QOS_1);
            if (client.error != MQTT_OK)
            {
                fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
                // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
                sockfd_bk = -1;
            }
#ifdef DEBUG_MQTT
            printf("published feedback: \"%s\"\r\n", message3);
#endif

            // sockfd_bk = -1;
            //  flag_info = 0;
            flag_chart = 0;
        }
    }
}

// Thread read modbus
#pragma region LUONG DOC MODBUS
void *ModBus(void *threadArgs)
{
#if defined(MODBUS_SIMULATOR)

    while (1)
    {
        if (is_flag_opened_websocket)
        {
            delay(10);
            parameter_MFM384.V1_MFM384 = Random(300);
            parameter_MFM384.V2_MFM384 = Random(300);
            parameter_MFM384.V3_MFM384 = Random(300);

            parameter_MFM384.I1_MFM384 = Random(1);
            parameter_MFM384.I2_MFM384 = Random(1);
            parameter_MFM384.I3_MFM384 = Random(1);
            parameter_MFM384.P1_MFM384 = Random(200);
            parameter_MFM384.P2_MFM384 = Random(200);
            parameter_MFM384.P3_MFM384 = Random(200);
            parameter_MFM384.P_Total_MFM384 = Random(200);
            parameter_MFM384.PF1_MFM384 = Random(1);
            parameter_MFM384.PF2_MFM384 = Random(1);
            parameter_MFM384.PF3_MFM384 = Random(1);
            parameter_MFM384.F1_MFM384 = Random(50);
            parameter_MFM384.F2_MFM384 = Random(50);
            parameter_MFM384.F3_MFM384 = Random(50);
            parameter_MFM384.F3_MFM384 = Random(50);

#ifdef DEBUG_MODBUS
            printf("--------------------------------------------------Parameter Power Update Modbus Simulate----------------------------------------------------\r\n");
            printf("\r\n");

            printf("V1: %.02f --- ", parameter_MFM384.V1_MFM384);
            printf("V2: %.02f --- ", parameter_MFM384.V2_MFM384);
            printf("V3: %.02f --- ", parameter_MFM384.V3_MFM384);

            printf("I1: %.02f --- ", parameter_MFM384.I1_MFM384);
            printf("I2: %.02f --- ", parameter_MFM384.I2_MFM384);
            printf("I3: %.02f --- ", parameter_MFM384.I3_MFM384);

            printf("P1: %.02f --- ", parameter_MFM384.P1_MFM384);
            printf("P2: %.02f --- ", parameter_MFM384.P2_MFM384);
            printf("P3: %.02f --- ", parameter_MFM384.P3_MFM384);
            printf("ToTal Power: %.02f --- ", parameter_MFM384.P_Total_MFM384);

            printf("PF1: %.02f --- ", parameter_MFM384.PF1_MFM384);
            printf("PF2: %.02f --- ", parameter_MFM384.PF2_MFM384);
            printf("PF3: %.02f --- ", parameter_MFM384.PF3_MFM384);

            printf("F1: %.02fHz --- ", parameter_MFM384.F1_MFM384);
            printf("F2: %.02fHz ---", parameter_MFM384.F2_MFM384);
            printf("F3: %.02fHz", parameter_MFM384.F3_MFM384);
            printf("\r\n---------------------------------------------------------------------------------------------------------------------------\r\n");
            printf("\r\n");
#endif
            // Encode Json send to websocket
            char parameter_modbus_read[1000] = {0};
            int parameter_modbus_read_len = sprintf(parameter_modbus_read, TEMPLATE_POWER,
                                                    CMD_CPU_TO_QT_PARAMETER_PWR,
                                                    parameter_MFM384.V1_MFM384,
                                                    parameter_MFM384.V2_MFM384,
                                                    parameter_MFM384.V3_MFM384,
                                                    parameter_MFM384.I1_MFM384,
                                                    parameter_MFM384.I2_MFM384,
                                                    parameter_MFM384.I3_MFM384,
                                                    parameter_MFM384.P1_MFM384,
                                                    parameter_MFM384.P2_MFM384,
                                                    parameter_MFM384.P3_MFM384,
                                                    parameter_MFM384.P_Total_MFM384,
                                                    parameter_MFM384.PF1_MFM384,
                                                    parameter_MFM384.PF2_MFM384,
                                                    parameter_MFM384.PF3_MFM384,
                                                    parameter_MFM384.F1_MFM384,
                                                    parameter_MFM384.F2_MFM384,
                                                    parameter_MFM384.F3_MFM384);

            // Send to websocket local data
            cl->send_ex(cl, UWSC_OP_TEXT, 1, parameter_modbus_read_len, parameter_modbus_read);
            // Memset data for send again
            memset(parameter_modbus_read, 0, 1000);
            delay(1000);
        }
    }
#elif defined(MODBUS_MF384AC)
    /* Handle read parameter from MF384AC */
    modbus_t *ctx = modbus_new_rtu("/dev/ttyAMA2", 9600, 'N', 8, 1);
    if (!ctx)
    {
        fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
        exit(1);
    }
#ifdef DEBUG_MODBUS
    printf("khoi tao thanh cong modbus pm select\r\n");
#endif
    delay(100);
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        exit(1);
    }
    modbus_set_slave(ctx, REMOTE_ID_PM_SELECT);
#ifdef DEBUG_MODBUS
    printf("Ket noi modbus RTU thanh cong\r\n");
#endif
    delay(1000);
#endif
    /* Initizaline modbus sensor */
    modbus_t *ctx_ss_light_l = modbus_new_rtu("/dev/ttyAMA2", 9600, 'N', 8, 1);
    if (!ctx_ss_light_l)
    {
        fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
        // exit(1);
    }
#ifdef DEBUG_MODBUS
    printf("RS485 connect sensor left khoi tao thanh cong\r\n");
#endif
    delay(100);
    if (modbus_connect(ctx_ss_light_l) == -1)
    {
        fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
        modbus_free(ctx_ss_light_l);
        // continue;
        // exit(1);
    }
    // Set remote id sensor ambient light LEFT
    modbus_set_slave(ctx_ss_light_l, REMOTE_ID_SS_LIGHT_L);
#ifdef DEBUG_MODBUS
    printf("Ket noi modbus sensor light 1 thanh cong\r\n");
#endif
    delay(200);

    modbus_t *ctx_ss_light_r = modbus_new_rtu("/dev/ttyAMA2", 9600, 'N', 8, 1);
    if (!ctx_ss_light_r)
    {
        fprintf(stderr, "Failed to create the context: %s\n", modbus_strerror(errno));
        // continue;
        // exit(1);
    }
#ifdef DEBUG_MODBUS
    printf("RS485 connect sensor left khoi tao thanh cong\r\n");
#endif
    delay(100);
    if (modbus_connect(ctx_ss_light_r) == -1)
    {
        fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
        modbus_free(ctx_ss_light_r);
        // continue;
        // exit(1);
    }
    // Set remote id sensor ambient light LEFT
    modbus_set_slave(ctx_ss_light_r, REMOTE_ID_SS_LIGHT_R);
#ifdef DEBUG_MODBUS
    printf("Ket noi modbus sensor light 1 thanh cong\r\n");
#endif
    // Set remote id sensor rain
    modbus_t *ctx_ss_rain = modbus_new_rtu("/dev/ttyAMA2", 9600, 'N', 8, 1);
    if (!ctx_ss_rain)
    {
        fprintf(stderr, "Failed to create sensor rain the context: %s\n", modbus_strerror(errno));
        // continue;
        // exit(1);
    }
#ifdef DEBUG_MODBUS
    printf("RS485 connect sensor rain khoi tao thanh cong\r\n");
#endif
    delay(100);
    if (modbus_connect(ctx_ss_rain) == -1)
    {
        fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
        modbus_free(ctx_ss_rain);
        // continue;
        // exit(1);
    }
    // Set remote id sensor ambient light LEFT
    modbus_set_slave(ctx_ss_rain, REMOTE_ID_SS_RAIN);
#ifdef DEBUG_MODBUS
    printf("Ket noi modbus sensor rain  thanh cong\r\n");
#endif
    // Set remote id sensor env
    modbus_t *ctx_ss_env = modbus_new_rtu("/dev/ttyAMA2", 9600, 'N', 8, 1);
    if (!ctx_ss_env)
    {
        fprintf(stderr, "Failed to create sensor env the context: %s\n", modbus_strerror(errno));
        // continue;
        // exit(1);
    }
#ifdef DEBUG_MODBUS
    printf("RS485 connect sensor env khoi tao thanh cong\r\n");
#endif
    delay(100);
    if (modbus_connect(ctx_ss_env) == -1)
    {
        fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
        modbus_free(ctx_ss_env);
        // continue;
        // exit(1);
    }
    // Set remote id sensor ambient light LEFT
    modbus_set_slave(ctx_ss_env, REMOTE_ID_SS_TEMP_HUM);
#ifdef DEBUG_MODBUS
    printf("Ket noi modbus sensor env  thanh cong\r\n");
#endif
    while (1)
    {
        delay(1);
        if (is_flag_opened_websocket)
        {
            /*----------------------------------------------Doc U, I, P, Cos Pi-------------------------------------------------------------*/
            uint16_t reg[200] = {0}; // will store read registers values
            int num = modbus_read_input_registers(ctx, 0, 60, reg);
            if (num != 60)
            {
                fprintf(stderr, "Failed to read: %s\n", modbus_strerror(errno));
                modbus_free(ctx);
                is_check_reset_thread_modbus = true;
            }
            // parameter_MFM384_t parameter_MFM384;
            parameter_MFM384.V1_MFM384 = convert_array_uint16_to_float(reg, 0, 1);
            parameter_MFM384.V2_MFM384 = convert_array_uint16_to_float(reg, 2, 3);
            parameter_MFM384.V3_MFM384 = convert_array_uint16_to_float(reg, 4, 5);

            parameter_MFM384.I1_MFM384 = convert_array_uint16_to_float(reg, 16, 17);
            parameter_MFM384.I2_MFM384 = convert_array_uint16_to_float(reg, 18, 19);
            parameter_MFM384.I3_MFM384 = convert_array_uint16_to_float(reg, 20, 21);

            parameter_MFM384.P1_MFM384 = convert_array_uint16_to_float(reg, 24, 25);
            parameter_MFM384.P2_MFM384 = convert_array_uint16_to_float(reg, 26, 27);
            parameter_MFM384.P3_MFM384 = convert_array_uint16_to_float(reg, 28, 29);
            parameter_MFM384.P_Total_MFM384 = convert_array_uint16_to_float(reg, 42, 43);

            parameter_MFM384.PF1_MFM384 = convert_array_uint16_to_float(reg, 48, 49);
            parameter_MFM384.PF2_MFM384 = convert_array_uint16_to_float(reg, 50, 51);
            parameter_MFM384.PF3_MFM384 = convert_array_uint16_to_float(reg, 52, 53);

            parameter_MFM384.F1_MFM384 = convert_array_uint16_to_float(reg, 56, 57);
            parameter_MFM384.F2_MFM384 = convert_array_uint16_to_float(reg, 56, 57);
            parameter_MFM384.F3_MFM384 = convert_array_uint16_to_float(reg, 56, 57);
            parameter_MFM384.P_Consumtion_kWH = convert_array_uint16_to_float(reg, 58, 59);

#ifdef DEBUG_MODBUS
            /*----------------------------------------------Doc U, I, P, Cos Pi------------------------------------------------------------*/
            printf("--------------------------------------------------Parameter Power Update----------------------------------------------------\r\n");
            printf("\r\n");

            printf("V1: %.02f --- ", parameter_MFM384.V1_MFM384);
            printf("V2: %.02f --- ", parameter_MFM384.V2_MFM384);
            printf("V3: %.02f --- ", parameter_MFM384.V3_MFM384);

            printf("I1: %.02f --- ", parameter_MFM384.I1_MFM384);
            printf("I2: %.02f --- ", parameter_MFM384.I2_MFM384);
            printf("I3: %.02f --- ", parameter_MFM384.I3_MFM384);

            printf("P1: %.02f --- ", parameter_MFM384.P1_MFM384);
            printf("P2: %.02f --- ", parameter_MFM384.P2_MFM384);
            printf("P3: %.02f --- ", parameter_MFM384.P3_MFM384);
            printf("ToTal Power: %.02f --- ", parameter_MFM384.P_Total_MFM384);

            printf("PF1: %.02f --- ", parameter_MFM384.PF1_MFM384);
            printf("PF2: %.02f --- ", parameter_MFM384.PF2_MFM384);
            printf("PF3: %.02f --- ", parameter_MFM384.PF3_MFM384);

            printf("F1: %.02fHz --- ", parameter_MFM384.F1_MFM384);
            printf("F2: %.02fHz ---", parameter_MFM384.F2_MFM384);
            printf("F3: %.02fHz", parameter_MFM384.F3_MFM384);
            printf("\r\n---------------------------------------------------------------------------------------------------------------------------\r\n");
            printf("\r\n");
#endif
            char data[1000] = {0};
            int data_len = sprintf(data, TEMPLATE_POWER,
                                   CMD_CPU_TO_QT_PARAMETER_PWR,
                                   parameter_MFM384.V1_MFM384,
                                   parameter_MFM384.V2_MFM384,
                                   parameter_MFM384.V3_MFM384,
                                   parameter_MFM384.I1_MFM384,
                                   parameter_MFM384.I2_MFM384,
                                   parameter_MFM384.I3_MFM384,
                                   parameter_MFM384.P1_MFM384,
                                   parameter_MFM384.P2_MFM384,
                                   parameter_MFM384.P3_MFM384,
                                   parameter_MFM384.P_Total_MFM384,
                                   parameter_MFM384.PF1_MFM384,
                                   parameter_MFM384.PF2_MFM384,
                                   parameter_MFM384.PF3_MFM384,
                                   parameter_MFM384.F1_MFM384,
                                   parameter_MFM384.F2_MFM384,
                                   parameter_MFM384.F3_MFM384,
                                   parameter_MFM384.P_Consumtion_kWH);
            cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
            // int data_len = strlen(data);
            // cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
            memset(data, 0, 1000);
            delay(1000);
            // send_data_cpu_to_qt(queue.inp_arr, queue.rear, TEMPLATE_NUMBER_GROUP_DIM1, CMD_CPU_TO_QT_LAMP_RF_GROUP_DIM1);
            // delay(2000);
            // send_data_cpu_to_qt(queue_lamps_port1.inp_arr, queue_lamps_port1.rear, TEMPLATE_NUMBER_GROUP_PORT1, CMD_QT_TO_CPU_SET_LAMP_RS485_PORT_TO_GROUP);
            // delay(2000);
            // send_data_cpu_to_qt(queue_lamps_port2.inp_arr, queue_lamps_port2.rear, TEMPLATE_NUMBER_GROUP_PORT2, CMD_QT_TO_CPU_SET_LAMP_RS485_PORT2_TO_GROUP);
            // delay(8000);
            flag_chart = 1;
        }
        uint16_t reg_light_l[10] = {0};
        uint16_t reg_light_r[10] = {0};
        uint16_t reg_rain[10] = {0};
        uint16_t reg_env[10] = {0};

        int number_mb_ambient_light_L = modbus_read_input_registers(ctx_ss_light_l, 0, 10, reg_light_l);
        delay(1000);
        int number_mb_ambient_light_R = modbus_read_input_registers(ctx_ss_light_r, 0, 10, reg_light_r);
        delay(1000);
        int number_mb_rain = modbus_read_input_registers(ctx_ss_rain, 0, 10, reg_rain);
        delay(1000);
        int number_mb_env = modbus_read_input_registers(ctx_ss_env, 0, 10, reg_env);
        delay(1000);
        if (number_mb_ambient_light_R != 10)
        {
            fprintf(stderr, "Failed to read sensor light right: %s\n", modbus_strerror(errno));
            is_check_timeout_sensor_light_1 = true;
        }
        else
        {
            is_check_timeout_sensor_light_1 = false;
        }
        if (number_mb_ambient_light_L != 10)
        {
            fprintf(stderr, "Failed to read sensor light left: %s\n", modbus_strerror(errno));
            is_check_timeout_sensor_light_2 = true;
        }
        else
        {
            is_check_timeout_sensor_light_2 = false;
        }

        if (is_check_timeout_sensor_light_1 == false && is_check_timeout_sensor_light_2 == false)
        {
            is_connect_sensor = true;
        }
        else
        {
            is_connect_sensor = false;
        }
        if (number_mb_rain != 10)
        {
            fprintf(stderr, "Failed to read sensor rain: %s\n", modbus_strerror(errno));
        }
        if (number_mb_env != 10)
        {
            fprintf(stderr, "Failed to read sensor env: %s\n", modbus_strerror(errno));
        }
        for (int i = 0; i < number_mb_rain; i++)
        {
            printf("reg rain[%d]=%d (0x%X)\n", i, reg_rain[i], reg_rain[i]);
        }
        for (int i = 0; i < number_mb_env; i++)
        {
            printf("reg env[%d]=%d (0x%X)\n", i, reg_env[i], reg_env[i]);
        }
        sys_sensor.value_sensor_rain = reg_rain[0];
        sys_sensor.value_hum_sensor = reg_env[0];
        sys_sensor.value_temp_sensor = reg_env[1];

        sys_sensor.value_sensor_present_1 = reg_light_l[3];
        sys_sensor.value_sensor_present_2 = reg_light_r[3];

        printf("Got value sensor 1 lux: [%d]\r\n", sys_sensor.value_sensor_present_1);
        printf("Got value sensor 2 lux: [%d]\r\n", sys_sensor.value_sensor_present_2);
        printf("Got value hum 1 lux: [%d]\r\n", sys_sensor.value_hum_sensor);
        printf("Got value temp 2 lux: [%d]\r\n", sys_sensor.value_temp_sensor);
    }
    modbus_close(ctx);
    modbus_free(ctx);
    modbus_close(ctx_ss_light_l);
    modbus_close(ctx_ss_light_r);
    modbus_close(ctx_ss_rain);
    modbus_close(ctx_ss_env);

    modbus_free(ctx_ss_light_l);
    modbus_free(ctx_ss_light_r);
    modbus_free(ctx_ss_rain);
    modbus_free(ctx_ss_env);
}
#pragma endregion

#pragma region LUONG DOC MODBUS CAM BIEN NGOAI VI
void *Register_Modbus_Sensor(void *threadArgs)
{
    /* cài đặt ngưỡng để test*/
    // mtlc_sensor.threshold_ambient_light = 400;
    while (1)
    {
        if (is_connect_sensor)
        {
            unsigned long long currentMillis = millis();
            unsigned long long currentMillis_CPU_60s = millis();
            if (currentMillis_CPU_60s - previousMillis_CPU_60s > interval_CPU_60s)
            {
                previousMillis_CPU_60s = currentMillis_CPU_60s;
                // mtlc_sensor_control(&sys_sensor, &current_time);
                char tmp_data[1200] = {0};
                int tmp_data_len = sprintf(tmp_data, TEMPLATE_STATUS_SENSOR_MONITOR, CMD_CPU_TO_QT_STATUS_SENSOR_MONITORING, is_check_timeout_sensor_light_1, is_check_timeout_sensor_light_2, 1, 1, 1, 1, sys_sensor.value_sensor_rain);
                printf("Monitor status sensor: %s\r\n", tmp_data);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, tmp_data_len, tmp_data);
                memset(tmp_data, 0x00, sizeof(tmp_data));
            }
            if (currentMillis - previousMillis_CPU_1s > interval_CPU_1s)
            {
                previousMillis_CPU_1s = currentMillis;
                send_byte(CMD_CPU_TO_MASTER_KEEP_ALLIVE_CPU, 1);
                mtlc_process_value_sensor(&sys_sensor, &current_time, &mtlc_sensor, &mtlc_force_cabinet);
                printf("time current local: %d:%d:%d\r\n", current_time.hh_current, current_time.mm_current, current_time.ss_current);
            }
        }
    }
}
#pragma endregion
#pragma region LUONG XU LY DATA SENSOR AMBIENT
void mtlc_sensor_control(modbus_ss_light_t *sys_sensor, current_time_t *current_time)
{
    char messStr[500];

    snprintf(messStr, sizeof(messStr), "{\"thoi gian\":%d:%d,\"light 1\":%d,\"light 2\":%d}", current_time->hh_current, current_time->mm_current, sys_sensor->value_sensor_present_1, sys_sensor->value_sensor_present_2);
    printf("got value messStr : %s\r\n", messStr);

    if (is_check_timeout_sensor_light_1 == false && is_check_timeout_sensor_light_2 == false)
    {
        fptr = fopen("/home/pi/data_log.txt", "a");
        if (fptr == NULL)
        {
            printf("Error!");
        }
        fprintf(fptr, "\n{\"thoi gian\":%d:%d,\"light 1\":%d,\"light 2\":%d}", current_time->hh_current, current_time->mm_current, sys_sensor->value_sensor_present_1, sys_sensor->value_sensor_present_2);
        fclose(fptr);
    }
}

void mtlc_read_file_sensor()
{
    char buffer[250];
    fptr = fopen("/home/pi/data_log.txt", "r");
    if (fptr != NULL)
    {
        while (fgets(buffer, sizeof(buffer), fptr))
        {
            printf("%s", buffer);
        }
    }
}

void mtlc_process_value_sensor(modbus_ss_light_t *sys_sensor, current_time_t *current_time, type_mtlc_sensor_t *mtlc_sensor_struct, type_mtlc_force_on_t *mtlc_force_cabinet)
{
    if (mtlc_sensor.check_enable == true)
    {
        int total_value_present_sensor = 0;
        unsigned long long currentMillis_sensor = millis();
        unsigned long long currentMillis_CPU_1800s = millis();
        unsigned long long currentMillis_CPU_2000s = millis();
        int check_time_current = current_time->hh_current * 60 + current_time->mm_current;

        if (is_check_timeout_sensor_light_1 == false && is_check_timeout_sensor_light_2 == false)
        {
            if ((check_time_current > ((mtlc_sensor.hh_start * 60) + mtlc_sensor.mm_start)) && (check_time_current < ((mtlc_sensor.hh_end * 60) + mtlc_sensor.mm_end)) && check_time_current != 0)
            {
                is_read_again_threshold_ambient_light = true;
                printf(" time hh start ss: %d\r\n", mtlc_sensor.hh_start);
                printf(" time mm start ss: %d\r\n", mtlc_sensor.mm_start);

                if (sys_sensor->value_sensor_present_1 != -1 && sys_sensor->value_sensor_present_2 != -1)
                {
                    total_value_present_sensor = (sys_sensor->value_sensor_present_1 + sys_sensor->value_sensor_present_2) / 2;
                    printf("total value present : %d\r\n", total_value_present_sensor);
                    printf("set value control   : %d\r\n", mtlc_sensor_struct->threshold_ambient_light);
                    printf("set value threshold high   : %f\r\n", mtlc_sensor_struct->threshold_ambient_light + (total_value_present_sensor * 0.06));
                    if (is_check_first_on_power)
                    {
                        if (total_value_present_sensor < mtlc_sensor_struct->threshold_ambient_light)
                        {
                            if (currentMillis_CPU_1800s - previousMillis_CPU_1800s > interval_CPU_1800s)
                            {
                                printf("bat tu =====================================================>\r\n");
                                previousMillis_CPU_1800s = currentMillis_CPU_1800s;
                                previousMillis_CPU_2000s = currentMillis_CPU_2000s;
                                mtlc_force_cabinet->check_time_current = check_time_current;
                                mtlc_force_cabinet->force_cabinet = 1;
                                is_enable_time_delay_level_1 = true;
                                is_check_second_after_power = true;
                                is_check_first_on_power = false;
                            }
                        }
                        else if (total_value_present_sensor > (mtlc_sensor_struct->threshold_ambient_light))
                        { //+ (total_value_present_sensor * 0.06)
                            if (currentMillis_CPU_1800s - previousMillis_CPU_1800s > interval_CPU_1800s)
                            {
                                printf("tat tu =====================================================>\r\n");
                                previousMillis_CPU_1800s = currentMillis_CPU_1800s;
                                previousMillis_CPU_2000s = currentMillis_CPU_2000s;
                                mtlc_force_cabinet->check_time_current = check_time_current;
                                mtlc_force_cabinet->force_cabinet = 0;

                                is_enable_time_delay_level_1 = true;
                            }
                        }
                    }
                    else if (is_check_second_after_power == true)
                    {
                        if (currentMillis_CPU_2000s - previousMillis_CPU_2000s > interval_CPU_2000s)
                        {
                            printf("value sensor set control new\r\n");
                            previousMillis_CPU_2000s = currentMillis_CPU_2000s;
                            // mtlc_sensor_struct->threshold_ambient_light = total_value_present_sensor - (total_value_present_sensor * 0.03);
                            is_check_second_after_power = false;
                            is_check_first_on_power = true;
                        }
                    }
                    char tmp_data[800] = {0};
                    int tmp_data_len = sprintf(tmp_data, TEMPLATE_SENSOR_MONITOR, CMD_CPU_TO_QT_SENSOR_MONITORING, sys_sensor->value_sensor_present_1, sys_sensor->value_sensor_present_2, 0, 0, sys_sensor->value_temp_sensor, sys_sensor->value_hum_sensor);
                    printf("Monitor sensor: %s\r\n", tmp_data);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, tmp_data_len, tmp_data);
                    memset(tmp_data, 0x00, sizeof(tmp_data));
                }
                if (is_enable_time_delay_level_1)
                { // cưỡng chế bật tủ chạy theo cảm biến
                    printf("time delay done =====================================================>\r\n");
                    mtlc_force_cabinet->hh_start = mtlc_sensor.hh_start;
                    mtlc_force_cabinet->mm_start = mtlc_sensor.mm_start;
                    mtlc_force_cabinet->hh_end = mtlc_sensor.hh_end;
                    mtlc_force_cabinet->mm_end = mtlc_sensor.mm_end;

                    send_struct(CMD_CPU_TO_MASTER_FORCE_ON_CABINET, (uint8_t *)mtlc_force_cabinet, sizeof(type_mtlc_force_on_t));
                    delay(100);
                    send_struct(CMD_CPU_TO_MASTER_FORCE_ON_CABINET, (uint8_t *)mtlc_force_cabinet, sizeof(type_mtlc_force_on_t));
                    delay(200);
                    // send_struct(CMD_CPU_TO_MASTER_FORCE_ON_CABINET, (uint8_t *)mtlc_force_cabinet, sizeof(type_mtlc_force_on_t));
                    is_enable_time_delay_level_1 = false;
                }
            }
            else
            {
                printf("Cam bien anh sang - dang duoc tat khong su dung\r\n");
                char tmp_data[800] = {0};
                int tmp_data_len = sprintf(tmp_data, TEMPLATE_SENSOR_MONITOR, CMD_CPU_TO_QT_SENSOR_MONITORING, sys_sensor->value_sensor_present_1, sys_sensor->value_sensor_present_2, 0, 0, sys_sensor->value_temp_sensor, sys_sensor->value_hum_sensor);
                printf("Monitor sensor: %s\r\n", tmp_data);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, tmp_data_len, tmp_data);
                memset(tmp_data, 0x00, sizeof(tmp_data));
                if (is_read_again_threshold_ambient_light)
                {
                    is_read_again_threshold_ambient_light = false;
                    mtlc_sensor = Read_struct_sensor_value_toFile(sensor_value_file, "/home/pi/sensor.txt", mtlc_sensor);
                }
            }
        }
    }
}
#pragma endregion
#pragma region LUONG XU LY CHE DO HOAT DONG
// Thread main process
void *main_process(void *threadArgs)
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    while (1)
    {
        delay(1);
        if (is_flag_opened_websocket)
        {
            if (is_app_connected)
            {
                continue;
            }
            else
            {
                if (digitalRead(auto_Mode) == 0)
                {
                    delay(200);
                    if (digitalRead(auto_Mode) == 0)
                    {
                        power_mode_status = 1;
                        if (status_power != power_mode_status)
                        {
                            status_power = power_mode_status;
                            send_byte(CMD_CPU_TO_MASTER_MODE_ACTIVE, 1);
                        }
                        // printf("mode is %d, auto Mode\r\n", mode_active);
                        mode_active = AUTO_MODE;
                        // Xu ly mode_active auto
                        time_t now;
                        struct tm *time_info;
                        int c_start_time = time_on_off.hh_start[0] * 60 + time_on_off.mm_start[0];
                        int c_end_time = time_on_off.hh_end[0] * 60 + time_on_off.mm_end[0];
                        int c_start_time1 = time_on_off.hh_start[1] * 60 + time_on_off.mm_start[1];
                        int c_end_time1 = time_on_off.hh_end[1] * 60 + time_on_off.mm_end[1];
                        getCurrent_Time(now, time_info);

                        int c_check_time = current_time.hh_current * 60 + current_time.mm_current;
                        int c_check_dim1_time = dim_schedule_active.hh_start_dim1 * 60 + dim_schedule_active.mm_start_dim1;
                        int c_check_dim2_time = dim_schedule_active.hh_start_dim2 * 60 + dim_schedule_active.mm_start_dim2;
                        int c_check_dim3_time = dim_schedule_active.hh_start_dim3 * 60 + dim_schedule_active.mm_start_dim3;
                        int c_check_dim4_time = dim_schedule_active.hh_start_dim4 * 60 + dim_schedule_active.mm_start_dim4;
                        int c_check_dim5_time = dim_schedule_active.hh_start_dim5 * 60 + dim_schedule_active.mm_start_dim5;
                        int c_check_dim6_time = dim_schedule_active.hh_start_dim6 * 60 + dim_schedule_active.mm_start_dim6;
                        int c_check_dim7_time = dim_schedule_active.hh_start_dim7 * 60 + dim_schedule_active.mm_start_dim7;
                        int c_check_dim8_time = dim_schedule_active.hh_start_dim8 * 60 + dim_schedule_active.mm_start_dim8;
                        int c_check_dim9_time = dim_schedule_active.hh_start_dim9 * 60 + dim_schedule_active.mm_start_dim9;

                        int c_check_dim1_group_1_time = dim_schedule_group_1.hh_start_dim1 * 60 + dim_schedule_group_1.mm_start_dim1;
                        int c_check_dim2_group_1_time = dim_schedule_group_1.hh_start_dim2 * 60 + dim_schedule_group_1.mm_start_dim2;
                        int c_check_dim3_group_1_time = dim_schedule_group_1.hh_start_dim3 * 60 + dim_schedule_group_1.mm_start_dim3;
                        int c_check_dim4_group_1_time = dim_schedule_group_1.hh_start_dim4 * 60 + dim_schedule_group_1.mm_start_dim4;
                        int c_check_dim5_group_1_time = dim_schedule_group_1.hh_start_dim5 * 60 + dim_schedule_group_1.mm_start_dim5;
                        int c_check_dim6_group_1_time = dim_schedule_group_1.hh_start_dim6 * 60 + dim_schedule_group_1.mm_start_dim6;
                        int c_check_dim7_group_1_time = dim_schedule_group_1.hh_start_dim7 * 60 + dim_schedule_group_1.mm_start_dim7;
                        int c_check_dim8_group_1_time = dim_schedule_group_1.hh_start_dim8 * 60 + dim_schedule_group_1.mm_start_dim8;
                        int c_check_dim9_group_1_time = dim_schedule_group_1.hh_start_dim9 * 60 + dim_schedule_group_1.mm_start_dim9;

                        int c_check_dim1_group_2_time = dim_schedule_group_2.hh_start_dim1 * 60 + dim_schedule_group_2.mm_start_dim1;
                        int c_check_dim2_group_2_time = dim_schedule_group_2.hh_start_dim2 * 60 + dim_schedule_group_2.mm_start_dim2;
                        int c_check_dim3_group_2_time = dim_schedule_group_2.hh_start_dim3 * 60 + dim_schedule_group_2.mm_start_dim3;
                        int c_check_dim4_group_2_time = dim_schedule_group_2.hh_start_dim4 * 60 + dim_schedule_group_2.mm_start_dim4;
                        int c_check_dim5_group_2_time = dim_schedule_group_2.hh_start_dim5 * 60 + dim_schedule_group_2.mm_start_dim5;
                        int c_check_dim6_group_2_time = dim_schedule_group_2.hh_start_dim6 * 60 + dim_schedule_group_2.mm_start_dim6;
                        int c_check_dim7_group_2_time = dim_schedule_group_2.hh_start_dim7 * 60 + dim_schedule_group_2.mm_start_dim7;
                        int c_check_dim8_group_2_time = dim_schedule_group_2.hh_start_dim8 * 60 + dim_schedule_group_2.mm_start_dim8;
                        int c_check_dim9_group_2_time = dim_schedule_group_2.hh_start_dim9 * 60 + dim_schedule_group_2.mm_start_dim9;

                        int c_check_dim1_group_3_time = dim_schedule_group_3.hh_start_dim1 * 60 + dim_schedule_group_3.mm_start_dim1;
                        int c_check_dim2_group_3_time = dim_schedule_group_3.hh_start_dim2 * 60 + dim_schedule_group_3.mm_start_dim2;
                        int c_check_dim3_group_3_time = dim_schedule_group_3.hh_start_dim3 * 60 + dim_schedule_group_3.mm_start_dim3;
                        int c_check_dim4_group_3_time = dim_schedule_group_3.hh_start_dim4 * 60 + dim_schedule_group_3.mm_start_dim4;
                        int c_check_dim5_group_3_time = dim_schedule_group_3.hh_start_dim5 * 60 + dim_schedule_group_3.mm_start_dim5;
                        int c_check_dim6_group_3_time = dim_schedule_group_3.hh_start_dim6 * 60 + dim_schedule_group_3.mm_start_dim6;
                        int c_check_dim7_group_3_time = dim_schedule_group_3.hh_start_dim7 * 60 + dim_schedule_group_3.mm_start_dim7;
                        int c_check_dim8_group_3_time = dim_schedule_group_3.hh_start_dim8 * 60 + dim_schedule_group_3.mm_start_dim8;
                        int c_check_dim9_group_3_time = dim_schedule_group_3.hh_start_dim9 * 60 + dim_schedule_group_3.mm_start_dim9;

                        if (time_on_off.size_array_on_off_schedule == 1)
                        {
                            if (c_start_time <= c_end_time)
                            {
                                if (c_check_time >= c_start_time && c_check_time <= c_end_time)
                                {
                                    digitalWrite(relay_1, 0);
                                    digitalWrite(relay_2, 0);
                                    if (is_status_power == false)
                                    {
                                        is_status_power = true;
                                        power_fb = 1;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 1);
                                        delay(200);
                                    }
                                }
                                else
                                {
                                    digitalWrite(relay_1, 1);
                                    digitalWrite(relay_2, 1);
                                    if (is_status_power == true)
                                    {
                                        is_status_power = false;
                                        power_fb = 0;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                        delay(200);
                                    }
                                }
                            }
                            else
                            {
                                if (c_check_time >= c_start_time || c_check_time <= c_end_time)
                                {
                                    digitalWrite(relay_1, 0);
                                    digitalWrite(relay_2, 0);
                                    if (is_status_power == false)
                                    {
                                        is_status_power = true;
                                        power_fb = 1;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 1);
                                        delay(200);
                                    }
                                }
                                else
                                {
                                    digitalWrite(relay_1, 1);
                                    digitalWrite(relay_2, 1);
                                    if (is_status_power == true)
                                    {
                                        is_status_power = false;
                                        power_fb = 0;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                        delay(200);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if ((c_start_time <= c_end_time) && (c_start_time1 <= c_end_time1))
                            {
                                if ((c_check_time >= c_start_time && c_check_time <= c_end_time) ||
                                    (c_check_time >= c_start_time1 && c_check_time <= c_end_time1))
                                {
                                    digitalWrite(relay_1, 0);
                                    digitalWrite(relay_2, 0);
                                    if (is_status_power == false)
                                    {
                                        is_status_power = true;
                                        power_fb = 1;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 1);
                                        delay(200);
                                    }
                                }
                                else
                                {
                                    digitalWrite(relay_1, 1);
                                    digitalWrite(relay_2, 1);
                                    if (is_status_power == true)
                                    {
                                        is_status_power = false;
                                        power_fb = 0;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                        delay(200);
                                    }
                                }
                            }
                            else if ((c_start_time > c_end_time) && (c_start_time1 <= c_end_time1))
                            {
                                // printf("Case 2\r\n");
                                if ((((c_check_time >= c_start_time || c_check_time <= c_end_time)) ||
                                     ((c_check_time >= c_start_time1) && (c_check_time <= c_end_time1))) &&
                                    (c_start_time > c_start_time1))
                                {
                                    digitalWrite(relay_1, 0);
                                    digitalWrite(relay_2, 0);
                                    if (is_status_power == false)
                                    {
                                        is_status_power = true;
                                        power_fb = 1;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 1);
                                        delay(200);
                                    }
                                }
                                else
                                {
                                    // printf("Case 2.2\r\n");
                                    digitalWrite(relay_1, 1);
                                    digitalWrite(relay_2, 1);
                                    if (is_status_power == true)
                                    {
                                        is_status_power = false;
                                        power_fb = 0;
                                        flag_onoffstatus = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                        delay(200);
                                    }
                                }
                            }
                            else if ((c_start_time1 > c_end_time1) && (c_start_time <= c_end_time))
                            {
                                if ((((c_check_time >= c_start_time1) || (c_check_time <= c_end_time1)) ||
                                     ((c_check_time >= c_start_time) && (c_check_time <= c_end_time))) &&
                                    (c_start_time1 > c_start_time))
                                {
                                    digitalWrite(relay_1, 0);
                                    digitalWrite(relay_2, 0);
                                    if (is_status_power == false)
                                    {
                                        is_status_power = true;
                                        power_fb = 1;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 1);
                                        delay(200);
                                    }
                                }
                                else
                                {
                                    digitalWrite(relay_1, 1);
                                    digitalWrite(relay_2, 1);
                                    if (is_status_power == true)
                                    {
                                        is_status_power = false;
                                        power_fb = 0;
                                        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                        delay(200);
                                    }
                                }
                            }
                            else
                            {
                                digitalWrite(relay_1, 1);
                                digitalWrite(relay_2, 1);
                                if (is_status_power == true)
                                {
                                    is_status_power = false;
                                    power_fb = 0;
                                    // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, 0);
                                    delay(200);
                                }
                            }
                        }

                        if (a != current_time.mm_current)
                        {
                            if (c_check_time == c_check_dim1_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim1;
                                add.group = dim_schedule_group_1.group;
                                // printf("id is: %d\r\n", add.id);
                                // printf("port is: %d\r\n", add.port);
                                // printf("group is: %d\r\n", add.group);
                                // printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");

                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    // for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                    // {
                                    //     if (fb_dim_port1.group[i] == add.group)
                                    //         fb_dim_port1.dim_fb[i] = add.dim;
                                    // }
                                    // for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                    // {
                                    //     if (fb_dim_port2.group[i] == add.group)
                                    //         fb_dim_port2.dim_fb[i] = add.dim;
                                    // }
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear: %d\r\n", queue.rear);
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim2_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim2;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");

                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim3_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim3;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim4_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim4;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim5_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim5;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));

                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim6_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim6;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                printf("gia tri setting type: %d\r\n", setting.type);
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));

                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim7_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim7;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));

                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim8_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim8;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim9_group_1_time)
                            {
                                add.dim = dim_schedule_group_1.dim9;
                                add.group = dim_schedule_group_1.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                if (setting.type == MODE_OUTPUT_CPU_RS485)
                                {
                                    for (int i = 0; i <= queue_lamps_port1.rear; i++)
                                    {
                                        if (queue_lamps_port1.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port1: %d\r\n", queue_lamps_port1.rear);
                                            add.id = queue_lamps_port1.inp_arr[i];
                                            add.group = 100;
                                            add.port = 1;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                    for (int i = 0; i <= queue_lamps_port2.rear; i++)
                                    {
                                        if (queue_lamps_port2.inp_arr[i] != 0)
                                        {
                                            printf("gia tri rear lamp port2: %d\r\n", queue_lamps_port2.rear);
                                            add.id = queue_lamps_port2.inp_arr[i];
                                            add.group = 100;
                                            add.port = 2;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            // send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                                            delay(500);
                                        }
                                    }
                                }
                                else if (setting.type == MODE_OUTPUT_CPU_RF)
                                {
                                    for (int i = 0; i <= queue.rear; i++)
                                    {
                                        if (queue.inp_arr[i] != 0)
                                        {
                                            add.id = queue.inp_arr[i];
                                            add.group = 100;
                                            printf("id is: %d\r\n", add.id);
                                            printf("port is: %d\r\n", add.port);
                                            printf("group is: %d\r\n", add.group);
                                            printf("dim is: %d\r\n", add.dim);
                                            send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));

                                            delay(500);
                                        }
                                    }
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                a = current_time.mm_current;
                            }
                        }

                        if (c != current_time.mm_current)
                        {
                            if (c_check_time == c_check_dim1_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim1;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim2_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim2;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim3_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim3;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim4_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim4;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim5_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim5;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim6_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim6;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim7_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim7;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim8_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim8;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim9_group_2_time)
                            {
                                add.dim = dim_schedule_group_2.dim9;
                                add.group = dim_schedule_group_2.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                c = current_time.mm_current;
                            }
                        }

                        if (d != current_time.mm_current)
                        {
                            if (c_check_time == c_check_dim1_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim1;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim2_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim2;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim3_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim3;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim4_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim4;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim5_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim5;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim6_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim6;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim7_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim7;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim8_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim8;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim9_group_3_time)
                            {
                                add.dim = dim_schedule_group_3.dim9;
                                add.group = dim_schedule_group_3.group;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim Group 1 ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == add.group)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == add.group)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                                d = current_time.mm_current;
                            }
                        }

                        if (b != current_time.mm_current)
                        {
                            if (c_check_time == c_check_dim1_time)
                            {
                                add.dim = dim_schedule_active.dim1;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim2_time)
                            {
                                add.dim = dim_schedule_active.dim2;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim3_time)
                            {
                                add.dim = dim_schedule_active.dim3;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim4_time)
                            {
                                add.dim = dim_schedule_active.dim4;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim5_time)
                            {
                                add.dim = dim_schedule_active.dim5;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim6_time)
                            {
                                add.dim = dim_schedule_active.dim6;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim7_time)
                            {
                                add.dim = dim_schedule_active.dim7;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim8_time)
                            {
                                add.dim = dim_schedule_active.dim8;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                            else if (c_check_time == c_check_dim9_time)
                            {
                                add.dim = dim_schedule_active.dim9;
                                add.group = 200;
                                printf("id is: %d\r\n", add.id);
                                printf("port is: %d\r\n", add.port);
                                printf("group is: %d\r\n", add.group);
                                printf("dim is: %d\r\n", add.dim);
                                printf("\nDim ALL ok\r\n");
                                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                                {
                                    if (fb_dim_port1.group[i] == 200)
                                        fb_dim_port1.dim_fb[i] = add.dim;
                                }
                                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                                {
                                    if (fb_dim_port2.group[i] == 200)
                                        fb_dim_port2.dim_fb[i] = add.dim;
                                }
                                // send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                                b = current_time.mm_current;
                            }
                        }

                        delay(1000);
                    }
                }
                else if (digitalRead(manual_Mode) == 0)
                {
                    delay(200);
                    if (digitalRead(manual_Mode) == 0)
                    {
                        power_mode_status = 2;
                        if (status_power != power_mode_status)
                        {
                            status_power = power_mode_status;
                            send_byte(CMD_CPU_TO_MASTER_MODE_ACTIVE, 2);
                        }
                        mode_active = MANUAL_MODE;
                        printf("mode is %d, on Mode\r\n", mode_active);
                        flag_onoffstatus = 1;
                        if (is_status_power == false)
                        {
                            is_status_power = true;
                            power_fb = 1;
                        }
                        delay(1000);
                    }
                }
                else
                {
                    power_mode_status = 0;
                    if (status_power != power_mode_status)
                    {
                        status_power = power_mode_status;
                        send_byte(CMD_CPU_TO_MASTER_MODE_ACTIVE, 0);
                    }
                    delay(200);
                    mode_active = NONE;
                    flag_onoffstatus = 1;
                    delay(1000);
                    printf("mode is %d, off Mode\r\n", mode_active);
                    if (is_status_power == true)
                    {
                        is_status_power = false;
                        power_fb = 0;
                    }
                }

                if (cpu_update_rtc.param == 2)
                {
                    type_date_time_t type_date_time_update_rtc;
                    memset((uint8_t *)&type_date_time_update_rtc, 0, sizeof(type_date_time_t));
                    system("cat /sys/class/rtc/rtc0/time > timeset.txt");
                    system("cat /sys/class/rtc/rtc0/date > dateset.txt");

                    f_time_update = fopen("timeset.txt", "r");
                    f_date_update = fopen("dateset.txt", "r");

                    if (f_time_update == NULL)
                    {
                        printf("Error openfile\r\n");
                        perror("Error opening file");
                    }
                    if (fgets(time_update, 18, f_time_update) != NULL)
                    {
                        /* writing content to stdout */
                    }
                    fclose(f_time_update);

                    if (fgets(date_update, 18, f_date_update) != NULL)
                    {
                        /* writing content to stdout */
                    }
                    fclose(f_date_update);
                    printf("Time set read from file timeset.txt: %s\r\n", time_update);
                    printf("Date set read from file dateset.txt: %s\r\n", date_update);

                    char *hhStr = NULL;
                    char *mmStr = NULL;
                    char *ssStr = NULL;

                    char *yyStr = NULL;
                    char *moStr = NULL;
                    char *ddStr = NULL;

                    char *tmpinpStr = NULL;
                    asprintf(&tmpinpStr, "%s", time_update);
                    const char s[2] = ":";
                    hhStr = strtok(tmpinpStr, s);
                    mmStr = strtok(NULL, s);
                    ssStr = strtok(NULL, s);
                    tmpinpStr = NULL;
                    asprintf(&tmpinpStr, "%s", date_update);
                    const char d[2] = "-";
                    yyStr = strtok(tmpinpStr, d);
                    moStr = strtok(NULL, d);
                    ddStr = strtok(NULL, d);

                    type_date_time_update_rtc.year = atoi(yyStr);
                    type_date_time_update_rtc.month = atoi(moStr);
                    type_date_time_update_rtc.date = atoi(ddStr);
                    type_date_time_update_rtc.hour = atoi(hhStr) + 7;
                    type_date_time_update_rtc.minute = atoi(mmStr);
                    type_date_time_update_rtc.seconds = atoi(ssStr);
                    send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&type_date_time_update_rtc, sizeof(type_date_time_t));

                    // printf("got value time hh: %s\r\n", hhStr);
                    // printf("got value time mm: %s\r\n", mmStr);
                    // printf("got value time ss: %s\r\n", ssStr);

                    // printf("got value time yy: %s\r\n", yyStr);
                    // printf("got value time mo: %s\r\n", moStr);
                    // printf("got value time dd: %s\r\n", ddStr);

                    cpu_update_rtc.param = 0;
                }
            }
        }
    }
}
#pragma endregion

#pragma region LUONG GIAO TIEP MAN HINH
static void uwsc_onopen(struct uwsc_client *cl)
{
    // static struct ev_io stdin_watcher;

    uwsc_log_info("onopen\n");

    // stdin_watcher.data = cl;

    // ev_io_init(&stdin_watcher, stdin_read_cb, STDIN_FILENO, EV_READ);
    // ev_io_start(cl->loop, &stdin_watcher);
    is_flag_opened_websocket = true;
    cl->send_ex(cl, UWSC_OP_TEXT, 1, 3, "CPU");
}

static void uwsc_onerror(struct uwsc_client *cl, int err, const char *msg)
{
    uwsc_log_err("onerror:%d: %s\n", err, msg);
    ev_break(cl->loop, EVBREAK_ALL);
    is_flag_opened_websocket = false;
    exit(1);
}

static void uwsc_onclose(struct uwsc_client *cl, int code, const char *reason)
{
    uwsc_log_err("onclose:%d: %s\n", code, reason);
    ev_break(cl->loop, EVBREAK_ALL);
    is_flag_opened_websocket = false;
    exit(1);
}

static void signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
    if (w->signum == SIGINT)
    {
        ev_break(loop, EVBREAK_ALL);
        uwsc_log_info("Normal quit\n");
        is_flag_opened_websocket = false;
        exit(1);
    }
}

static void uwsc_onmessage(struct uwsc_client *cl, void *data, size_t len, bool binary)
{
    if (binary)
    {
        size_t i;
        uint8_t *p = data;

        for (i = 0; i < len; i++)
        {
            printf("%02hhX ", p[i]);
            if (i % 16 == 0 && i > 0)
                puts("");
        }
        puts("");
    }
    else
    {
        if (!strncmp(data, "qtApp-", 6))
        {
            printf("reach here\r\n");
            // qtApp-{"CMD": 5, "PWR": "OFF"}
            char *temp = strtok(data, "-");
            temp = strtok(NULL, "\n");
            printf("[%.*s]\n", strlen(temp), (char *)temp);
            struct json_object *new_obj;
            new_obj = json_tokener_parse(temp);
            printf("Parsed is: %s\n", temp);
            printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
            if (!new_obj)
            {
                printf("Error Null");
                return;
            }
            struct json_object *o = NULL;
            if (!json_object_object_get_ex(new_obj, "CMD", &o))
            {
                printf("Field %s does not exist\n", "CMD");
                return;
            }

            printf("reach here\r\n");
            switch (json_object_get_int(o))
            {
            case CMD_QT_SET_RTC_MANUAL:
            {
                type_RTC_set_CPU_CM4_t type_RTC_set_CPU_CM4;
                type_date_time_t type_date_time_update_rtc;

                if (!json_object_object_get_ex(new_obj, "years", &o))
                {
                    return;
                }

                type_RTC_set_CPU_CM4.years = json_object_get_int(o); // Lay du lieu cai nam
                type_date_time_update_rtc.year = type_RTC_set_CPU_CM4.years;
                if (!json_object_object_get_ex(new_obj, "month", &o))
                {
                    return;
                }
                type_RTC_set_CPU_CM4.month = json_object_get_int(o); // Lay du lieu cai thang
                type_date_time_update_rtc.month = type_RTC_set_CPU_CM4.month;
                if (!json_object_object_get_ex(new_obj, "day", &o))
                {
                    return;
                }
                type_RTC_set_CPU_CM4.day = json_object_get_int(o); // Lay du lieu cai ngay
                type_date_time_update_rtc.date = type_RTC_set_CPU_CM4.day;
                if (!json_object_object_get_ex(new_obj, "hh", &o))
                {
                    return;
                }
                type_RTC_set_CPU_CM4.hh = json_object_get_int(o); // Lay du lieu cai gio
                type_date_time_update_rtc.hour = type_RTC_set_CPU_CM4.hh;
                if (!json_object_object_get_ex(new_obj, "mm", &o))
                {
                    return;
                }
                type_RTC_set_CPU_CM4.mm = json_object_get_int(o); // Lay du lieu cai gio
                type_date_time_update_rtc.minute = type_RTC_set_CPU_CM4.mm;
                type_RTC_set_CPU_CM4.ss = 0;
                type_date_time_update_rtc.seconds = 0;

                printf("Time setting: %d/%d/%d %d:%d:%d\r\n", type_RTC_set_CPU_CM4.years,
                       type_RTC_set_CPU_CM4.month,
                       type_RTC_set_CPU_CM4.day,
                       type_RTC_set_CPU_CM4.hh,
                       type_RTC_set_CPU_CM4.mm, 0);

                send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&type_date_time_update_rtc, sizeof(type_date_time_t));

                char timeSeting_temp[200];
                // sudo date --set="2012/12/12 12:12:12"
                // sudo hwclock --set --date "2012/12/12 12:12:12"
                int len = sprintf(timeSeting_temp, "sudo hwclock --set --date \"%d/%d/%d %d:%d:%d +07\"", type_RTC_set_CPU_CM4.years,
                                  type_RTC_set_CPU_CM4.month,
                                  type_RTC_set_CPU_CM4.day,
                                  type_RTC_set_CPU_CM4.hh,
                                  type_RTC_set_CPU_CM4.mm, 0);
                printf("%s\r\n", timeSeting_temp);
                system(timeSeting_temp);
                delay(100);
                memset(timeSeting_temp, 0, sizeof(timeSeting_temp));
                len = sprintf(timeSeting_temp, "sudo date --set=\"%d/%d/%d %d:%d:%d\"", type_RTC_set_CPU_CM4.years,
                              type_RTC_set_CPU_CM4.month,
                              type_RTC_set_CPU_CM4.day,
                              type_RTC_set_CPU_CM4.hh,
                              type_RTC_set_CPU_CM4.mm, 0);
                printf("%s\r\n", timeSeting_temp);
                system(timeSeting_temp);

                delay(100);
                char data[1000];
                len = sprintf(data, "CPU-{\"CMD\": %d}", CMD_QT_SET_RTC_MANUAL);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                memset(data, 0, sizeof(data));
                printf("send status set new time success\r\n");
                break;
            }
            case CMD_QT_CALL_SCHEDULE_LOAD_EDIT:
            {
                // time_on_off = Read_struct_time_on_off_toFile(time_on_off_file, "/home/pi/time_on_off.txt", time_on_off);
                //  int data_len = sprintf(data, TEMPLATE_TIME_ACTIVE, CMD_CPU_TO_QT_TIME_ACTIVE,
                //  time_active.hh_start1, time_active.mm_start1, time_active.hh_end1, time_active.mm_end1,
                //  time_active.hh_start2, time_active.mm_start2, time_active.hh_end2, time_active.mm_end2,
                //  time_active.hh_start3, time_active.mm_start3, time_active.hh_end3, time_active.mm_end3);
                char message1[1280] = {0};
                char message2[1280] = {0};
                if (time_on_off.size_array_on_off_schedule == 1)
                {
                    snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                             time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
                    // data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}",CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                }
                else
                {
                    for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
                    {
                        if (i == 0)
                        {
                            snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        else
                        {
                            snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        strcat(message1, message2);
                        printf("message1: %s\r\n", message1);
                    }
                }
                int data_len = sprintf(data, "CPU-{\"CMD\": %d,\"time\":[%s]}", CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                break;
            }
            case CMD_CPU_TO_QT_LOAD_DIM_VALUE:
            {
                dim_active = Read_struct_dim_active_toFile(dim_active_file, "/home/pi/dim_active.txt", dim_active);
                int data_len = sprintf(data, TEMPLATE_DIM_ACTIVE, CMD_CPU_TO_QT_DIM_VALUE,
                                       dim_active.dim);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                break;
            }
            case CMD_CPU_TO_QT_DIM_ALL_SCHEDULE:
            {
                dim_schedule_active = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_active.txt", dim_schedule_active);

                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, 200,
                                       dim_schedule_active.dim1,
                                       dim_schedule_active.dim2,
                                       dim_schedule_active.dim3,
                                       dim_schedule_active.dim4,
                                       dim_schedule_active.dim5,
                                       dim_schedule_active.dim6,
                                       dim_schedule_active.dim7,
                                       dim_schedule_active.dim8,
                                       dim_schedule_active.dim9,
                                       dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1,
                                       dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2,
                                       dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3,
                                       dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4,
                                       dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5,
                                       dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6,
                                       dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7,
                                       dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8,
                                       dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                printf("Schedule dim all da dc xuat len man hinh QT\r\n");
                break;
            }

            case CMD_QT_TO_CPU_MODE_AUTO_RTC: // mode
            {
                // printf("Hoi auto mode RTC\r\n");
                if (get_State_Cmd_Terminal("timedatectl", "NTP service: active"))
                {
                    char data[1000];
                    // printf("Founded keyword\r\n");
                    int len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_TO_CPU_MODE_AUTO_RTC, 1);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));
                }
                else
                {
                    char data[1000];
                    // printf("Not found keyword\r\n");
                    int len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_TO_CPU_MODE_AUTO_RTC, 0);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));
                }
                break;
            }

            case CMD_QT_CONTROL_UPDATE_NTP_RTC:
            {
                type_date_time_t type_date_time_update_rtc;
                if (!json_object_object_get_ex(new_obj, "NTP", &o))
                {
                    printf("Field %s does not exist\n", "PWR");
                    return;
                }
                if (json_object_get_int(o))
                {
                    printf("NTP_update: active uwsc on message\r\n");
                    system("sudo timedatectl set-ntp true");
                    char data[1000];
                    int len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_CONTROL_UPDATE_NTP_RTC, 1);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));

                    // printf("Founded keyword\r\n");
                    len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_TO_CPU_MODE_AUTO_RTC, 1);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));
                    delay(500);
                    struct timeval curTime;
                    struct tm *my_date_time;

                    gettimeofday(&curTime, NULL);
                    my_date_time = localtime(&curTime.tv_sec);
                    printf("%d gio %d phut %d giay\r\n", my_date_time->tm_hour, my_date_time->tm_min, my_date_time->tm_sec);
                    printf("ngay %d thang %d nam %d\r\n", my_date_time->tm_mday, my_date_time->tm_mon + 1, my_date_time->tm_year - 100);

                    // RTC_set_master.years = (my_date_time->tm_year - 100) + 2000;
                    // RTC_set_master.month = my_date_time->tm_mon + 1;
                    // RTC_set_master.day = my_date_time->tm_mday;
                    // RTC_set_master.hh = my_date_time->tm_hour;
                    // RTC_set_master.mm = my_date_time->tm_min;
                    // RTC_set_master.ss = my_date_time->tm_sec;
                    // send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&RTC_set_master, sizeof(type_RTC_set_CPU_CM4_t));

                    
                    type_date_time_update_rtc.year = (my_date_time->tm_year - 100) + 2000;
                    type_date_time_update_rtc.month = my_date_time->tm_mon + 1;
                    type_date_time_update_rtc.date = my_date_time->tm_mday;
                    type_date_time_update_rtc.hour = my_date_time->tm_hour;
                    type_date_time_update_rtc.minute = my_date_time->tm_min;
                    type_date_time_update_rtc.seconds = my_date_time->tm_sec;
                    send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&type_date_time_update_rtc, sizeof(type_date_time_t));
                }
                else
                {
                    printf("NTP update: inactive\r\n");
                    char data[1000];
                    system("sudo timedatectl set-ntp false");
                    int len = sprintf(data, "CPU-{\"CMD\": %d,\"Value\": %d}", CMD_QT_CONTROL_UPDATE_NTP_RTC, 0);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));

                    // printf("Not found keyword\r\n");
                    len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_TO_CPU_MODE_AUTO_RTC, 0);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
                    memset(data, 0, sizeof(data));
                }
                break;
            }

            case CMD_QT_TO_CPU_GET_MODE: // Gui 1 lan khi khoi dong man hinh lay che do
            {
                char data_mode_active[1000] = {0};
                // Gui trang thai che do
                int data_mode_active_len = sprintf(data_mode_active, TEMPLATE_MODE,
                                                   CMD_CPU_TO_QT_MODE, mode_active);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
                memset(data_mode_active, 0, 1000);

                // delay(10);
                //  Gui trang thai nguon khi connect QT
                data_mode_active_len = sprintf(data_mode_active, TEMPLATE_STATUS_PWR,
                                               CMD_CPU_TO_QT_STATUS_PWR, is_status_power);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
                memset(data_mode_active, 0, 1000);
                // Gui time  bat tat
                // data_mode_active_len = sprintf(data_mode_active, TEMPLATE_TIME_ACTIVE, CMD_CPU_TO_QT_TIME_ACTIVE,
                // time_active.hh_start1, time_active.mm_start1, time_active.hh_end1, time_active.mm_end1,
                // time_active.hh_start2, time_active.mm_start2, time_active.hh_end2, time_active.mm_end2,
                // time_active.hh_start3, time_active.mm_start3, time_active.hh_end3, time_active.mm_end3);
                char message1[1280] = {0};
                char message2[1280] = {0};
                char message3[2000] = {0};
                if (time_on_off.size_array_on_off_schedule == 1)
                {
                    snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                             time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
                    // data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}",CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                }
                else
                {
                    for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
                    {
                        if (i == 0)
                        {
                            snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        else
                        {
                            snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d,  \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        strcat(message1, message2);
                    }
                }
                data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}", CMD_CPU_TO_QT_TIME_ACTIVE, message1);

                // printf("message3: %s\r\n", data_mode_active);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, message3);
                memset(message3, 0, 1000);
                // delay(10);
                //  Gui connect
                data_mode_active_len = sprintf(data_mode_active, "CPU-{\"CMD\": %d,\"connect\":%d}", CMD_CPU_TO_QT_CONNECTION, checkconnect);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
                memset(data_mode_active, 0, 1000);
                // Gui dim schedule
                data_mode_active_len = sprintf(data_mode_active, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, 200,
                                               dim_schedule_active.dim1,
                                               dim_schedule_active.dim2,
                                               dim_schedule_active.dim3,
                                               dim_schedule_active.dim4,
                                               dim_schedule_active.dim5,
                                               dim_schedule_active.dim6,
                                               dim_schedule_active.dim7,
                                               dim_schedule_active.dim8,
                                               dim_schedule_active.dim9,
                                               dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1,
                                               dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2,
                                               dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3,
                                               dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4,
                                               dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5,
                                               dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6,
                                               dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7,
                                               dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8,
                                               dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
                memset(data_mode_active, 0, 1000);
                delay(300);

                char data[1000] = {0};
                int data_len_sensor_on_dashboad = sprintf(data, TEMPLATE_SENSOR_ON_DASHBOARD, CMD_CPU_TO_QT_SENSOR_ON_DASHBOARD, mtlc_sensor.hh_start, mtlc_sensor.mm_start, mtlc_sensor.hh_end, mtlc_sensor.mm_end, mtlc_sensor.check_enable, mtlc_sensor.threshold_ambient_light);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len_sensor_on_dashboad, data);
                memset(data, 0, 1000);
                delay(100);

                char data_server[1000] = {0};
                int data_len = sprintf(data_server, TEMPLATE_SETTING, CMD_CPU_TO_QT_OPEN_SERVER, setting.server, setting.type);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data_server);
                memset(data_server, 0, 1000);
                break;
            }

            case CMD_QT_TO_CPU_TIME_ACTIVE:
            {
                printf("save time active to file\r\n");
                if (!json_object_object_get_ex(new_obj, "hh_start1", &o))
                {
                    printf("Field %s does not exist\n", "hh_start1");
                    return;
                }
                time_on_off.hh_start[0] = json_object_get_int(o);
                printf("\nhh_start is: %d", time_on_off.hh_start[0]);

                if (!json_object_object_get_ex(new_obj, "mm_start1", &o))
                {
                    printf("Field %s does not exist\n", "mm_start1");
                    return;
                }
                time_on_off.mm_start[0] = json_object_get_int(o);
                printf("\nmm_start is: %d", time_on_off.mm_start[0]);

                if (!json_object_object_get_ex(new_obj, "hh_end1", &o))
                {
                    printf("Field %s does not exist\n", "hh_end1");
                    return;
                }
                time_on_off.hh_end[0] = json_object_get_int(o);
                printf("\nhh_end is: %d", time_on_off.hh_end[0]);

                if (!json_object_object_get_ex(new_obj, "mm_end1", &o))
                {
                    printf("Field %s does not exist\n", "mm_end1");
                    return;
                }
                time_on_off.mm_end[0] = json_object_get_int(o);
                printf("\nmm_end is: %d", time_on_off.mm_end[0]);

                time_on_off.size_array_on_off_schedule = 1;

                if (!json_object_object_get_ex(new_obj, "hh_start2", &o))
                {
                    printf("Field %s does not exist\n", "hh_start2");
                    // return;
                }
                time_on_off.hh_start[1] = json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "mm_start2", &o))
                {
                    printf("Field %s does not exist\n", "mm_start2");
                    // return;
                }
                time_on_off.mm_start[1] = json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "hh_end2", &o))
                {
                    printf("Field %s does not exist\n", "hh_end2");
                    // return;
                }
                time_on_off.hh_end[1] = json_object_get_int(o);
                printf("\nhh_end is: %d", time_on_off.hh_end[0]);

                if (!json_object_object_get_ex(new_obj, "mm_end2", &o))
                {
                    printf("Field %s does not exist\n", "mm_end2");
                    // return;
                }
                time_on_off.mm_end[1] = json_object_get_int(o);

                if ((time_on_off.hh_start[1] == 0) && (time_on_off.mm_start[1] == 0) && (time_on_off.hh_end[1] == 0) && (time_on_off.mm_end[1] == 0))
                    time_on_off.size_array_on_off_schedule = 1;
                else
                    time_on_off.size_array_on_off_schedule = 2;
                printf("\ntime_on_off.size_array_on_off_schedule: %d", time_on_off.size_array_on_off_schedule);

                // Write_struct_time_active_toFile(time_active_file, "/home/pi/time_active.txt", time_active);
                send_struct(CMD_CPU_TO_MASTER_SCHEDULE_POWER, (uint8_t *)&time_on_off, sizeof(time_on_off_t));
                Write_struct_time_on_off_toFile(time_on_off_file, "/home/pi/time_on_off.txt", time_on_off);
                char data_send_active_time[2000];
                int data_send_active_time_len = sprintf(data_send_active_time, TEMPLATE_TIME_ACTIVE, CMD_CPU_TO_QT_TIME_ACTIVE,
                                                        time_active.hh_start1, time_active.mm_start1, time_active.hh_end1, time_active.mm_end1,
                                                        time_active.hh_start2, time_active.mm_start2, time_active.hh_end2, time_active.mm_end2,
                                                        time_active.hh_start3, time_active.mm_start3, time_active.hh_end3, time_active.mm_end3);
                char message1[1280] = {0};
                char message2[1280] = {0};
                if (time_on_off.size_array_on_off_schedule == 1)
                {
                    snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                             time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
                    // data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}",CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                }
                else
                {
                    for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
                    {
                        if (i == 0)
                        {
                            snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        else
                        {
                            snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d,  \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        strcat(message1, message2);
                        printf("message1: %s\r\n", message1);
                    }
                }
                data_send_active_time_len = sprintf(data_send_active_time, "CPU-{\"CMD\": %d,\"time\":[%s]}", CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                printf("\time is: %s", data_send_active_time);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_send_active_time_len, data_send_active_time);
                printf("\nsend qt ok\r\n");
                break;
            }

            case CMD_QT_TO_CPU_DIM_ALL:
            {
                printf("save dim active to file\r\n");
                if (!json_object_object_get_ex(new_obj, "dim", &o))
                {
                    printf("Field %s does not exist\n", "dim");
                    return;
                }
                add.dim = (uint8_t)json_object_get_int(o);
                add.group = 200;
                printf("id is: %d\r\n", add.id);
                printf("port is: %d\r\n", add.port);
                printf("group is: %d\r\n", add.group);
                printf("dim is: %d\r\n", add.dim);
                printf("fb_dim_port1.lampport is: %d\r\n", fb_dim_port1.lampport);
                printf("\nDim ALL ok\r\n");
                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                {
                    if (fb_dim_port1.group[i] == 200)
                        fb_dim_port1.dim_fb[i] = add.dim;
                }
                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                {
                    if (fb_dim_port2.group[i] == 200)
                        fb_dim_port2.dim_fb[i] = add.dim;
                }
                send_struct(CMD_CPU_TO_MASTER_DIM_ALL, (uint8_t *)&add, sizeof(add_t));
                flag_dimstatus = 1;
                break;
            }

            case CMD_QT_TO_CPU_DIM_ALL_SCHEDULE:
            {
                if (!json_object_object_get_ex(new_obj, "dim1", &o))
                {
                    printf("Field %s does not exist\n", "dim1");
                    return;
                }
                dim_schedule_active.dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim2", &o))
                {
                    printf("Field %s does not exist\n", "dim2");
                    return;
                }
                dim_schedule_active.dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim3", &o))
                {
                    printf("Field %s does not exist\n", "dim3");
                    return;
                }
                dim_schedule_active.dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim4", &o))
                {
                    printf("Field %s does not exist\n", "dim4");
                    return;
                }
                dim_schedule_active.dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim5", &o))
                {
                    printf("Field %s does not exist\n", "dim5");
                    return;
                }
                dim_schedule_active.dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim6", &o))
                {
                    printf("Field %s does not exist\n", "dim6");
                    return;
                }
                dim_schedule_active.dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim7", &o))
                {
                    printf("Field %s does not exist\n", "dim7");
                    return;
                }
                dim_schedule_active.dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim8", &o))
                {
                    printf("Field %s does not exist\n", "dim8");
                    return;
                }
                dim_schedule_active.dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim9", &o))
                {
                    printf("Field %s does not exist\n", "dim9");
                    return;
                }
                dim_schedule_active.dim9 = (uint8_t)json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "hh_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim1");
                    return;
                }
                dim_schedule_active.hh_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim1");
                    return;
                }
                dim_schedule_active.mm_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim2");
                    return;
                }
                dim_schedule_active.hh_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim2");
                    return;
                }
                dim_schedule_active.mm_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim3");
                    return;
                }
                dim_schedule_active.hh_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim3");
                    return;
                }
                dim_schedule_active.mm_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim4");
                    return;
                }
                dim_schedule_active.hh_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim4");
                    return;
                }
                dim_schedule_active.mm_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim5");
                    return;
                }
                dim_schedule_active.hh_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim5");
                    return;
                }
                dim_schedule_active.mm_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim6");
                    return;
                }
                dim_schedule_active.hh_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim6");
                    return;
                }
                dim_schedule_active.mm_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim7");
                    return;
                }
                dim_schedule_active.hh_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim7");
                    return;
                }
                dim_schedule_active.mm_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim8");
                    return;
                }
                dim_schedule_active.hh_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim8");
                    return;
                }
                dim_schedule_active.mm_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim9");
                    return;
                }
                dim_schedule_active.hh_start_dim9 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim9");
                    return;
                }
                dim_schedule_active.mm_start_dim9 = (uint8_t)json_object_get_int(o);
                dim_schedule_active.group = 200;
                // #ifdef DEBUG_MAIN
                printf("group is: %d\r\n", dim_schedule_active.group);
                printf("dim1 is: %d\r\n", dim_schedule_active.dim1);
                printf("dim2 is: %d\r\n", dim_schedule_active.dim2);
                printf("dim3 is: %d\r\n", dim_schedule_active.dim3);
                printf("dim4 is: %d\r\n", dim_schedule_active.dim4);
                printf("dim5 is: %d\r\n", dim_schedule_active.dim5);
                printf("dim6 is: %d\r\n", dim_schedule_active.dim6);
                printf("dim7 is: %d\r\n", dim_schedule_active.dim7);
                printf("dim8 is: %d\r\n", dim_schedule_active.dim8);
                printf("dim9 is: %d\r\n", dim_schedule_active.dim9);
                printf("hh_start_dim1 is: %d\r\n", dim_schedule_active.hh_start_dim1);
                printf("mm_start_dim1 is: %d\r\n", dim_schedule_active.mm_start_dim1);
                printf("hh_start_dim2 is: %d\r\n", dim_schedule_active.hh_start_dim2);
                printf("mm_start_dim2 is: %d\r\n", dim_schedule_active.mm_start_dim2);
                printf("hh_start_dim3 is: %d\r\n", dim_schedule_active.hh_start_dim3);
                printf("mm_start_dim3 is: %d\r\n", dim_schedule_active.mm_start_dim3);
                printf("hh_start_dim4 is: %d\r\n", dim_schedule_active.hh_start_dim4);
                printf("mm_start_dim4 is: %d\r\n", dim_schedule_active.mm_start_dim4);
                printf("hh_start_dim5 is: %d\r\n", dim_schedule_active.hh_start_dim5);
                printf("mm_start_dim5 is: %d\r\n", dim_schedule_active.mm_start_dim5);
                printf("hh_start_dim6 is: %d\r\n", dim_schedule_active.hh_start_dim6);
                printf("mm_start_dim6 is: %d\r\n", dim_schedule_active.mm_start_dim6);
                printf("hh_start_dim7 is: %d\r\n", dim_schedule_active.hh_start_dim7);
                printf("mm_start_dim7 is: %d\r\n", dim_schedule_active.mm_start_dim7);
                printf("hh_start_dim8 is: %d\r\n", dim_schedule_active.hh_start_dim8);
                printf("mm_start_dim8 is: %d\r\n", dim_schedule_active.mm_start_dim8);
                printf("hh_start_dim9 is: %d\r\n", dim_schedule_active.hh_start_dim9);
                printf("mm_start_dim9 is: %d\r\n", dim_schedule_active.mm_start_dim9);
                // #endif
                printf("Schedule dim all ok\r\n");
                Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_active.txt", dim_schedule_active);

                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, 200,
                                       dim_schedule_active.dim1,
                                       dim_schedule_active.dim2,
                                       dim_schedule_active.dim3,
                                       dim_schedule_active.dim4,
                                       dim_schedule_active.dim5,
                                       dim_schedule_active.dim6,
                                       dim_schedule_active.dim7,
                                       dim_schedule_active.dim8,
                                       dim_schedule_active.dim9,
                                       dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1,
                                       dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2,
                                       dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3,
                                       dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4,
                                       dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5,
                                       dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6,
                                       dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7,
                                       dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8,
                                       dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                printf("Schedule dim all da dc xuat len man hinh QT\r\n");
                send_struct(CMD_CPU_TO_MASTER_SETTING_SCHEDULE_DIM_ALL_CONTROL, (uint8_t *)&dim_schedule_active, sizeof(dim_active_schedule_t));
                break;
            }

            case CMD_QT_TO_CPU_DIM_ID:
            {
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add.id = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim", &o))
                {
                    printf("Field %s does not exist\n", "dim");
                    return;
                }
                add.dim = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "port", &o))
                {
                    printf("Field %s does not exist\n", "port");
                    return;
                }
                add.port = (uint8_t)json_object_get_int(o);
                add.group = 100;
                printf("id is: %d\r\n", add.id);
                printf("port is: %d\r\n", add.port);
                printf("group is: %d\r\n", add.group);
                printf("dim is: %d\r\n", add.dim);
                printf("DIM theo id ok\r\n");
                if (add.port == 1)
                {
                    fb_dim_port1.group[add.id] = 100;
                    fb_dim_port1.dim_fb[add.id] = add.dim;
                }
                if (add.port == 2)
                {
                    fb_dim_port2.group[add.id] = 100;
                    fb_dim_port2.dim_fb[add.id] = add.dim;
                }
                send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                flag_dimstatus = 1;
                printf("Da send den master ok\r\n");
                break;
            }

            case CMD_QT_TO_CPU_DIM_ID_DISABLE:
            {
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add.id = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim", &o))
                {
                    printf("Field %s does not exist\n", "dim");
                    return;
                }
                add.dim = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "port", &o))
                {
                    printf("Field %s does not exist\n", "port");
                    return;
                }
                add.port = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                add.group = (uint8_t)json_object_get_int(o);
                printf("id is: %d\r\n", add.id);
                printf("port is: %d\r\n", add.port);
                printf("group is: %d\r\n", add.group);
                printf("dim is: %d\r\n", add.dim);
                printf("DIM theo id ok\r\n");
                if (add.port == 1)
                {
                    fb_dim_port1.group[add.id] = 200;
                    fb_dim_port1.dim_fb[add.id] = add.dim;
                }
                if (add.port == 2)
                {
                    fb_dim_port2.group[add.id] = 200;
                    fb_dim_port2.dim_fb[add.id] = add.dim;
                }
                send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
                flag_dimstatus = 1;
                printf("Da send den master ok\r\n");
                break;
            }

            case CMD_QT_TO_CPU_SCHEDULE_GROUP_1:
            {
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                dim_schedule_group_1.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim1", &o))
                {
                    printf("Field %s does not exist\n", "dim1");
                    return;
                }
                dim_schedule_group_1.dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim2", &o))
                {
                    printf("Field %s does not exist\n", "dim2");
                    return;
                }
                dim_schedule_group_1.dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim3", &o))
                {
                    printf("Field %s does not exist\n", "dim3");
                    return;
                }
                dim_schedule_group_1.dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim4", &o))
                {
                    printf("Field %s does not exist\n", "dim4");
                    return;
                }
                dim_schedule_group_1.dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim5", &o))
                {
                    printf("Field %s does not exist\n", "dim5");
                    return;
                }
                dim_schedule_group_1.dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim6", &o))
                {
                    printf("Field %s does not exist\n", "dim6");
                    return;
                }
                dim_schedule_group_1.dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim7", &o))
                {
                    printf("Field %s does not exist\n", "dim7");
                    return;
                }
                dim_schedule_group_1.dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim8", &o))
                {
                    printf("Field %s does not exist\n", "dim8");
                    return;
                }
                dim_schedule_group_1.dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim9", &o))
                {
                    printf("Field %s does not exist\n", "dim9");
                    return;
                }
                dim_schedule_group_1.dim9 = (uint8_t)json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "hh_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim1");
                    return;
                }
                dim_schedule_group_1.hh_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim1");
                    return;
                }
                dim_schedule_group_1.mm_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim2");
                    return;
                }
                dim_schedule_group_1.hh_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim2");
                    return;
                }
                dim_schedule_group_1.mm_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim3");
                    return;
                }
                dim_schedule_group_1.hh_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim3");
                    return;
                }
                dim_schedule_group_1.mm_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim4");
                    return;
                }
                dim_schedule_group_1.hh_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim4");
                    return;
                }
                dim_schedule_group_1.mm_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim5");
                    return;
                }
                dim_schedule_group_1.hh_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim5");
                    return;
                }
                dim_schedule_group_1.mm_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim6");
                    return;
                }
                dim_schedule_group_1.hh_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim6");
                    return;
                }
                dim_schedule_group_1.mm_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim7");
                    return;
                }
                dim_schedule_group_1.hh_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim7");
                    return;
                }
                dim_schedule_group_1.mm_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim8");
                    return;
                }
                dim_schedule_group_1.hh_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim8");
                    return;
                }
                dim_schedule_group_1.mm_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim9");
                    return;
                }
                dim_schedule_group_1.hh_start_dim9 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim9");
                    return;
                }
                dim_schedule_group_1.mm_start_dim9 = (uint8_t)json_object_get_int(o);
                // #ifdef DEBUG_MAIN
                printf("group is: %d\r\n", dim_schedule_group_1.group);
                printf("dim1 is: %d\r\n", dim_schedule_group_1.dim1);
                printf("dim2 is: %d\r\n", dim_schedule_group_1.dim2);
                printf("dim3 is: %d\r\n", dim_schedule_group_1.dim3);
                printf("dim4 is: %d\r\n", dim_schedule_group_1.dim4);
                printf("dim5 is: %d\r\n", dim_schedule_group_1.dim5);
                printf("dim6 is: %d\r\n", dim_schedule_group_1.dim6);
                printf("dim7 is: %d\r\n", dim_schedule_group_1.dim7);
                printf("dim8 is: %d\r\n", dim_schedule_group_1.dim8);
                printf("dim9 is: %d\r\n", dim_schedule_group_1.dim9);
                printf("hh_start_dim1 is: %d\r\n", dim_schedule_group_1.hh_start_dim1);
                printf("mm_start_dim1 is: %d\r\n", dim_schedule_group_1.mm_start_dim1);
                printf("hh_start_dim2 is: %d\r\n", dim_schedule_group_1.hh_start_dim2);
                printf("mm_start_dim2 is: %d\r\n", dim_schedule_group_1.mm_start_dim2);
                printf("hh_start_dim3 is: %d\r\n", dim_schedule_group_1.hh_start_dim3);
                printf("mm_start_dim3 is: %d\r\n", dim_schedule_group_1.mm_start_dim3);
                printf("hh_start_dim4 is: %d\r\n", dim_schedule_group_1.hh_start_dim4);
                printf("mm_start_dim4 is: %d\r\n", dim_schedule_group_1.mm_start_dim4);
                printf("hh_start_dim5 is: %d\r\n", dim_schedule_group_1.hh_start_dim5);
                printf("mm_start_dim5 is: %d\r\n", dim_schedule_group_1.mm_start_dim5);
                printf("hh_start_dim6 is: %d\r\n", dim_schedule_group_1.hh_start_dim6);
                printf("mm_start_dim6 is: %d\r\n", dim_schedule_group_1.mm_start_dim6);
                printf("hh_start_dim7 is: %d\r\n", dim_schedule_group_1.hh_start_dim7);
                printf("mm_start_dim7 is: %d\r\n", dim_schedule_group_1.mm_start_dim7);
                printf("hh_start_dim8 is: %d\r\n", dim_schedule_group_1.hh_start_dim8);
                printf("mm_start_dim8 is: %d\r\n", dim_schedule_group_1.mm_start_dim8);
                printf("hh_start_dim9 is: %d\r\n", dim_schedule_group_1.hh_start_dim9);
                printf("mm_start_dim9 is: %d\r\n", dim_schedule_group_1.mm_start_dim9);
                // #endif
                if ((dim_schedule_group_1.group == 1) || (dim_schedule_group_1.group == 99))
                {
                    printf("Schedule dim group 1 ok\r\n");
                    send_struct(CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP, (uint8_t *)&dim_schedule_group_1, sizeof(dim_active_schedule_t));
                    Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_1.txt", dim_schedule_group_1);
                    printf("Schedule dim group 1 da duoc luu ra file ok\r\n");
                }
                if ((dim_schedule_group_1.group == 2) || (dim_schedule_group_1.group == 98))
                {
                    printf("Schedule dim group 2 ok\r\n");
                    Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
                    printf("Schedule dim group 2 da duoc luu ra file ok\r\n");
                }
                if ((dim_schedule_group_1.group == 3) || (dim_schedule_group_1.group == 97))
                {
                    printf("Schedule dim group 3 ok\r\n");
                    Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
                    printf("Schedule dim group 3 da duoc luu ra file ok\r\n");
                }
                break;
            }

            case CMD_QT_TO_CPU_SCHEDULE_GROUP_2:
            {
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                dim_schedule_group_2.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim1", &o))
                {
                    printf("Field %s does not exist\n", "dim1");
                    return;
                }
                dim_schedule_group_2.dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim2", &o))
                {
                    printf("Field %s does not exist\n", "dim2");
                    return;
                }
                dim_schedule_group_2.dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim3", &o))
                {
                    printf("Field %s does not exist\n", "dim3");
                    return;
                }
                dim_schedule_group_2.dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim4", &o))
                {
                    printf("Field %s does not exist\n", "dim4");
                    return;
                }
                dim_schedule_group_2.dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim5", &o))
                {
                    printf("Field %s does not exist\n", "dim5");
                    return;
                }
                dim_schedule_group_2.dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim6", &o))
                {
                    printf("Field %s does not exist\n", "dim6");
                    return;
                }
                dim_schedule_group_2.dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim7", &o))
                {
                    printf("Field %s does not exist\n", "dim7");
                    return;
                }
                dim_schedule_group_2.dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim8", &o))
                {
                    printf("Field %s does not exist\n", "dim8");
                    return;
                }
                dim_schedule_group_2.dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim9", &o))
                {
                    printf("Field %s does not exist\n", "dim9");
                    return;
                }
                dim_schedule_group_2.dim9 = (uint8_t)json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "hh_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim1");
                    return;
                }
                dim_schedule_group_2.hh_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim1");
                    return;
                }
                dim_schedule_group_2.mm_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim2");
                    return;
                }
                dim_schedule_group_2.hh_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim2");
                    return;
                }
                dim_schedule_group_2.mm_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim3");
                    return;
                }
                dim_schedule_group_2.hh_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim3");
                    return;
                }
                dim_schedule_group_2.mm_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim4");
                    return;
                }
                dim_schedule_group_2.hh_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim4");
                    return;
                }
                dim_schedule_group_2.mm_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim5");
                    return;
                }
                dim_schedule_group_2.hh_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim5");
                    return;
                }
                dim_schedule_group_2.mm_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim6");
                    return;
                }
                dim_schedule_group_2.hh_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim6");
                    return;
                }
                dim_schedule_group_2.mm_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim7");
                    return;
                }
                dim_schedule_group_2.hh_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim7");
                    return;
                }
                dim_schedule_group_2.mm_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim8");
                    return;
                }
                dim_schedule_group_2.hh_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim8");
                    return;
                }
                dim_schedule_group_2.mm_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim9");
                    return;
                }
                dim_schedule_group_2.hh_start_dim9 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim9");
                    return;
                }
                dim_schedule_group_2.mm_start_dim9 = (uint8_t)json_object_get_int(o);
                // #ifdef DEBUG_MAIN
                printf("group is: %d\r\n", dim_schedule_group_2.group);
                printf("dim1 is: %d\r\n", dim_schedule_group_2.dim1);
                printf("dim2 is: %d\r\n", dim_schedule_group_2.dim2);
                printf("dim3 is: %d\r\n", dim_schedule_group_2.dim3);
                printf("dim4 is: %d\r\n", dim_schedule_group_2.dim4);
                printf("dim5 is: %d\r\n", dim_schedule_group_2.dim5);
                printf("dim6 is: %d\r\n", dim_schedule_group_2.dim6);
                printf("dim7 is: %d\r\n", dim_schedule_group_2.dim7);
                printf("dim8 is: %d\r\n", dim_schedule_group_2.dim8);
                printf("dim9 is: %d\r\n", dim_schedule_group_2.dim9);
                printf("hh_start_dim1 is: %d\r\n", dim_schedule_group_2.hh_start_dim1);
                printf("mm_start_dim1 is: %d\r\n", dim_schedule_group_2.mm_start_dim1);
                printf("hh_start_dim2 is: %d\r\n", dim_schedule_group_2.hh_start_dim2);
                printf("mm_start_dim2 is: %d\r\n", dim_schedule_group_2.mm_start_dim2);
                printf("hh_start_dim3 is: %d\r\n", dim_schedule_group_2.hh_start_dim3);
                printf("mm_start_dim3 is: %d\r\n", dim_schedule_group_2.mm_start_dim3);
                printf("hh_start_dim4 is: %d\r\n", dim_schedule_group_2.hh_start_dim4);
                printf("mm_start_dim4 is: %d\r\n", dim_schedule_group_2.mm_start_dim4);
                printf("hh_start_dim5 is: %d\r\n", dim_schedule_group_2.hh_start_dim5);
                printf("mm_start_dim5 is: %d\r\n", dim_schedule_group_2.mm_start_dim5);
                printf("hh_start_dim6 is: %d\r\n", dim_schedule_group_2.hh_start_dim6);
                printf("mm_start_dim6 is: %d\r\n", dim_schedule_group_2.mm_start_dim6);
                printf("hh_start_dim7 is: %d\r\n", dim_schedule_group_2.hh_start_dim7);
                printf("mm_start_dim7 is: %d\r\n", dim_schedule_group_2.mm_start_dim7);
                printf("hh_start_dim8 is: %d\r\n", dim_schedule_group_2.hh_start_dim8);
                printf("mm_start_dim8 is: %d\r\n", dim_schedule_group_2.mm_start_dim8);
                printf("hh_start_dim9 is: %d\r\n", dim_schedule_group_2.hh_start_dim9);
                printf("mm_start_dim9 is: %d\r\n", dim_schedule_group_2.mm_start_dim9);
                // #endif
                if ((dim_schedule_group_2.group == 2) || (dim_schedule_group_2.group == 98))
                {
                    printf("Schedule dim group 2 ok\r\n");
                    send_struct(CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP, (uint8_t *)&dim_schedule_group_2, sizeof(dim_active_schedule_t));
                    Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
                    printf("Schedule dim group 2 da duoc luu ra file ok\r\n");
                }
                break;
            }

            case CMD_QT_TO_CPU_SCHEDULE_GROUP_3:
            {
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                dim_schedule_group_3.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim1", &o))
                {
                    printf("Field %s does not exist\n", "dim1");
                    return;
                }
                dim_schedule_group_3.dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim2", &o))
                {
                    printf("Field %s does not exist\n", "dim2");
                    return;
                }
                dim_schedule_group_3.dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim3", &o))
                {
                    printf("Field %s does not exist\n", "dim3");
                    return;
                }
                dim_schedule_group_3.dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim4", &o))
                {
                    printf("Field %s does not exist\n", "dim4");
                    return;
                }
                dim_schedule_group_3.dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim5", &o))
                {
                    printf("Field %s does not exist\n", "dim5");
                    return;
                }
                dim_schedule_group_3.dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim6", &o))
                {
                    printf("Field %s does not exist\n", "dim6");
                    return;
                }
                dim_schedule_group_3.dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim7", &o))
                {
                    printf("Field %s does not exist\n", "dim7");
                    return;
                }
                dim_schedule_group_3.dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim8", &o))
                {
                    printf("Field %s does not exist\n", "dim8");
                    return;
                }
                dim_schedule_group_3.dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim9", &o))
                {
                    printf("Field %s does not exist\n", "dim9");
                    return;
                }
                dim_schedule_group_3.dim9 = (uint8_t)json_object_get_int(o);

                if (!json_object_object_get_ex(new_obj, "hh_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim1");
                    return;
                }
                dim_schedule_group_3.hh_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim1", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim1");
                    return;
                }
                dim_schedule_group_3.mm_start_dim1 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim2");
                    return;
                }
                dim_schedule_group_3.hh_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim2", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim2");
                    return;
                }
                dim_schedule_group_3.mm_start_dim2 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim3");
                    return;
                }
                dim_schedule_group_3.hh_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim3", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim3");
                    return;
                }
                dim_schedule_group_3.mm_start_dim3 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim4");
                    return;
                }
                dim_schedule_group_3.hh_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim4", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim4");
                    return;
                }
                dim_schedule_group_3.mm_start_dim4 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim5");
                    return;
                }
                dim_schedule_group_3.hh_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim5", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim5");
                    return;
                }
                dim_schedule_group_3.mm_start_dim5 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim6");
                    return;
                }
                dim_schedule_group_3.hh_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim6", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim6");
                    return;
                }
                dim_schedule_group_3.mm_start_dim6 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim7");
                    return;
                }
                dim_schedule_group_3.hh_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim7", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim7");
                    return;
                }
                dim_schedule_group_3.mm_start_dim7 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim8");
                    return;
                }
                dim_schedule_group_3.hh_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim8", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim8");
                    return;
                }
                dim_schedule_group_3.mm_start_dim8 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "hh_start_dim9");
                    return;
                }
                dim_schedule_group_3.hh_start_dim9 = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_dim9", &o))
                {
                    printf("Field %s does not exist\n", "mm_start_dim9");
                    return;
                }
                dim_schedule_group_3.mm_start_dim9 = (uint8_t)json_object_get_int(o);
                // #ifdef DEBUG_MAIN
                printf("group is: %d\r\n", dim_schedule_group_3.group);
                printf("dim1 is: %d\r\n", dim_schedule_group_3.dim1);
                printf("dim2 is: %d\r\n", dim_schedule_group_3.dim2);
                printf("dim3 is: %d\r\n", dim_schedule_group_3.dim3);
                printf("dim4 is: %d\r\n", dim_schedule_group_3.dim4);
                printf("dim5 is: %d\r\n", dim_schedule_group_3.dim5);
                printf("dim6 is: %d\r\n", dim_schedule_group_3.dim6);
                printf("dim7 is: %d\r\n", dim_schedule_group_3.dim7);
                printf("dim8 is: %d\r\n", dim_schedule_group_3.dim8);
                printf("dim9 is: %d\r\n", dim_schedule_group_3.dim9);
                printf("hh_start_dim1 is: %d\r\n", dim_schedule_group_3.hh_start_dim1);
                printf("mm_start_dim1 is: %d\r\n", dim_schedule_group_3.mm_start_dim1);
                printf("hh_start_dim2 is: %d\r\n", dim_schedule_group_3.hh_start_dim2);
                printf("mm_start_dim2 is: %d\r\n", dim_schedule_group_3.mm_start_dim2);
                printf("hh_start_dim3 is: %d\r\n", dim_schedule_group_3.hh_start_dim3);
                printf("mm_start_dim3 is: %d\r\n", dim_schedule_group_3.mm_start_dim3);
                printf("hh_start_dim4 is: %d\r\n", dim_schedule_group_3.hh_start_dim4);
                printf("mm_start_dim4 is: %d\r\n", dim_schedule_group_3.mm_start_dim4);
                printf("hh_start_dim5 is: %d\r\n", dim_schedule_group_3.hh_start_dim5);
                printf("mm_start_dim5 is: %d\r\n", dim_schedule_group_3.mm_start_dim5);
                printf("hh_start_dim6 is: %d\r\n", dim_schedule_group_3.hh_start_dim6);
                printf("mm_start_dim6 is: %d\r\n", dim_schedule_group_3.mm_start_dim6);
                printf("hh_start_dim7 is: %d\r\n", dim_schedule_group_3.hh_start_dim7);
                printf("mm_start_dim7 is: %d\r\n", dim_schedule_group_3.mm_start_dim7);
                printf("hh_start_dim8 is: %d\r\n", dim_schedule_group_3.hh_start_dim8);
                printf("mm_start_dim8 is: %d\r\n", dim_schedule_group_3.mm_start_dim8);
                printf("hh_start_dim9 is: %d\r\n", dim_schedule_group_3.hh_start_dim9);
                printf("mm_start_dim9 is: %d\r\n", dim_schedule_group_3.mm_start_dim9);
                // #endif
                if ((dim_schedule_group_3.group == 3) || (dim_schedule_group_3.group == 97))
                {
                    printf("Schedule dim group 3 ok\r\n");
                    send_struct(CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP, (uint8_t *)&dim_schedule_group_3, sizeof(dim_active_schedule_t));
                    Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
                    printf("Schedule dim group 3 da duoc luu ra file ok\r\n");
                }
                break;
            }

            case CMD_CPU_TO_QT_SCHEDULE_GROUP_1:
            {
                dim_schedule_group_1 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_1.txt", dim_schedule_group_1);
                printf("group is %d\r\n", dim_schedule_group_1.group);
                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_1.group,
                                       dim_schedule_group_1.dim1,
                                       dim_schedule_group_1.dim2,
                                       dim_schedule_group_1.dim3,
                                       dim_schedule_group_1.dim4,
                                       dim_schedule_group_1.dim5,
                                       dim_schedule_group_1.dim6,
                                       dim_schedule_group_1.dim7,
                                       dim_schedule_group_1.dim8,
                                       dim_schedule_group_1.dim9,
                                       dim_schedule_group_1.hh_start_dim1, dim_schedule_group_1.mm_start_dim1,
                                       dim_schedule_group_1.hh_start_dim2, dim_schedule_group_1.mm_start_dim2,
                                       dim_schedule_group_1.hh_start_dim3, dim_schedule_group_1.mm_start_dim3,
                                       dim_schedule_group_1.hh_start_dim4, dim_schedule_group_1.mm_start_dim4,
                                       dim_schedule_group_1.hh_start_dim5, dim_schedule_group_1.mm_start_dim5,
                                       dim_schedule_group_1.hh_start_dim6, dim_schedule_group_1.mm_start_dim6,
                                       dim_schedule_group_1.hh_start_dim7, dim_schedule_group_1.mm_start_dim7,
                                       dim_schedule_group_1.hh_start_dim8, dim_schedule_group_1.mm_start_dim8,
                                       dim_schedule_group_1.hh_start_dim9, dim_schedule_group_1.mm_start_dim9);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                printf("Schedule dim group 1 da dc gui den QT ok\r\n");
                break;
            }

            case CMD_CPU_TO_QT_SCHEDULE_GROUP_2:
            {
                dim_schedule_group_2 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
                printf("group is %d\r\n", dim_schedule_group_2.group);
                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_2.group,
                                       dim_schedule_group_2.dim1,
                                       dim_schedule_group_2.dim2,
                                       dim_schedule_group_2.dim3,
                                       dim_schedule_group_2.dim4,
                                       dim_schedule_group_2.dim5,
                                       dim_schedule_group_2.dim6,
                                       dim_schedule_group_2.dim7,
                                       dim_schedule_group_2.dim8,
                                       dim_schedule_group_2.dim9,
                                       dim_schedule_group_2.hh_start_dim1, dim_schedule_group_2.mm_start_dim1,
                                       dim_schedule_group_2.hh_start_dim2, dim_schedule_group_2.mm_start_dim2,
                                       dim_schedule_group_2.hh_start_dim3, dim_schedule_group_2.mm_start_dim3,
                                       dim_schedule_group_2.hh_start_dim4, dim_schedule_group_2.mm_start_dim4,
                                       dim_schedule_group_2.hh_start_dim5, dim_schedule_group_2.mm_start_dim5,
                                       dim_schedule_group_2.hh_start_dim6, dim_schedule_group_2.mm_start_dim6,
                                       dim_schedule_group_2.hh_start_dim7, dim_schedule_group_2.mm_start_dim7,
                                       dim_schedule_group_2.hh_start_dim8, dim_schedule_group_2.mm_start_dim8,
                                       dim_schedule_group_2.hh_start_dim9, dim_schedule_group_2.mm_start_dim9);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                printf("Schedule dim group 2 da dc gui den QT ok\r\n");
                break;
            }

            case CMD_CPU_TO_QT_SCHEDULE_GROUP_3:
            {
                dim_schedule_group_3 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
                printf("group is %d\r\n", dim_schedule_group_3.group);
                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_3.group,
                                       dim_schedule_group_3.dim1,
                                       dim_schedule_group_3.dim2,
                                       dim_schedule_group_3.dim3,
                                       dim_schedule_group_3.dim4,
                                       dim_schedule_group_3.dim5,
                                       dim_schedule_group_3.dim6,
                                       dim_schedule_group_3.dim7,
                                       dim_schedule_group_3.dim8,
                                       dim_schedule_group_3.dim9,
                                       dim_schedule_group_3.hh_start_dim1, dim_schedule_group_3.mm_start_dim1,
                                       dim_schedule_group_3.hh_start_dim2, dim_schedule_group_3.mm_start_dim2,
                                       dim_schedule_group_3.hh_start_dim3, dim_schedule_group_3.mm_start_dim3,
                                       dim_schedule_group_3.hh_start_dim4, dim_schedule_group_3.mm_start_dim4,
                                       dim_schedule_group_3.hh_start_dim5, dim_schedule_group_3.mm_start_dim5,
                                       dim_schedule_group_3.hh_start_dim6, dim_schedule_group_3.mm_start_dim6,
                                       dim_schedule_group_3.hh_start_dim7, dim_schedule_group_3.mm_start_dim7,
                                       dim_schedule_group_3.hh_start_dim8, dim_schedule_group_3.mm_start_dim8,
                                       dim_schedule_group_3.hh_start_dim9, dim_schedule_group_3.mm_start_dim9);

                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                printf("Schedule dim group 3 da dc gui den QT ok\r\n");
                break;
            }

            case CMD_QT_TO_CPU_SETUP_INFO:
                printf("Bat dau thiet lap add va group\r\n");
                add.id = 1;
                add.port = 100;
                add.group = 200;
                add.dim = 70;
                send_struct(CMD_CPU_TO_MASTER_SETUP_INFO, (uint8_t *)&add, sizeof(add_t));
                break;
            case CMD_QT_TO_CPU_COUNT_LAMP:
                printf("Bat dau dem so luong den\r\n");
                send_struct(CMD_CPU_TO_MASTER_COUNT_LAMP, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
                break;
            case CMD_QT_TO_CPU_ADD_ID_TO_GROUP:
            {
                if (!json_object_object_get_ex(new_obj, "port", &o))
                {
                    printf("Field %s does not exist\n", "port");
                    return;
                }
                add.port = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                add.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add.id = (uint8_t)json_object_get_int(o);
                if (add.group == 1)
                {
                    if ((group1.add16 == 0) && (group1.add15 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10) && (add.id != group1.add11) && (add.id != group1.add12) && (add.id != group1.add13) && (add.id != group1.add14) && (add.id != group1.add15))
                        group1.add16 = (uint8_t)json_object_get_int(o);
                    if ((group1.add15 == 0) && (group1.add14 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10) && (add.id != group1.add11) && (add.id != group1.add12) && (add.id != group1.add13) && (add.id != group1.add14))
                        group1.add15 = (uint8_t)json_object_get_int(o);
                    if ((group1.add14 == 0) && (group1.add13 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10) && (add.id != group1.add11) && (add.id != group1.add12) && (add.id != group1.add13))
                        group1.add14 = (uint8_t)json_object_get_int(o);
                    if ((group1.add13 == 0) && (group1.add12 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10) && (add.id != group1.add11) && (add.id != group1.add12))
                        group1.add13 = (uint8_t)json_object_get_int(o);
                    if ((group1.add12 == 0) && (group1.add11 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10) && (add.id != group1.add11))
                        group1.add12 = (uint8_t)json_object_get_int(o);
                    if ((group1.add11 == 0) && (group1.add10 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9) && (add.id != group1.add10))
                        group1.add11 = (uint8_t)json_object_get_int(o);
                    if ((group1.add10 == 0) && (group1.add9 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8) && (add.id != group1.add9))
                        group1.add10 = (uint8_t)json_object_get_int(o);
                    if ((group1.add9 == 0) && (group1.add8 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7) && (add.id != group1.add8))
                        group1.add9 = (uint8_t)json_object_get_int(o);
                    if ((group1.add8 == 0) && (group1.add7 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6) && (add.id != group1.add7))
                        group1.add8 = (uint8_t)json_object_get_int(o);
                    if ((group1.add7 == 0) && (group1.add6 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5) && (add.id != group1.add6))
                        group1.add7 = (uint8_t)json_object_get_int(o);
                    if ((group1.add6 == 0) && (group1.add5 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4) && (add.id != group1.add5))
                        group1.add6 = (uint8_t)json_object_get_int(o);
                    if ((group1.add5 == 0) && (group1.add4 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3) && (add.id != group1.add4))
                        group1.add5 = (uint8_t)json_object_get_int(o);
                    if ((group1.add4 == 0) && (group1.add3 != 0) && (add.id != group1.add1) && (add.id != group1.add2) && (add.id != group1.add3))
                        group1.add4 = (uint8_t)json_object_get_int(o);
                    if ((group1.add3 == 0) && (group1.add2 != 0) && (add.id != group1.add1) && (add.id != group1.add2))
                        group1.add3 = (uint8_t)json_object_get_int(o);
                    if ((group1.add2 == 0) && (group1.add1 != 0) && (add.id != group1.add1))
                        group1.add2 = (uint8_t)json_object_get_int(o);
                    if (group1.add1 == 0)
                        group1.add1 = (uint8_t)json_object_get_int(o);
                    char data[1000] = {0};
                    int data_len = sprintf(data, TEMPLATE_FB_ADD_IN_GROUP, 244,
                                           1, group1.add1, group1.add2, group1.add3, group1.add4, group1.add5, group1.add6, group1.add7, group1.add8, group1.add9, group1.add10, group1.add11, group1.add12, group1.add13, group1.add14, group1.add15, group1.add16);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                }
                if (add.group == 2)
                {
                    if ((group2.add16 == 0) && (group2.add15 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10) && (add.id != group2.add11) && (add.id != group2.add12) && (add.id != group2.add13) && (add.id != group2.add14) && (add.id != group2.add15))
                        group2.add16 = (uint8_t)json_object_get_int(o);
                    if ((group2.add15 == 0) && (group2.add14 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10) && (add.id != group2.add11) && (add.id != group2.add12) && (add.id != group2.add13) && (add.id != group2.add14))
                        group2.add15 = (uint8_t)json_object_get_int(o);
                    if ((group2.add14 == 0) && (group2.add13 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10) && (add.id != group2.add11) && (add.id != group2.add12) && (add.id != group2.add13))
                        group2.add14 = (uint8_t)json_object_get_int(o);
                    if ((group2.add13 == 0) && (group2.add12 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10) && (add.id != group2.add11) && (add.id != group2.add12))
                        group2.add13 = (uint8_t)json_object_get_int(o);
                    if ((group2.add12 == 0) && (group2.add11 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10) && (add.id != group2.add11))
                        group2.add12 = (uint8_t)json_object_get_int(o);
                    if ((group2.add11 == 0) && (group2.add10 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9) && (add.id != group2.add10))
                        group2.add11 = (uint8_t)json_object_get_int(o);
                    if ((group2.add10 == 0) && (group2.add9 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8) && (add.id != group2.add9))
                        group2.add10 = (uint8_t)json_object_get_int(o);
                    if ((group2.add9 == 0) && (group2.add8 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7) && (add.id != group2.add8))
                        group2.add9 = (uint8_t)json_object_get_int(o);
                    if ((group2.add8 == 0) && (group2.add7 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6) && (add.id != group2.add7))
                        group2.add8 = (uint8_t)json_object_get_int(o);
                    if ((group2.add7 == 0) && (group2.add6 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5) && (add.id != group2.add6))
                        group2.add7 = (uint8_t)json_object_get_int(o);
                    if ((group2.add6 == 0) && (group2.add5 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4) && (add.id != group2.add5))
                        group2.add6 = (uint8_t)json_object_get_int(o);
                    if ((group2.add5 == 0) && (group2.add4 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3) && (add.id != group2.add4))
                        group2.add5 = (uint8_t)json_object_get_int(o);
                    if ((group2.add4 == 0) && (group2.add3 != 0) && (add.id != group2.add1) && (add.id != group2.add2) && (add.id != group2.add3))
                        group2.add4 = (uint8_t)json_object_get_int(o);
                    if ((group2.add3 == 0) && (group2.add2 != 0) && (add.id != group2.add1) && (add.id != group2.add2))
                        group2.add3 = (uint8_t)json_object_get_int(o);
                    if ((group2.add2 == 0) && (group2.add1 != 0) && (add.id != group2.add1))
                        group2.add2 = (uint8_t)json_object_get_int(o);
                    if (group2.add1 == 0)
                        group2.add1 = (uint8_t)json_object_get_int(o);
                    char data[1000] = {0};
                    int data_len = sprintf(data, TEMPLATE_FB_ADD_IN_GROUP, 244,
                                           2, group2.add1, group2.add2, group2.add3, group2.add4, group2.add5, group2.add6, group2.add7, group2.add8, group2.add9, group2.add10, group2.add11, group2.add12, group2.add13, group2.add14, group2.add15, group2.add16);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                }
                if (add.group == 3)
                {
                    if ((group3.add16 == 0) && (group3.add15 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10) && (add.id != group3.add11) && (add.id != group3.add12) && (add.id != group3.add13) && (add.id != group3.add14) && (add.id != group3.add15))
                        group3.add16 = (uint8_t)json_object_get_int(o);
                    if ((group3.add15 == 0) && (group3.add14 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10) && (add.id != group3.add11) && (add.id != group3.add12) && (add.id != group3.add13) && (add.id != group3.add14))
                        group3.add15 = (uint8_t)json_object_get_int(o);
                    if ((group3.add14 == 0) && (group3.add13 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10) && (add.id != group3.add11) && (add.id != group3.add12) && (add.id != group3.add13))
                        group3.add14 = (uint8_t)json_object_get_int(o);
                    if ((group3.add13 == 0) && (group3.add12 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10) && (add.id != group3.add11) && (add.id != group3.add12))
                        group3.add13 = (uint8_t)json_object_get_int(o);
                    if ((group3.add12 == 0) && (group3.add11 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10) && (add.id != group3.add11))
                        group3.add12 = (uint8_t)json_object_get_int(o);
                    if ((group3.add11 == 0) && (group3.add10 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9) && (add.id != group3.add10))
                        group3.add11 = (uint8_t)json_object_get_int(o);
                    if ((group3.add10 == 0) && (group3.add9 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8) && (add.id != group3.add9))
                        group3.add10 = (uint8_t)json_object_get_int(o);
                    if ((group3.add9 == 0) && (group3.add8 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7) && (add.id != group3.add8))
                        group3.add9 = (uint8_t)json_object_get_int(o);
                    if ((group3.add8 == 0) && (group3.add7 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6) && (add.id != group3.add7))
                        group3.add8 = (uint8_t)json_object_get_int(o);
                    if ((group3.add7 == 0) && (group3.add6 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5) && (add.id != group3.add6))
                        group3.add7 = (uint8_t)json_object_get_int(o);
                    if ((group3.add6 == 0) && (group3.add5 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4) && (add.id != group3.add5))
                        group3.add6 = (uint8_t)json_object_get_int(o);
                    if ((group3.add5 == 0) && (group3.add4 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3) && (add.id != group3.add4))
                        group3.add5 = (uint8_t)json_object_get_int(o);
                    if ((group3.add4 == 0) && (group3.add3 != 0) && (add.id != group3.add1) && (add.id != group3.add2) && (add.id != group3.add3))
                        group3.add4 = (uint8_t)json_object_get_int(o);
                    if ((group3.add3 == 0) && (group3.add2 != 0) && (add.id != group3.add1) && (add.id != group3.add2))
                        group3.add3 = (uint8_t)json_object_get_int(o);
                    if ((group3.add2 == 0) && (group3.add1 != 0) && (add.id != group3.add1))
                        group3.add2 = (uint8_t)json_object_get_int(o);
                    if (group3.add1 == 0)
                        group3.add1 = (uint8_t)json_object_get_int(o);
                    char data[1000] = {0};
                    int data_len = sprintf(data, TEMPLATE_FB_ADD_IN_GROUP, 244,
                                           3, group3.add1, group3.add2, group3.add3, group3.add4, group3.add5, group3.add6, group3.add7, group3.add8, group3.add9, group3.add10, group3.add11, group3.add12, group3.add13, group3.add14, group3.add15, group3.add16);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                }
                add.dim = 70;
                send_struct(CMD_CPU_TO_MASTER_ADD_ID_TO_GROUP, (uint8_t *)&add, sizeof(add_t));
                printf("Set id %d vao nhom ok\r\n", add.id);
                break;
            }
            case CMD_QT_TO_CPU_DIM_GROUP:
            {
                if (!json_object_object_get_ex(new_obj, "group", &o))
                {
                    printf("Field %s does not exist\n", "group");
                    return;
                }
                add.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "dim", &o))
                {
                    printf("Field %s does not exist\n", "dim");
                    return;
                }
                add.dim = (uint8_t)json_object_get_int(o);
                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                {
                    if (fb_dim_port1.group[i] == add.group)
                        fb_dim_port1.dim_fb[i] = add.dim;
                }
                for (int i = 1; i <= fb_dim_port2.lampport; i++)
                {
                    if (fb_dim_port2.group[i] == add.group)
                        fb_dim_port2.dim_fb[i] = add.dim;
                }
                send_struct(CMD_CPU_TO_MASTER_DIM_GROUP, (uint8_t *)&add, sizeof(add_t));
                printf("Dim nhom ok\r\n");
                break;
            }
            case CMD_QT_TO_CPU_OFF_SCREEN:
                printf("Tat man hinh\r\n");
                system("sudo vcgencmd display_power 0");
                break;
            case CMD_QT_TO_CPU_ON_SCREEN:
                printf("Bat man hinh\r\n");
                system("sudo vcgencmd display_power 1");
                break;
            case CMD_QT_TO_CPU_DKTB:
                printf("Dang ky thiet bi\r\n");
                printf("Mac ID: %s\r\n", mac_final);
                if (!json_object_object_get_ex(new_obj, "url", &o))
                {
                    printf("Field %s does not exist\n", "url");
                    return;
                }
                char *tmp = (char *)malloc(100);
                strcpy(tmp, json_object_get_string(o));
                memcpy(&setting.urlmqtt, tmp, 100);
                free(tmp);
                Write_struct_setting_toFile(setting_file, "/home/pi/setting.txt", setting);
                printf("setting.urlmqtt: %s\r\n", setting.urlmqtt);
                // char url[100] = "http://api.sitechlighting.one/device";
                char message[10000];
                snprintf(message, sizeof(message), "%s api.%s/device %s \"%s\", %s", "curl -X POST", setting.urlmqtt, "-H \"Content-Type: application/json\" -d '{\"deviceId\":", mac_final, "\"password\": \"aabbccddeeff\"}'");
                printf("%s\r\n", message);
                system(message);
                break;
            case CMD_QT_TO_CPU_CHANGE_ID_RF:
                printf("Change id rf\r\n");
                if (!json_object_object_get_ex(new_obj, "oldid", &o))
                {
                    printf("Field %s does not exist\n", "oldid");
                    return;
                }
                changeid.oldid = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "newid", &o))
                {
                    printf("Field %s does not exist\n", "newid");
                    return;
                }
                changeid.newid = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "cabinetid", &o))
                {
                    printf("Field %s does not exist\n", "cabinetid");
                    return;
                }
                changeid.cabinetid = (uint8_t)json_object_get_int(o);

                printf("parsing change id rf:%d, %d, %d\r\n", changeid.oldid, changeid.newid, changeid.cabinetid);
                send_struct(CMD_CPU_TO_MASTER_CHANGE_ID_RF, (uint8_t *)&changeid, sizeof(changeid_t));

                printf("Da send change id rf to master ok\r\n");
                break;
            case CMD_QT_TO_CPU_COUNT_LAMP_RF:
                printf("Count lamp rf\r\n");
                count_lamp_rf = 0;
                for (int i = 1; i <= fb_dim_port1.lampport; i++)
                {
                    send_byte(CMD_CPU_TO_MASTER_COUNT_LAMP_RF, i);
                    delay(200);
                }
                printf("Da send count lamp rf to master ok\r\n");
                break;
            case CMD_QT_TO_CPU_OPEN_SERVER:
                printf("Open connect to server!\r\n");
                if (!json_object_object_get_ex(new_obj, "server", &o))
                {
                    printf("Field %s does not exist\n", "server");
                    return;
                }
                setting.server = (uint8_t)json_object_get_int(o);
                printf("Setting server %d\r\n", setting.server);
                Write_struct_setting_toFile(setting_file, "/home/pi/setting.txt", setting);
                delay(100);
                if (setting.server == 1)
                {
                    char data_domain[200] = {0};
                    int data_domain_len = sprintf(data_domain, TEMPLATE_DOMAIN_SERVER, CMD_CPU_TO_QT_DOMAIN_SERVER, setting.urlmqtt);
                    cl->send_ex(cl, UWSC_OP_TEXT, 1, data_domain_len, data_domain);
                }
                break;

            case CMD_QT_TO_CPU_TYPE:
                printf("Set type!\r\n");
                if (!json_object_object_get_ex(new_obj, "type", &o))
                {
                    printf("Field %s does not exist\n", "type");
                    return;
                }
                setting.type = (uint8_t)json_object_get_int(o);
                Write_struct_setting_toFile(setting_file, "/home/pi/setting.txt", setting);
                printf("Server %d\r\n", setting.server);
                printf("Type %d\r\n", setting.type);
                break;
            case CMD_CPU_TO_QT_OPEN_SERVER:
                printf("Open connect to QT!\r\n");
                printf("Type %d\r\n", setting.type);
                char data_server[1000] = {0};
                int data_len = sprintf(data_server, TEMPLATE_SETTING, CMD_CPU_TO_QT_OPEN_SERVER, setting.server, setting.type);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data_server);
                printf("%s\r\n", data_server);
                break;
            case CMD_QT_TO_CPU_DELETE_LAMP_RF_GROUP_DIM1:
                printf("Clear all lamp in group 1\r\n");
                if (!json_object_object_get_ex(new_obj, "delete_group1", &o))
                {
                    printf("Field %s does not exist\n", "delete_group1");
                    return;
                }
                add_lamp_rf_group.trigger_delele_all = (uint8_t)json_object_get_int(o);
                if (add_lamp_rf_group.trigger_delele_all == 1)
                {
                    lamp_rf_group.port = 1;
                    lamp_rf_group.group = 200;
                    lamp_rf_group.dim = add.dim;
                    printf("gia tri queue rear: %d\r\n", queue.rear);
                    for (int i = 0; i <= queue.rear; i++)
                    {
                        lamp_rf_group.id = queue.inp_arr[i];
                        descrease_queue();
                        send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                        delay(500);
                        // show_queue();
                    }
                    Write_struct_queue_toFile(queue_file, "/home/pi/dim_group.txt", queue);
                    sort_up(queue.inp_arr, queue.rear);
                }
                // send_data_cpu_to_qt(queue.inp_arr, queue.rear, TEMPLATE_NUMBER_GROUP_DIM1, CMD_CPU_TO_QT_LAMP_RF_GROUP_DIM1);
                break;
            case CMD_QT_TO_CPU_SET_LAMP_RF_TO_GROUP:
                printf("Add id rf to group\r\n");
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add_lamp_rf_group.id = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "group_set", &o))
                {
                    printf("Field %s does not exist\n", "group_set");
                    return;
                }
                add_lamp_rf_group.group = (uint8_t)json_object_get_int(o);
                printf("parsing add id rf:%d, %d\r\n", add_lamp_rf_group.id, add_lamp_rf_group.group);
                if (add_lamp_rf_group.group == 1)
                {
                    lamp_rf_group.id = add_lamp_rf_group.id;
                    lamp_rf_group.port = 1;
                    lamp_rf_group.group = 100;
                    lamp_rf_group.dim = add.dim;
                    insert_queue(add_lamp_rf_group.id);
                    show_queue();
                    Write_struct_queue_toFile(queue_file, "/home/pi/dim_group.txt", queue);
                }
                else if (add_lamp_rf_group.group == 0)
                {
                    lamp_rf_group.id = add_lamp_rf_group.id;
                    lamp_rf_group.port = 1;
                    lamp_rf_group.group = 200;
                    lamp_rf_group.dim = add.dim;
                    printf("gia tri queue rear: %d\r\n", queue.rear);
                    for (int i = 0; i <= queue.rear; i++)
                    {
                        if (queue.inp_arr[i] == add_lamp_rf_group.id)
                        {
                            queue.inp_arr[i] = 0;
                            descrease_queue();
                            show_queue();
                            break;
                        }
                    }
                    send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                    Write_struct_queue_toFile(queue_file, "/home/pi/dim_group.txt", queue);
                    sort_up(queue.inp_arr, queue.rear);
                }
                send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                printf("Da send add id rf group to master ok\r\n");
                // send_data_cpu_to_qt(queue.inp_arr, queue.rear, TEMPLATE_NUMBER_GROUP_DIM1, CMD_CPU_TO_QT_LAMP_RF_GROUP_DIM1);
                break;
            case CMD_QT_TO_CPU_SET_LAMP_RS485_PORT_TO_GROUP:
            {
                printf("Add id lamp to group (MODE RS485)\r\n");
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add_lamp_port_group.id = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "group_set", &o))
                {
                    printf("Field %s does not exist\n", "group_set");
                    return;
                }
                add_lamp_port_group.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "port_set", &o))
                {
                    printf("Field %s does not exist\n", "port_set");
                    return;
                }
                add_lamp_port_group.port = (uint8_t)json_object_get_int(o);

                printf("parsing add id lamp:%d, %d, %d\r\n", add_lamp_port_group.id, add_lamp_port_group.group, add_lamp_port_group.port);

                lamp_rf_group.id = add_lamp_port_group.id;
                lamp_rf_group.port = add_lamp_port_group.port;
                lamp_rf_group.group = add_lamp_port_group.group;
                lamp_rf_group.dim = add.dim;

                if (add_lamp_port_group.group == 1)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        lamp_gr1_port1.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr1_port1, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);
                        lamp_gr1_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);

                        sort_down_lamp_group(lamp_gr1_port1.inp_lamp_arr, lamp_gr1_port1.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr1_port1, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        lamp_gr1_port2.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr1_port2, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);
                        lamp_gr1_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);

                        sort_down_lamp_group(lamp_gr1_port2.inp_lamp_arr, lamp_gr1_port2.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr1_port2, sizeof(xqueue_lamp_group_t));
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr1_port1, &lamp_gr1_port2, TEMPLATE_NUMBER_DIM_GROUP1, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1);
                }
                else if (add_lamp_port_group.group == 2)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        lamp_gr2_port1.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr2_port1, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);

                        lamp_gr2_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);

                        sort_down_lamp_group(lamp_gr2_port1.inp_lamp_arr, lamp_gr2_port1.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr2_port1, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        lamp_gr2_port2.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr2_port2, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);

                        lamp_gr2_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);

                        sort_down_lamp_group(lamp_gr2_port2.inp_lamp_arr, lamp_gr2_port2.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr2_port2, sizeof(xqueue_lamp_group_t));
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr2_port1, &lamp_gr2_port2, TEMPLATE_NUMBER_DIM_GROUP2, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2);
                }
                else if (add_lamp_port_group.group == 3)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        lamp_gr3_port1.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr3_port1, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);

                        lamp_gr3_port1 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);

                        sort_down_lamp_group(lamp_gr3_port1.inp_lamp_arr, lamp_gr3_port1.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr3_port1, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        lamp_gr3_port2.group = add_lamp_port_group.group;
                        insert_queue_lamp_group(add_lamp_port_group.id, &lamp_gr3_port2, add_lamp_port_group.port);
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);

                        lamp_gr3_port2 = Read_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);

                        sort_down_lamp_group(lamp_gr3_port2.inp_lamp_arr, lamp_gr3_port2.rear);

                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr3_port2, sizeof(xqueue_lamp_group_t));
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr3_port1, &lamp_gr3_port2, TEMPLATE_NUMBER_DIM_GROUP3, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3);
                }
                printf("Da send add id rs485 group to master ok\r\n");
                break;
            }
            case CMD_QT_TO_CPU_DELETE_LAMP_RS485_TO_GROUP:
                printf("Delete id lamp to group (MODE RS485)\r\n");
                if (!json_object_object_get_ex(new_obj, "id", &o))
                {
                    printf("Field %s does not exist\n", "id");
                    return;
                }
                add_lamp_port_group.id = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "group_set", &o))
                {
                    printf("Field %s does not exist\n", "group_set");
                    return;
                }
                add_lamp_port_group.group = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "port_set", &o))
                {
                    printf("Field %s does not exist\n", "port_set");
                    return;
                }
                add_lamp_port_group.port = (uint8_t)json_object_get_int(o);

                printf("parsing add id lamp:%d, %d, %d\r\n", add_lamp_port_group.id, add_lamp_port_group.group, add_lamp_port_group.port);

                lamp_rf_group.id = add_lamp_port_group.id;
                lamp_rf_group.port = add_lamp_port_group.port;
                lamp_rf_group.group = 200;
                lamp_rf_group.dim = add.dim;
                if (add_lamp_port_group.group == 1)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        printf("gia tri queue rear lamp rs483 group 1 port 1: %d\r\n", lamp_gr1_port1.rear);

                        for (int i = 0; i <= lamp_gr1_port1.rear; i++)
                        {
                            if (lamp_gr1_port1.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr1_port1.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr1_port1, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr1_port1, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr1_port1.inp_lamp_arr, lamp_gr1_port1.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr1_port1, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        for (int i = 0; i <= lamp_gr1_port2.rear; i++)
                        {
                            // printf("id : %d\r\n", lamp_gr1_port2.inp_lamp_arr[i]);
                            if (lamp_gr1_port2.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr1_port2.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr1_port2, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr1_port2, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr1_port2.inp_lamp_arr, lamp_gr1_port2.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr1_port2, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);
                        printf("gia tri queue rear lamp rs483 group 1 port 2: %d\r\n", lamp_gr1_port2.rear);
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr1_port1, &lamp_gr1_port2, TEMPLATE_NUMBER_DIM_GROUP1, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1);
                }
                else if (add_lamp_port_group.group == 2)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        printf("gia tri queue rear lamp rs483 group 2 port 1: %d\r\n", lamp_gr2_port1.rear);
                        for (int i = 0; i <= lamp_gr2_port1.rear; i++)
                        {
                            if (lamp_gr2_port1.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr2_port1.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr2_port1, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr2_port1, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr2_port1.inp_lamp_arr, lamp_gr2_port1.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr2_port1, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        printf("gia tri queue rear lamp rs483 group 2 port 2: %d\r\n", lamp_gr2_port2.rear);
                        for (int i = 0; i <= lamp_gr2_port2.rear; i++)
                        {
                            if (lamp_gr2_port2.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr2_port2.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr2_port2, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr2_port2, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr2_port2.inp_lamp_arr, lamp_gr2_port2.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr2_port2, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr2_port1, &lamp_gr2_port2, TEMPLATE_NUMBER_DIM_GROUP2, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2);
                }
                else if (add_lamp_port_group.group == 3)
                {
                    if (add_lamp_port_group.port == 1)
                    {
                        printf("gia tri queue rear lamp rs483 group [%d] port [%d]: %d, \r\n", add_lamp_port_group.group, add_lamp_port_group.port, lamp_gr2_port1.rear);
                        for (int i = 0; i <= lamp_gr3_port1.rear; i++)
                        {
                            if (lamp_gr3_port1.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr3_port1.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr3_port1, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr3_port1, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr3_port1.inp_lamp_arr, lamp_gr3_port1.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr3_port1, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);
                    }
                    else if (add_lamp_port_group.port == 2)
                    {
                        printf("gia tri queue rear lamp rs483 group [%d] port [%d]: %d, \r\n", add_lamp_port_group.group, add_lamp_port_group.port, lamp_gr2_port1.rear);
                        for (int i = 0; i <= lamp_gr3_port2.rear; i++)
                        {
                            if (lamp_gr3_port2.inp_lamp_arr[i] == add_lamp_port_group.id)
                            {
                                lamp_gr3_port2.inp_lamp_arr[i] = 0;
                                descrease_queue_lamp_group(&lamp_gr3_port2, add_lamp_port_group.port);
                                show_queue_lamp_group(&lamp_gr3_port2, add_lamp_port_group.port);
                                break;
                            }
                        }
                        sort_down_lamp_group(lamp_gr3_port2.inp_lamp_arr, lamp_gr3_port2.rear);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr3_port2, sizeof(xqueue_lamp_group_t));
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);
                    }
                    send_lamp_group_cpu_to_qt(&lamp_gr3_port1, &lamp_gr3_port2, TEMPLATE_NUMBER_DIM_GROUP3, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3);
                }
                break;
            case CMD_QT_TO_CPU_DELETE_LAMP_RS485_PORT_TO_GROUP:
                printf("Clear all lamp rs485 in group\r\n");
                if (!json_object_object_get_ex(new_obj, "delete_group1", &o))
                {
                    printf("Field %s does not exist\n", "delete_group1");
                    return;
                }
                add_lamp_port_group.trigger_delete_all = (uint8_t)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "port", &o))
                {
                    printf("Field %s does not exist\n", "port");
                    return;
                }
                add_lamp_port_group.port = (uint8_t)json_object_get_int(o);

                if (add_lamp_port_group.port == 1)
                {
                    lamp_rf_group.port = add_lamp_port_group.port;
                    lamp_rf_group.group = 200;
                    lamp_rf_group.dim = add.dim;

                    if (add_lamp_port_group.trigger_delete_all == 1) // group
                    {
                        printf("gia tri rear lamp port 1 group 1: %d\r\n", lamp_gr1_port1.rear);
                        for (int i = 0; i <= lamp_gr1_port1.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr1_port1.inp_lamp_arr[i];
                            lamp_gr1_port1.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr1_port1, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port1_file, ADDR_STORAGE_GROUP1_PORT1, lamp_gr1_port1);
                        sort_up_lamp_group(lamp_gr1_port1.inp_lamp_arr, lamp_gr1_port1.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr1_port1, &lamp_gr1_port2, TEMPLATE_NUMBER_DIM_GROUP1, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr1_port1, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.trigger_delete_all == 2)
                    {
                        printf("gia tri rear lamp port 1 group 2: %d\r\n", lamp_gr2_port1.rear);
                        for (int i = 0; i <= lamp_gr2_port1.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr2_port1.inp_lamp_arr[i];
                            lamp_gr2_port1.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr2_port1, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port1_file, ADDR_STORAGE_GROUP2_PORT1, lamp_gr2_port1);
                        sort_up_lamp_group(lamp_gr2_port1.inp_lamp_arr, lamp_gr2_port1.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr2_port1, &lamp_gr2_port2, TEMPLATE_NUMBER_DIM_GROUP2, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr2_port1, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.trigger_delete_all == 3)
                    {
                        printf("gia tri rear lamp port 1 group 2: %d\r\n", lamp_gr3_port1.rear);
                        for (int i = 0; i <= lamp_gr3_port1.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr3_port1.inp_lamp_arr[i];
                            lamp_gr3_port1.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr3_port1, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port1_file, ADDR_STORAGE_GROUP3_PORT1, lamp_gr3_port1);
                        sort_up_lamp_group(lamp_gr3_port1.inp_lamp_arr, lamp_gr3_port1.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr3_port1, &lamp_gr3_port2, TEMPLATE_NUMBER_DIM_GROUP3, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1, (uint8_t *)&lamp_gr3_port1, sizeof(xqueue_lamp_group_t));
                    }
                    // send_data_cpu_to_qt(queue_lamps_port1.inp_arr, queue_lamps_port1.rear, TEMPLATE_NUMBER_GROUP_PORT1, CMD_QT_TO_CPU_SET_LAMP_RS485_PORT_TO_GROUP);
                }
                else if (add_lamp_port_group.port == 2)
                {
                    lamp_rf_group.port = add_lamp_port_group.port;
                    lamp_rf_group.group = 200;
                    lamp_rf_group.dim = add.dim;

                    if (add_lamp_port_group.trigger_delete_all == 1) // group
                    {
                        printf("gia tri rear lamp port 2 group 1: %d\r\n", lamp_gr1_port2.rear);
                        for (int i = 0; i <= lamp_gr1_port2.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr1_port2.inp_lamp_arr[i];
                            lamp_gr1_port2.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr1_port2, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr1_port2_file, ADDR_STORAGE_GROUP1_PORT2, lamp_gr1_port2);
                        sort_up_lamp_group(lamp_gr1_port2.inp_lamp_arr, lamp_gr1_port2.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr1_port1, &lamp_gr1_port2, TEMPLATE_NUMBER_DIM_GROUP1, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr1_port2, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.trigger_delete_all == 2)
                    {
                        printf("gia tri rear lamp port 2 group 2: %d\r\n", lamp_gr2_port2.rear);
                        for (int i = 0; i <= lamp_gr2_port2.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr2_port2.inp_lamp_arr[i];
                            lamp_gr2_port2.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr2_port2, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr2_port2_file, ADDR_STORAGE_GROUP2_PORT2, lamp_gr2_port2);
                        sort_up_lamp_group(lamp_gr2_port2.inp_lamp_arr, lamp_gr2_port2.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr2_port1, &lamp_gr2_port2, TEMPLATE_NUMBER_DIM_GROUP2, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr2_port2, sizeof(xqueue_lamp_group_t));
                    }
                    else if (add_lamp_port_group.trigger_delete_all == 3)
                    {
                        printf("gia tri rear lamp port 2 group 2: %d\r\n", lamp_gr3_port2.rear);
                        for (int i = 0; i <= lamp_gr3_port2.rear; i++)
                        {
                            lamp_rf_group.id = lamp_gr3_port2.inp_lamp_arr[i];
                            lamp_gr3_port2.inp_lamp_arr[i] = 0;
                            descrease_queue_lamp_group(&lamp_gr3_port2, add_lamp_port_group.port);
                            send_struct(CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2, (uint8_t *)&lamp_rf_group, sizeof(change_group_lamp_t));
                            delay(500);
                        }
                        Write_struct_queue_lamps_group_toFile(queue_lamp_gr3_port2_file, ADDR_STORAGE_GROUP3_PORT2, lamp_gr3_port2);
                        sort_up_lamp_group(lamp_gr3_port2.inp_lamp_arr, lamp_gr3_port2.rear);
                        send_lamp_group_cpu_to_qt(&lamp_gr3_port2, &lamp_gr3_port2, TEMPLATE_NUMBER_DIM_GROUP3, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3);
                        send_struct(CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2, (uint8_t *)&lamp_gr3_port2, sizeof(xqueue_lamp_group_t));
                    }
                }
                break;
            case CMD_QT_TO_CPU_MONITORING_LAMP_RS485:
                printf("process cmd monitoring");
                delay(500);
                // send_data_cpu_to_qt(queue_lamps_port1.inp_arr, queue_lamps_port1.rear, TEMPLATE_NUMBER_GROUP_PORT1, CMD_QT_TO_CPU_SET_LAMP_RS485_PORT_TO_GROUP);
                delay(1000);
                // send_data_cpu_to_qt(queue_lamps_port2.inp_arr, queue_lamps_port2.rear, TEMPLATE_NUMBER_GROUP_PORT2, CMD_QT_TO_CPU_SET_LAMP_RS485_PORT2_TO_GROUP);
                break;
            case CMD_QT_TO_CPU_MONITORING_LAMP_RF:
                printf("process cmd monitoring");
                delay(500);
                // send_data_cpu_to_qt(queue.inp_arr, queue.rear, TEMPLATE_NUMBER_GROUP_DIM1, CMD_CPU_TO_QT_LAMP_RF_GROUP_DIM1);
                break;
            case CMD_QT_TO_CPU_CONFIG_POWER_LAMP:
                if (!json_object_object_get_ex(new_obj, "powerLamp", &o))
                {
                    printf("Field %s does not exist\n", "powerLamp");
                    return;
                }
                system_control_lamp.value_power = (uint8_t)json_object_get_int(o);
                printf("parsing number change power lamp: %d\r\n", system_control_lamp.value_power);
                send_struct(CMD_CPU_TO_MASTER_CHANGE_POWER_LAMP, (uint8_t *)&system_control_lamp, sizeof(changeid_t));
                break;
            case CMD_QT_TO_CPU_CHECK_LAMP_IN_GROUP:
                if (!json_object_object_get_ex(new_obj, "type", &o))
                {
                    return;
                }
                type_check_lamp_in_group = json_object_get_int(o);
                printf("got value tmp type: %d\r\n", type_check_lamp_in_group);
                if (type_check_lamp_in_group == 1)
                {
                    send_lamp_group_cpu_to_qt(&lamp_gr1_port1, &lamp_gr1_port2, TEMPLATE_NUMBER_DIM_GROUP1, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1);
                }
                else if (type_check_lamp_in_group == 2)
                {
                    send_lamp_group_cpu_to_qt(&lamp_gr2_port1, &lamp_gr2_port2, TEMPLATE_NUMBER_DIM_GROUP2, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2);
                }
                else if (type_check_lamp_in_group == 3)
                {
                    send_lamp_group_cpu_to_qt(&lamp_gr3_port1, &lamp_gr3_port2, TEMPLATE_NUMBER_DIM_GROUP3, CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3);
                }
                break;
            case CMD_QT_TO_CPU_GET_VALUE_AMBIENT_SENSOR:
                if (!json_object_object_get_ex(new_obj, "set_value_ambient_sensor", &o))
                {
                    return;
                }
                mtlc_sensor.threshold_ambient_light = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "ambient_sensor_check", &o))
                {
                    return;
                }
                mtlc_sensor.check_enable = (int)json_object_get_int(o);
                Write_struct_sensor_value_toFile(sensor_value_file, "/home/pi/sensor.txt", mtlc_sensor);
                break;
            case CMD_QT_CALL_LOAD_PARAMETERS_SENSOR:
            {
                char message1[1280];
                printf("qt yeu cau CPU cung cap param cam bien Success\r\n");
                int data_len = sprintf(message1, TEMPLATE_TIMEACTIVE_SENSOR, CMD_QT_CALL_LOAD_PARAMETERS_SENSOR, mtlc_sensor.hh_start, mtlc_sensor.mm_start, mtlc_sensor.hh_end, mtlc_sensor.mm_end, mtlc_sensor.threshold_ambient_light, mtlc_sensor.check_enable);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, message1);
                printf(message1);
                break;
            }
            case CMD_QT_TO_CPU_ACTIVE_SENSOR_AMBIENT_LIGHT:
            {
                if (!json_object_object_get_ex(new_obj, "hh_start_ss_light", &o))
                {
                    return;
                }
                mtlc_sensor.hh_start = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_start_ss_light", &o))
                {
                    return;
                }
                mtlc_sensor.mm_start = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "hh_end_ss_light", &o))
                {
                    return;
                }
                mtlc_sensor.hh_end = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "mm_end_ss_light", &o))
                {
                    return;
                }
                mtlc_sensor.mm_end = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "threshold_light", &o))
                {
                    return;
                }
                mtlc_sensor.threshold_ambient_light = (int)json_object_get_int(o);
                if (!json_object_object_get_ex(new_obj, "check", &o))
                {
                    return;
                }
                mtlc_sensor.check_enable = (int)json_object_get_int(o);
                printf("check enable: %d\r\n", mtlc_sensor.check_enable);
                send_struct(CMD_CPU_TO_MASTER_STATUS_ACTIVE_SENSOR, (uint8_t *)&mtlc_sensor, sizeof(type_mtlc_sensor_t));
                Write_struct_sensor_value_toFile(sensor_value_file, "/home/pi/sensor.txt", mtlc_sensor);
                break;
            }
            default:
                break;
            }
        }
    }
}
//**********************************************************************CAC HAM WEBSOCKET*********************************************//
#pragma endregion

#pragma region THREAD READ GPS
void *gps_process(void *threadArgs)
{
    // system("./../../GPS/gps");
    modem_get_gps();
    pthread_exit("bye");
}
#pragma endregion

#pragma region THREAD CHECK RESET THREAD
void *check_reset_thread(void *threadArgs)
{
    while(1)
    {
        if(is_check_reset_thread_modbus)
        {
            is_check_reset_thread_modbus = false;
            pthread_cancel(thread_ModBus);
            pthread_join(thread_ModBus, NULL);
            if (pthread_create(&thread_ModBus, NULL, ModBus, NULL) != 0)
            {
                printf("Failed to create thread\n");
                exit(1);
            }
            printf("Thread has been reset and restarted.\n");
        }
        sleep(5);
    }
}
#pragma endregion

void mtlc_startup_config(void)
{
    for (int i = 1; i <= 50; i++)
    {
        fb_dim_port1.group[i] = 200;
        fb_dim_port2.group[i] = 200;
    }
    fb_dim_port1.lampport = 50;
    fb_dim_port2.lampport = 50;
    queue.rear = -1;
    queue.front = -1;

    memset((uint8_t *)&sys_sensor, 0, sizeof(modbus_ss_light_t));
    memset((uint8_t *)&mtlc_sensor, 0, sizeof(type_mtlc_sensor_t));
    reset_settings_lamp_group();
}

#pragma region LUONG MAIN INIT
int main()
{
    if (wiringPiSetup() < 0)
        return 1;

    if ((fd = serialOpen("/dev/ttyAMA1", 115200)) < 0)
    {
        return 1;
    }
    delay(100);

    pinMode(relay_1, OUTPUT);
    pinMode(relay_2, OUTPUT);
    pinMode(auto_Mode, PUD_UP);
    pinMode(manual_Mode, PUD_UP);

    digitalWrite(relay_1, 1);
    digitalWrite(relay_2, 1);

    mtlc_startup_config();

    is_status_power = false;

    printf("Preperal data fist\r\n");

    time_on_off = Read_struct_time_on_off_toFile(time_on_off_file, "/home/pi/time_on_off.txt", time_on_off);
    dim_schedule_active = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_active.txt", dim_schedule_active);
    dim_schedule_group_1 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_1.txt", dim_schedule_group_1);
    dim_schedule_group_2 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
    dim_schedule_group_3 = Read_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
    counters_package_p2 = Read_struct_counter_lamp_toFile(counter_lamp_file, "/home/pi/counter_lamp.txt", counters_package_p2);

    counters_package = Read_struct_counter_lamp_toFile(counter_lamp_file, "/home/pi/counters_package.txt", counters_package);
    setting = Read_struct_setting_toFile(setting_file, "/home/pi/setting.txt", setting);

    queue = Read_struct_queue_toFile(queue_file, "/home/pi/dim_group.txt", queue);
    queue_lamps_port1 = Read_struct_queue_lamps_port1_toFile(queue_lamp_port1_file, "/home/pi/dim_group_lamp_port1.txt", queue_lamps_port1);
    queue_lamps_port2 = Read_struct_queue_lamps_port2_toFile(queue_lamp_port2_file, "/home/pi/dim_group_lamp_port2.txt", queue_lamps_port2);
    mtlc_sensor = Read_struct_sensor_value_toFile(sensor_value_file, "/home/pi/sensor.txt", mtlc_sensor);

    get_all_settings_storage_lamp_group();
    sort_down(queue.inp_arr, queue.rear);

    for (int i = 0; i <= queue.rear; i++)
    {
        printf("data: %d\r\n", queue.inp_arr[i]);
    }

    // mtlc_read_file_sensor();

    printf("setting.server is: %d\r\n", setting.server);
    printf("setting.type is: %d\r\n", setting.type);
    printf("setting.urlmqtt is: %s\r\n", setting.urlmqtt);

    system("cat /sys/class/net/eth0/address > mac.txt");
    fp = fopen("mac.txt", "r");
    if (fp == NULL)
    {
        printf("Error openfile\r\n");
        perror("Error opening file");
    }
    if (fgets(mac, 18, fp) != NULL)
    {
        /* writing content to stdout */
    }
    /* Closing the file mac.txt */
    fclose(fp);
    printf("Mac CM4: %s\r\n", mac);

    // printf("MAC: %s\r\n",mac);
    substr(mac, 6, 16);

    asprintf(&mac_id_gateway, "GW-%s", re_mac);
    // printf("MAC ID: %s\r\n", mac_id_gateway);
    char *token = NULL;
    const char s[2] = ":";
    token = strtok(mac_id_gateway, s);
    mac_final = token;
    // strcat(mac_final, token);
    token = strtok(NULL, s);
    strcat(mac_final, token);
    token = strtok(NULL, s);
    strcat(mac_final, token);
    token = strtok(NULL, s);
    strcat(mac_final, token);
    printf("Mac ID: %s\r\n", mac_final);
    // -------------------------------------------Tao luong UART------------------------------------------------------------------------------------//
    if (pthread_create(&thread_Serial, NULL, Serial, NULL) != 0)
    {
        printf("thread_create() failed\n");
        return 1;
    }
    if (setting.server)
    {
        if (pthread_create(&thread_Mqtt, NULL, Mqtt, NULL) != 0)
        {
            printf("thread_create() failed\n");
            return 1;
        }
    }
    if (pthread_create(&thread_ModBus, NULL, ModBus, NULL) != 0)
    {
#ifdef DEBUG_MODBUS
        printf("thread_create() failed\n");
#endif
        return 1;
    }

    if (pthread_create(&thread_Main, NULL, main_process, NULL) != 0)
    {
#ifdef DEBUG_MAIN
        printf("thread_create main process() failed\n");
#endif
        return 1;
    }

    if (pthread_create(&thread_send_parameter_to_Monitor, NULL, send_parameter_to_Monitor, NULL) != 0)
    {
#ifdef DEBUG_MAIN
        printf("thread_create main process() failed\n");
#endif
        return 1;
    }

    if (pthread_create(&thread_GPS, NULL, gps_process, NULL) != 0)
    {
#ifdef DEBUG_MAIN
        printf("thread_create main gps_process() failed\n");
#endif
        return 1;
    }

    if (pthread_create(&thread_registor_modbus_sensor, NULL, Register_Modbus_Sensor, NULL) != 0)
    {
#ifdef DEBUG_MAIN
        printf("thread_create main Register_Modbus_Sensor() failed\n");
#endif
        return 1;
    }

    if (pthread_create(&thread_registor_check_reset, NULL, check_reset_thread, NULL) != 0)
    {
#ifdef DEBUG_MAIN
        printf("failed to create thread check reset.\n");
#endif
        return 1;
    }
    struct ev_loop *loop = EV_DEFAULT;
    while (1)
    {
        // Thread websocket client
        uwsc_log_info("Libuwsc: %s\n", UWSC_VERSION_STRING);
        cl = uwsc_new(loop, url, ping_interval, NULL);

        if (!cl)
        {
            printf("Create new socket fail\r\n");
            continue;
        }
        else
        {
            printf("Create new socket success\r\n");
        }

        uwsc_log_info("Start connect...\n");

        cl->onopen = uwsc_onopen;
        cl->onmessage = uwsc_onmessage;
        cl->onerror = uwsc_onerror;
        cl->onclose = uwsc_onclose;

        ev_signal_init(&signal_watcher, signal_cb, SIGINT);
        ev_signal_start(loop, &signal_watcher);

        ev_run(loop, 0);
        free(cl);

        return 0;
    }
    exit(1);
}
#pragma endregion

#pragma region LUONG GUI DU LIEU LEN MAN HINH
void *send_parameter_to_Monitor(void *threadArgs)
{
    volatile bool is_first_send_status_pwr = false;
    volatile bool is_first_send_time_active = false;
    volatile bool is_first_send_dim_schedule_active = false;
    volatile bool is_first_send_connect = false;
    volatile bool is_first_send_data_server = false;
    volatile bool is_first_send_sensor_active = false;

    while (1)
    {
        delay(1);
        if (is_flag_opened_websocket)
        {
            if (!is_first_send_connect)
            {
                char data[1000] = {0};
                int data_len = sprintf(data, "CPU-{\"CMD\": %d,\"connect\":%d}", CMD_CPU_TO_QT_CONNECTION, 0);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                is_first_send_connect = true;
            }
            if (!is_first_send_time_active)
            {
                is_first_send_time_active = true;
                char data[2000] = {0};
                char message1[1280] = {0};
                char message2[1280] = {0};

                if (time_on_off.size_array_on_off_schedule == 1)
                {
                    snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                             time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
                    // data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}",CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                }
                else
                {
                    for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
                    {
                        if (i == 0)
                        {
                            snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        else
                        {
                            snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d, \"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                                     time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                        }
                        strcat(message1, message2);
                        printf("message1: %s\r\n", message1);
                    }
                }
                int data_len = sprintf(data, "CPU-{\"CMD\": %d,\"time\":[%s]}", CMD_CPU_TO_QT_TIME_ACTIVE, message1);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
            }
            if (!is_first_send_dim_schedule_active)
            {
                is_first_send_dim_schedule_active = true;
                char data[1000] = {0};
                int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, 200,
                                       dim_schedule_active.dim1,
                                       dim_schedule_active.dim2,
                                       dim_schedule_active.dim3,
                                       dim_schedule_active.dim4,
                                       dim_schedule_active.dim5,
                                       dim_schedule_active.dim6,
                                       dim_schedule_active.dim7,
                                       dim_schedule_active.dim8,
                                       dim_schedule_active.dim9,
                                       dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1,
                                       dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2,
                                       dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3,
                                       dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4,
                                       dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5,
                                       dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6,
                                       dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7,
                                       dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8,
                                       dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
            }
            if (!is_first_send_status_pwr)
            {
                is_first_send_status_pwr = true;
                char data[1000] = {0};
                int data_len = sprintf(data, TEMPLATE_STATUS_PWR,
                                       CMD_CPU_TO_QT_STATUS_PWR, is_status_power);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                delay(1000);
            }
            if (!is_first_send_sensor_active)
            {
                is_first_send_sensor_active = true;
                char data[1000] = {0};
                int data_len = sprintf(data, TEMPLATE_SENSOR_ON_DASHBOARD, CMD_CPU_TO_QT_SENSOR_ON_DASHBOARD, mtlc_sensor.hh_start, mtlc_sensor.mm_start, mtlc_sensor.hh_end, mtlc_sensor.mm_end, mtlc_sensor.check_enable, mtlc_sensor.threshold_ambient_light);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
                delay(1000);
            }
            if ((mode_pre == mode_active))
            {
                continue;
            }
            else
            {
                mode_pre = mode_active;
                char data_mode_active[1000] = {0};
                int data_mode_active_len = sprintf(data_mode_active, TEMPLATE_MODE,
                                                   CMD_CPU_TO_QT_MODE, mode_pre);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);
                memset(data_mode_active, 0, 1000);
                data_mode_active_len = sprintf(data_mode_active, TEMPLATE_STATUS_PWR,
                                               CMD_CPU_TO_QT_STATUS_PWR, is_status_power);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_mode_active_len, data_mode_active);

                delay(1000);
            }
            if (!is_first_send_data_server)
            {
                is_first_send_data_server = true;
                char data_server[1000] = {0};
                int data_len = sprintf(data_server, TEMPLATE_SETTING, CMD_CPU_TO_QT_OPEN_SERVER, setting.server, setting.type);
                cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data_server);
                memset(data_server, 0, 1000);
            }
        }
    }
}
#pragma endregion

#pragma region HAM NHAN type_mtfc_working_message UART
void serial_get_buffer(void)
{
    // printf("Has a data\r\n");
    uint8_t c = 0;
    if (serialDataAvail(fd))
    {
        c = (uint8_t)(serialGetchar(fd));
        // cout << "This is check: " << c <<endl;
        if (c_state == IDLE)
        {
            c_state = (c == '$') ? HEADER_START : IDLE;

            if (c_state == IDLE)
            {
            }
        }
        else if (c_state == HEADER_START)
        {
            c_state = (c == 'M') ? HEADER_M : IDLE;
        }
        else if (c_state == HEADER_M)
        {
            c_state = (c == '>') ? HEADER_ARROW : IDLE;
        }
        else if (c_state == HEADER_ARROW)
        {
            if (c > 255)
            {
                c_state = IDLE;
            }
            else
            {
                dataSize = c;
                offset = 0;
                checksum = 0;
                indRX = 0;
                checksum ^= c;
                c_state = HEADER_SIZE;
            }
        }
        else if (c_state == HEADER_SIZE)
        {
            cmdMSP = c;
            checksum ^= c;
            c_state = HEADER_CMD;
        }
        else if (c_state == HEADER_CMD && offset < dataSize)
        {
            checksum ^= c;
            inBuf[offset++] = c;
        }
        else if (c_state == HEADER_CMD && offset >= dataSize)
        {
            if (checksum == c)
            {
                // printf("Has a data\r\n");
                get_data_finish();
            }
            c_state = IDLE;
        }
    }
}
#pragma endregion

#pragma region HAM XU LY type_mtfc_working_message UART
void get_data_finish(void)
{
    type_lightAddConfig_t tt;
    printf("Has a data %d\r\n", cmdMSP);
    switch (cmdMSP)
    {
    case 240:
        readstruct((uint8_t *)&dim_active, sizeof(dim_active_t));
        printf("Dim is %d\n", dim_active.dim);
        break;
    case 241:
        readstruct((uint8_t *)&tt, sizeof(type_lightAddConfig_t));
        printf("ID is %d\n", tt.add);
        printf("Group is %d\n", tt.group);
        break;
    case 219:
        readstruct((uint8_t *)&add, sizeof(add_t));
        printf("add is %d\n", add.id);
        printf("dim is %d\n", add.dim);
        printf("port is %d\n", add.port);
        printf("port is %d\n", add.group);
        if (add.port == 1)
        {
            fb_dim_port1.group[add.id] = 200;
            fb_dim_port1.dim_fb[add.id] = add.dim;
        }
        if (add.port == 2)
        {
            fb_dim_port2.group[add.id] = 200;
            fb_dim_port2.dim_fb[add.id] = add.dim;
        }
        flag_dimstatus = 1;
        break;
    case CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1:
        readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
        printf("counters_package port 2 is %d\n", counters_package.val1);
        Write_struct_counter_lamp_toFile(counter_lamp_file, "/home/pi/counter_lamp.txt", counters_package);
        char data2[1000] = {0};
        int data_len2 = sprintf(data2, TEMPLATE_COUNTER_LAMP2, 244,
                                counters_package.val1);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len2, data2);
        flag_inforstatus = 1;
        break;
    case CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2:
        readstruct((uint8_t *)&counters_package_p2, sizeof(type_configPackage_t));
        printf("counters_package port 1 is %d\n", counters_package_p2.val1);
        // printf("Da doc gia tri p1 %d\n", counters_package.val1);
        // counters_package.val2 = counters_package_p2.val1;
        Write_struct_counter_lamp_toFile(counter_lamp_file, "/home/pi/counters_package.txt", counters_package_p2);
        printf("Da luu gia tri p1 %d\n", counters_package_p2.val1);
        // printf("Da luu gia tri p2 %d\n", counters_package.val2);
        char data[1000] = {0};
        int data_len = sprintf(data, TEMPLATE_COUNTER_LAMP, 243,
                               counters_package_p2.val1, counters_package_p2.val2);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
        flag_inforstatus = 1;
        break;
    case CMD_MASTER_TO_CPU_CHANGE_ID_RF_FB:
    {
        char data3[1000] = {0};
        int data_len3 = sprintf(data3, TEMPLATE_NOTIFICATION, CMD_CPU_TO_QT_CHANGE_ID_RF_FB,
                                "\"Change id success!\"");
        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len3, data3);
        printf("Da send den qt ok %s\n", data3);
    }
    break;
    case CMD_MASTER_TO_CPU_COUNT_LAMP_RF_FB:
    {
        readstruct((uint8_t *)&add_fb, sizeof(add_t));
        printf("ID %d\n", add_fb.id);
        printf("dim %d\n", add_fb.dim);
        if (add_fb.port == 1)
        {
            fb_dim_port1.group[add_fb.id] = 200;
            fb_dim_port1.dim_fb[add_fb.id] = add_fb.dim;
            count_lamp_rf++;
        }
        if (add_fb.port == 2)
        {
            fb_dim_port2.group[add.id] = 200;
            fb_dim_port2.dim_fb[add.id] = add.dim;
        }
        flag_dimstatus = 1;
        char data4[1000] = {0};
        int data_len4 = sprintf(data4, TEMPLATE_COUNTER_LAMP, 243,
                                count_lamp_rf, count_lamp_rf);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len4, data4);
    }
    break;
    case CMD_MASTER_CPU_TIME_CURRENT:
    {
        readstruct((uint8_t *)&current_time_master, sizeof(current_time_t));
        char tmp_data[100] = {0};
        int tmp_data_len = sprintf(tmp_data, TEMPLATE_TIME_MASTER_MONITOR, CMD_CPU_TO_QT_TIME_MONITORING_FROM_MASTER, current_time_master.hh_current, current_time_master.mm_current);
        printf("Time master monitor: %s\r\n", tmp_data);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, tmp_data_len, tmp_data);
        memset(tmp_data, 0x00, sizeof(tmp_data));
    }
    break;
    case CMD_MASTER_TO_CPU_SYN_RTC_TIME:
    {
        printf("Nhan yeu cau dong bo lai thoi gian\r\n");
        readstruct((uint8_t *)&cpu_update_rtc, sizeof(cpu_require_update_rtc_t));
        printf("nhan yeu cau dong bo: %d\r\n", cpu_update_rtc.param);
    }
    break;
    case CMD_MASTER_TO_CPU_STATUS_FORCE_CABINET:
    {
        printf("Force Cabinet Success\n");
        readstruct((uint8_t *)&mtlc_system_response, sizeof(type_mtlc_data_response_t));
        printf("data status force cabinet: %d\r\n", mtlc_system_response.force_cabinet_status);
    }
    break;
    default:
        break;
    }
}

void publish_callback(void **unused, struct mqtt_response_publish *published)
{
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char *topic_name = (char *)malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';

    printf("Received publish('%s'): %s\n", topic_name, (const char *)published->application_message);
    char *temp = strtok(topic_name, "/");
    temp = strtok(NULL, "\n");
    printf("[%.*s]\n", strlen(temp), (char *)temp);
    char *toppic = strtok(temp, "/");
    toppic = strtok(NULL, "\n");
    printf("[%.*s]\n", strlen(toppic), (char *)toppic);
    printf("Parsed toppic is: %s\n", toppic);
    int i, j;
    void *data = (char *)published->application_message;

    if (!strncmp(toppic, "power-control", 13))
    {
        printf("reach here \r\n");
        char *temp = strtok(data, ":");
        temp = strtok(NULL, "\n");
        printf("[%.*s]\n", strlen(temp), (char *)temp);
        struct json_object *new_obj;
        new_obj = json_tokener_parse(temp);
        printf("Parsed is: %s\n", temp);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }

        power_fb = atoi(temp);
        // send_byte(CMD_CPU_TO_MASTER_POWER_CONTROL, power_fb);
        send_byte(99, 0);
        flag_control.flag_power = 1;
    }

    if (!strncmp(toppic, "information", 11))
    {
        printf("reach here \r\n");
        printf("Bat dau dem so luong den\r\n");
        send_struct(CMD_CPU_TO_MASTER_COUNT_LAMP, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
        char *temp = strtok(data, ":");
        temp = strtok(NULL, "\n");
        printf("[%.*s]\n", strlen(temp), (char *)temp);
        struct json_object *new_obj;
        new_obj = json_tokener_parse(temp);
        printf("Parsed is: %s\n", temp);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }
        flag_control.flag_info = 1;
    }

    if (!strncmp(toppic, "group-setting", 13))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *tmpQueries, *resultsObj, *tmpResults, *resultsObjGroup;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }
        struct json_object *o;
        if (!json_object_object_get_ex(new_obj, "port1", &o))
        {
            printf("Field %s does not exist\n", "port1");
            return;
        }
        printf("\n");
        printf("Port1:\n\n");
        for (i = 0; i < json_object_array_length(o); i++)
        {

            tmpQueries = json_object_array_get_idx(o, i);
            if (!json_object_object_get_ex(tmpQueries, "groupId", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "groupId");
                return;
            }
            printf("Group:[%d] = %s \n", i, json_object_to_json_string(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "groupDevice", &resultsObj))
            {
                printf("Field %s does not exist\n", "groupDevice");
                return;
            }
            for (j = 0; j < json_object_array_length(resultsObj); j++)
            {
                tmpResults = json_object_array_get_idx(resultsObj, j);
                printf("LampID:[%d] = %s \n", j, json_object_to_json_string(tmpResults));
            }
        }
        if (!json_object_object_get_ex(new_obj, "port2", &o))
        {
            printf("Field %s does not exist\n", "port1");
            return;
        }
        printf("\n");
        printf("Port2:\n\n");

        for (i = 0; i < json_object_array_length(o); i++)
        {
            tmpQueries = json_object_array_get_idx(o, i);
            if (!json_object_object_get_ex(tmpQueries, "groupId", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "groupId");
                return;
            }
            printf("Group:[%d] = %s \n", i, json_object_to_json_string(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "groupDevice", &resultsObj))
            {
                printf("Field %s does not exist\n", "groupDevice");
                return;
            }
            for (j = 0; j < json_object_array_length(resultsObj); j++)
            {
                tmpResults = json_object_array_get_idx(resultsObj, j);
                printf("LampID:[%d] = %s \n", j, json_object_to_json_string(tmpResults));
            }
        }
        flag_control.flag_setgroup = 1;
    }
    if (!strncmp(toppic, "power-schedule-setting", 22))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *tmpQueries, *resultsObjGroup;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }
        /* Loop through array of queries */
        for (i = 0; i < json_object_array_length(new_obj); i++)
        {

            tmpQueries = json_object_array_get_idx(new_obj, i);
            if (!json_object_object_get_ex(tmpQueries, "hh_start", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "hh_start");
                return;
            }

            time_on_off.hh_start[i] = json_object_get_int(resultsObjGroup);
            printf("hh_start:[%d] = %d \n", i, json_object_get_int(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "mm_start", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "mm_start");
                return;
            }

            time_on_off.mm_start[i] = json_object_get_int(resultsObjGroup);
            printf("mm_start:[%d] = %d \n", i, json_object_get_int(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "hh_end", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "hh_end");
                return;
            }

            time_on_off.hh_end[i] = json_object_get_int(resultsObjGroup);
            printf("hh_end:[%d] = %d \n", i, json_object_get_int(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "mm_end", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "mm_end");
                return;
            }

            time_on_off.mm_end[i] = json_object_get_int(resultsObjGroup);
            printf("mm_end:[%d] = %d \n", i, time_on_off.mm_end[i]);
        }
        time_on_off.size_array_on_off_schedule = i;
        // Write_struct_time_active_toFile(time_active_file, "/home/pi/time_active.txt", time_active);
        Write_struct_time_on_off_toFile(time_on_off_file, "/home/pi/time_on_off.txt", time_on_off);
        send_struct(CMD_CPU_TO_MASTER_SCHEDULE_POWER, (uint8_t *)&time_on_off, sizeof(time_on_off_t));

        char data_send_active_time[2000];
        char message1[1280] = {0};
        char message2[1280] = {0};

        if (time_on_off.size_array_on_off_schedule == 1)
        {
            snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d,\"check\":%d}", time_on_off.hh_start[0], time_on_off.mm_start[0],
                     time_on_off.hh_end[0], time_on_off.mm_end[0], 1);
            // data_mode_active_len = sprintf(message3, "CPU-{\"CMD\": %d,\"time\":[%s]}",CMD_CPU_TO_QT_TIME_ACTIVE, message1);
        }
        else
        {
            for (int i = 0; i < time_on_off.size_array_on_off_schedule; i++)
            {
                if (i == 0)
                {
                    snprintf(message1, sizeof(message1), "{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d,\"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                             time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                }
                else
                {
                    snprintf(message2, sizeof(message2), ",{\"hh_start\":%d,\"mm_start\":%d,\"hh_end\":%d,\"mm_end\":%d,\"check\":%d}", time_on_off.hh_start[i], time_on_off.mm_start[i],
                             time_on_off.hh_end[i], time_on_off.mm_end[i], 1);
                }
                strcat(message1, message2);
                printf("message1: %s\r\n", message1);
            }
        }
        int data_send_active_time_len = sprintf(data_send_active_time, "CPU-{\"CMD\": %d,\"time\":[%s]}", CMD_CPU_TO_QT_TIME_ACTIVE, message1);
        printf("\time is: %s", data_send_active_time);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_send_active_time_len, data_send_active_time);

        printf("\nsend qt ok\r\n");
        flag_control.flag_setgroupschedule = 1;
    }
    if (!strncmp(toppic, "light-schedule-setting", 22))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *tmpQueries, *resultsObjGroup;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }
        struct json_object *o;
        if (!json_object_object_get_ex(new_obj, "group", &o))
        {
            printf("Field %s does not exist\n", "group");
            return;
        }
        printf("\n");
        printf("group: %d\n\n", json_object_get_int(o));
        /* Loop through array of queries */
        if (!json_object_object_get_ex(new_obj, "schedule", &o))
        {
            printf("Field %s does not exist\n", "schedule");
            return;
        }
        for (i = 0; i < json_object_array_length(o); i++)
        {

            tmpQueries = json_object_array_get_idx(o, i);
            if (!json_object_object_get_ex(tmpQueries, "hh_start_dim", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "hh_start_dim");
                return;
            }
            if (i == 0)
            {
                dim_schedule_active.hh_start_dim1 = json_object_get_int(resultsObjGroup);
            }
            if (i == 1)
            {
                dim_schedule_active.hh_start_dim2 = json_object_get_int(resultsObjGroup);
            }
            if (i == 2)
            {
                dim_schedule_active.hh_start_dim3 = json_object_get_int(resultsObjGroup);
            }
            if (i == 3)
            {
                dim_schedule_active.hh_start_dim4 = json_object_get_int(resultsObjGroup);
            }
            if (i == 4)
            {
                dim_schedule_active.hh_start_dim5 = json_object_get_int(resultsObjGroup);
            }
            if (i == 5)
            {
                dim_schedule_active.hh_start_dim6 = json_object_get_int(resultsObjGroup);
            }
            if (i == 6)
            {
                dim_schedule_active.hh_start_dim7 = json_object_get_int(resultsObjGroup);
            }
            if (i == 7)
            {
                dim_schedule_active.hh_start_dim8 = json_object_get_int(resultsObjGroup);
            }
            if (i == 8)
            {
                dim_schedule_active.hh_start_dim9 = json_object_get_int(resultsObjGroup);
            }
            printf("hh_start_dim:[%d] = %s \n", i, json_object_to_json_string(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "mm_start_dim", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "mm_start_dim");
                return;
            }
            if (i == 0)
            {
                dim_schedule_active.mm_start_dim1 = json_object_get_int(resultsObjGroup);
            }
            if (i == 1)
            {
                dim_schedule_active.mm_start_dim2 = json_object_get_int(resultsObjGroup);
            }
            if (i == 2)
            {
                dim_schedule_active.mm_start_dim3 = json_object_get_int(resultsObjGroup);
            }
            if (i == 3)
            {
                dim_schedule_active.mm_start_dim4 = json_object_get_int(resultsObjGroup);
            }
            if (i == 4)
            {
                dim_schedule_active.mm_start_dim5 = json_object_get_int(resultsObjGroup);
            }
            if (i == 5)
            {
                dim_schedule_active.mm_start_dim6 = json_object_get_int(resultsObjGroup);
            }
            if (i == 6)
            {
                dim_schedule_active.mm_start_dim7 = json_object_get_int(resultsObjGroup);
            }
            if (i == 7)
            {
                dim_schedule_active.mm_start_dim8 = json_object_get_int(resultsObjGroup);
            }
            if (i == 8)
            {
                dim_schedule_active.mm_start_dim9 = json_object_get_int(resultsObjGroup);
            }
            printf("mm_start_dim:[%d] = %s \n", i, json_object_to_json_string(resultsObjGroup));
            if (!json_object_object_get_ex(tmpQueries, "dimmer", &resultsObjGroup))
            {
                printf("Field %s does not exist\n", "dimmer");
                return;
            }
            if (i == 0)
            {
                dim_schedule_active.dim1 = json_object_get_int(resultsObjGroup);
            }
            if (i == 1)
            {
                dim_schedule_active.dim2 = json_object_get_int(resultsObjGroup);
            }
            if (i == 2)
            {
                dim_schedule_active.dim3 = json_object_get_int(resultsObjGroup);
            }
            if (i == 3)
            {
                dim_schedule_active.dim4 = json_object_get_int(resultsObjGroup);
            }
            if (i == 4)
            {
                dim_schedule_active.dim5 = json_object_get_int(resultsObjGroup);
            }
            if (i == 5)
            {
                dim_schedule_active.dim6 = json_object_get_int(resultsObjGroup);
            }
            if (i == 6)
            {
                dim_schedule_active.dim7 = json_object_get_int(resultsObjGroup);
            }
            if (i == 7)
            {
                dim_schedule_active.dim8 = json_object_get_int(resultsObjGroup);
            }
            if (i == 8)
            {
                dim_schedule_active.dim9 = json_object_get_int(resultsObjGroup);
            }
            printf("dimmer:[%d] = %s \n", i, json_object_to_json_string(resultsObjGroup));

            Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_active.txt", dim_schedule_active);
            send_struct(CMD_CPU_TO_MASTER_SETTING_SCHEDULE_DIM_ALL_CONTROL, (uint8_t *)&dim_schedule_active, sizeof(dim_active_schedule_t));

            int data_len = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, 200,
                                   dim_schedule_active.dim1,
                                   dim_schedule_active.dim2,
                                   dim_schedule_active.dim3,
                                   dim_schedule_active.dim4,
                                   dim_schedule_active.dim5,
                                   dim_schedule_active.dim6,
                                   dim_schedule_active.dim7,
                                   dim_schedule_active.dim8,
                                   dim_schedule_active.dim9,
                                   dim_schedule_active.hh_start_dim1, dim_schedule_active.mm_start_dim1,
                                   dim_schedule_active.hh_start_dim2, dim_schedule_active.mm_start_dim2,
                                   dim_schedule_active.hh_start_dim3, dim_schedule_active.mm_start_dim3,
                                   dim_schedule_active.hh_start_dim4, dim_schedule_active.mm_start_dim4,
                                   dim_schedule_active.hh_start_dim5, dim_schedule_active.mm_start_dim5,
                                   dim_schedule_active.hh_start_dim6, dim_schedule_active.mm_start_dim6,
                                   dim_schedule_active.hh_start_dim7, dim_schedule_active.mm_start_dim7,
                                   dim_schedule_active.hh_start_dim8, dim_schedule_active.mm_start_dim8,
                                   dim_schedule_active.hh_start_dim9, dim_schedule_active.mm_start_dim9);

            cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len, data);
            printf("schedule dim all setting from app sucess\r\n");
        }
        flag_control.flag_lightschedulesetting = 1;
    }
    if (!strncmp(toppic, "onlyone-schedule-setting", 24))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *tmpQueries, *o;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }
        for (i = 0; i < json_object_array_length(new_obj); i++)
        {

            tmpQueries = json_object_array_get_idx(new_obj, i);
            if (!json_object_object_get_ex(tmpQueries, "port", &o))
            {
                printf("Field %s does not exist\n", "port");
                return;
            }
            add.port = (uint8_t)json_object_get_int(o);
            printf("port: %d\n\n", json_object_get_int(o));
            /* Loop through array of queries */
            if (!json_object_object_get_ex(tmpQueries, "light", &o))
            {
                printf("Field %s does not exist\n", "light");
                return;
            }
            add.id = (uint8_t)json_object_get_int(o);
            printf("id: %d\n\n", json_object_get_int(o));
            if (!json_object_object_get_ex(tmpQueries, "dimmer", &o))
            {
                printf("Field %s does not exist\n", "dimmer");
                return;
            }
            add.dim = (uint8_t)json_object_get_int(o);
            printf("dim: %d\n\n", json_object_get_int(o));
            if (!json_object_object_get_ex(tmpQueries, "group", &o))
            {
                printf("Field %s does not exist\n", "dimmer");
                return;
            }
            add.group = (uint8_t)json_object_get_int(o);
            printf("group: %d\n\n", json_object_get_int(o));
            if (add.port == 1)
            {
                fb_dim_port1.group[add.id] = add.group;
                fb_dim_port1.dim_fb[add.id] = add.dim;
            }
            if (add.port == 2)
            {
                fb_dim_port2.group[add.id] = add.group;
                fb_dim_port2.dim_fb[add.id] = add.dim;
            }
            send_struct(CMD_CPU_TO_MASTER_DIM_ID, (uint8_t *)&add, sizeof(add_t));
            printf("Da send den master ok\r\n");
            delay(100);
        }
        flag_control.flag_dimid = 1;
    }
    if (!strncmp(toppic, "group-schedule-setting", 22))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *tmpQueries, *resultsObj, *o;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }

        if (!json_object_object_get_ex(new_obj, "onOff", &o))
        {
            printf("Field %s does not exist\n", "onOff");
            return;
        }
        check_onoff = json_object_get_int(o);
        printf("Mode: %d\n\n", json_object_get_int(o));
        printf("check_onoff: %d\n\n", check_onoff);

        if (!json_object_object_get_ex(new_obj, "group", &o))
        {
            printf("Field %s does not exist\n", "group");
            return;
        }
        check_group = json_object_get_int(o);
        printf("Group: %d\n\n", json_object_get_int(o));
        printf("check_group: %d\n\n", check_group);

        if (!json_object_object_get_ex(new_obj, "port", &o))
        {
            printf("Field %s does not exist\n", "port");
            return;
        }
        check_port = json_object_get_int(o);
        printf("Port: %d\n\n", json_object_get_int(o));
        printf("check_port: %d\n\n", check_port);
        if (!json_object_object_get_ex(new_obj, "schedule", &o))
        {
            printf("Field %s does not exist\n", "schedule");
            return;
        }
        for (i = 0; i < json_object_array_length(o); i++)
        {
            tmpQueries = json_object_array_get_idx(o, i);
            if (!json_object_object_get_ex(tmpQueries, "hh_start_dim", &resultsObj))
            {
                printf("Field %s does not exist\n", "hh_start_dim");
                return;
            }
            if (check_port == 1)
            {
                if (check_group == 1)
                {
                    if (check_onoff == 0)
                        dim_schedule_group_1.group = 99;
                    else
                        dim_schedule_group_1.group = 1;
                    if (i == 0)
                    {
                        dim_schedule_group_1.hh_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_1.hh_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_1.hh_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_1.hh_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_1.hh_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_1.hh_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_1.hh_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_1.hh_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_1.hh_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
                if (check_group == 2)
                {
                    if (check_onoff == 0)
                        dim_schedule_group_2.group = 98;
                    else
                        dim_schedule_group_2.group = 2;
                    if (i == 0)
                    {
                        dim_schedule_group_2.hh_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_2.hh_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_2.hh_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_2.hh_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_2.hh_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_2.hh_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_2.hh_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_2.hh_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_2.hh_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
            }
            if (check_port == 2)
            {
                if (check_group == 1)
                {
                    if (check_onoff == 0)
                        dim_schedule_group_3.group = 97;
                    else
                        dim_schedule_group_3.group = 3;
                    if (i == 0)
                    {
                        dim_schedule_group_3.hh_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_3.hh_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_3.hh_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_3.hh_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_3.hh_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_3.hh_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_3.hh_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_3.hh_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_3.hh_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
                // if (check_group == 2)
                // {
                // if(i==0){dim_schedule_group_2.hh_start_dim1 = json_object_get_int( resultsObj );}
                // if(i==1){dim_schedule_group_2.hh_start_dim2 = json_object_get_int( resultsObj );}
                // if(i==2){dim_schedule_group_2.hh_start_dim3 = json_object_get_int( resultsObj );}
                // if(i==3){dim_schedule_group_2.hh_start_dim4 = json_object_get_int( resultsObj );}
                // if(i==4){dim_schedule_group_2.hh_start_dim5 = json_object_get_int( resultsObj );}
                // if(i==5){dim_schedule_group_2.hh_start_dim6 = json_object_get_int( resultsObj );}
                // if(i==6){dim_schedule_group_2.hh_start_dim7 = json_object_get_int( resultsObj );}
                // if(i==7){dim_schedule_group_2.hh_start_dim8 = json_object_get_int( resultsObj );}
                // if(i==8){dim_schedule_group_2.hh_start_dim9 = json_object_get_int( resultsObj );}
                // }
            }
            printf("hh_start_dim:[%d] = %s \n", i, json_object_to_json_string(resultsObj));
            if (!json_object_object_get_ex(tmpQueries, "mm_start_dim", &resultsObj))
            {
                printf("Field %s does not exist\n", "mm_start_dim");
                return;
            }
            if (check_port == 1)
            {
                if (check_group == 1)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_1.mm_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_1.mm_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_1.mm_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_1.mm_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_1.mm_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_1.mm_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_1.mm_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_1.mm_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_1.mm_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
                if (check_group == 2)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_2.mm_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_2.mm_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_2.mm_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_2.mm_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_2.mm_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_2.mm_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_2.mm_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_2.mm_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_2.mm_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
            }
            if (check_port == 2)
            {
                if (check_group == 1)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_3.mm_start_dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_3.mm_start_dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_3.mm_start_dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_3.mm_start_dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_3.mm_start_dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_3.mm_start_dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_3.mm_start_dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_3.mm_start_dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_3.mm_start_dim9 = json_object_get_int(resultsObj);
                    }
                }
                // if (check_group == 2)
                // {
                // if(i==0){dim_schedule_group_2.mm_start_dim1 = json_object_get_int( resultsObj );}
                // if(i==1){dim_schedule_group_2.mm_start_dim2 = json_object_get_int( resultsObj );}
                // if(i==2){dim_schedule_group_2.mm_start_dim3 = json_object_get_int( resultsObj );}
                // if(i==3){dim_schedule_group_2.mm_start_dim4 = json_object_get_int( resultsObj );}
                // if(i==4){dim_schedule_group_2.mm_start_dim5 = json_object_get_int( resultsObj );}
                // if(i==5){dim_schedule_group_2.mm_start_dim6 = json_object_get_int( resultsObj );}
                // if(i==6){dim_schedule_group_2.mm_start_dim7 = json_object_get_int( resultsObj );}
                // if(i==7){dim_schedule_group_2.mm_start_dim8 = json_object_get_int( resultsObj );}
                // if(i==8){dim_schedule_group_2.mm_start_dim9 = json_object_get_int( resultsObj );}
                // }
            }
            printf("mm_start_dim:[%d] = %s \n", i, json_object_to_json_string(resultsObj));
            if (!json_object_object_get_ex(tmpQueries, "dimmer", &resultsObj))
            {
                printf("Field %s does not exist\n", "dimmer");
                return;
            }
            if (check_port == 1)
            {
                if (check_group == 1)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_1.dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_1.dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_1.dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_1.dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_1.dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_1.dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_1.dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_1.dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_1.dim9 = json_object_get_int(resultsObj);
                    }
                }
                if (check_group == 2)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_2.dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_2.dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_2.dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_2.dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_2.dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_2.dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_2.dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_2.dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_2.dim9 = json_object_get_int(resultsObj);
                    }
                }
            }
            if (check_port == 2)
            {
                if (check_group == 1)
                {
                    if (i == 0)
                    {
                        dim_schedule_group_3.dim1 = json_object_get_int(resultsObj);
                    }
                    if (i == 1)
                    {
                        dim_schedule_group_3.dim2 = json_object_get_int(resultsObj);
                    }
                    if (i == 2)
                    {
                        dim_schedule_group_3.dim3 = json_object_get_int(resultsObj);
                    }
                    if (i == 3)
                    {
                        dim_schedule_group_3.dim4 = json_object_get_int(resultsObj);
                    }
                    if (i == 4)
                    {
                        dim_schedule_group_3.dim5 = json_object_get_int(resultsObj);
                    }
                    if (i == 5)
                    {
                        dim_schedule_group_3.dim6 = json_object_get_int(resultsObj);
                    }
                    if (i == 6)
                    {
                        dim_schedule_group_3.dim7 = json_object_get_int(resultsObj);
                    }
                    if (i == 7)
                    {
                        dim_schedule_group_3.dim8 = json_object_get_int(resultsObj);
                    }
                    if (i == 8)
                    {
                        dim_schedule_group_3.dim9 = json_object_get_int(resultsObj);
                    }
                }
                // if (check_group == 2)
                // {
                // if(i==0){dim_schedule_group_2.dim1 = json_object_get_int( resultsObj );}
                // if(i==1){dim_schedule_group_2.dim2 = json_object_get_int( resultsObj );}
                // if(i==2){dim_schedule_group_2.dim3 = json_object_get_int( resultsObj );}
                // if(i==3){dim_schedule_group_2.dim4 = json_object_get_int( resultsObj );}
                // if(i==4){dim_schedule_group_2.dim5 = json_object_get_int( resultsObj );}
                // if(i==5){dim_schedule_group_2.dim6 = json_object_get_int( resultsObj );}
                // if(i==6){dim_schedule_group_2.dim7 = json_object_get_int( resultsObj );}
                // if(i==7){dim_schedule_group_2.dim8 = json_object_get_int( resultsObj );}
                // if(i==8){dim_schedule_group_2.dim9 = json_object_get_int( resultsObj );}
                // }
            }
            printf("dimmer:[%d] = %s \n", i, json_object_to_json_string(resultsObj));
        }
        Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_1.txt", dim_schedule_group_1);
        Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_2.txt", dim_schedule_group_2);
        Write_struct_dim_schedule_active_toFile(dim_schedule_active_file, "/home/pi/dim_schedule_group_3.txt", dim_schedule_group_3);
        int data_len1 = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_1.group,
                                dim_schedule_group_1.dim1,
                                dim_schedule_group_1.dim2,
                                dim_schedule_group_1.dim3,
                                dim_schedule_group_1.dim4,
                                dim_schedule_group_1.dim5,
                                dim_schedule_group_1.dim6,
                                dim_schedule_group_1.dim7,
                                dim_schedule_group_1.dim8,
                                dim_schedule_group_1.dim9,
                                dim_schedule_group_1.hh_start_dim1, dim_schedule_group_1.mm_start_dim1,
                                dim_schedule_group_1.hh_start_dim2, dim_schedule_group_1.mm_start_dim2,
                                dim_schedule_group_1.hh_start_dim3, dim_schedule_group_1.mm_start_dim3,
                                dim_schedule_group_1.hh_start_dim4, dim_schedule_group_1.mm_start_dim4,
                                dim_schedule_group_1.hh_start_dim5, dim_schedule_group_1.mm_start_dim5,
                                dim_schedule_group_1.hh_start_dim6, dim_schedule_group_1.mm_start_dim6,
                                dim_schedule_group_1.hh_start_dim7, dim_schedule_group_1.mm_start_dim7,
                                dim_schedule_group_1.hh_start_dim8, dim_schedule_group_1.mm_start_dim8,
                                dim_schedule_group_1.hh_start_dim9, dim_schedule_group_1.mm_start_dim9);

        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len1, data);
        printf("Schedule dim group 1 da dc gui den QT ok\r\n");
        int data_len2 = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_2.group,
                                dim_schedule_group_2.dim1,
                                dim_schedule_group_2.dim2,
                                dim_schedule_group_2.dim3,
                                dim_schedule_group_2.dim4,
                                dim_schedule_group_2.dim5,
                                dim_schedule_group_2.dim6,
                                dim_schedule_group_2.dim7,
                                dim_schedule_group_2.dim8,
                                dim_schedule_group_2.dim9,
                                dim_schedule_group_2.hh_start_dim1, dim_schedule_group_2.mm_start_dim1,
                                dim_schedule_group_2.hh_start_dim2, dim_schedule_group_2.mm_start_dim2,
                                dim_schedule_group_2.hh_start_dim3, dim_schedule_group_2.mm_start_dim3,
                                dim_schedule_group_2.hh_start_dim4, dim_schedule_group_2.mm_start_dim4,
                                dim_schedule_group_2.hh_start_dim5, dim_schedule_group_2.mm_start_dim5,
                                dim_schedule_group_2.hh_start_dim6, dim_schedule_group_2.mm_start_dim6,
                                dim_schedule_group_2.hh_start_dim7, dim_schedule_group_2.mm_start_dim7,
                                dim_schedule_group_2.hh_start_dim8, dim_schedule_group_2.mm_start_dim8,
                                dim_schedule_group_2.hh_start_dim9, dim_schedule_group_2.mm_start_dim9);

        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len2, data);
        printf("Schedule dim group 2 da dc gui den QT ok\r\n");
        int data_len3 = sprintf(data, TEMPLATE_DIM_SCHEDULE_ACTIVE, CMD_QT_LOAD_DIM_ALL_SCHEDULE, dim_schedule_group_3.group,
                                dim_schedule_group_3.dim1,
                                dim_schedule_group_3.dim2,
                                dim_schedule_group_3.dim3,
                                dim_schedule_group_3.dim4,
                                dim_schedule_group_3.dim5,
                                dim_schedule_group_3.dim6,
                                dim_schedule_group_3.dim7,
                                dim_schedule_group_3.dim8,
                                dim_schedule_group_3.dim9,
                                dim_schedule_group_3.hh_start_dim1, dim_schedule_group_3.mm_start_dim1,
                                dim_schedule_group_3.hh_start_dim2, dim_schedule_group_3.mm_start_dim2,
                                dim_schedule_group_3.hh_start_dim3, dim_schedule_group_3.mm_start_dim3,
                                dim_schedule_group_3.hh_start_dim4, dim_schedule_group_3.mm_start_dim4,
                                dim_schedule_group_3.hh_start_dim5, dim_schedule_group_3.mm_start_dim5,
                                dim_schedule_group_3.hh_start_dim6, dim_schedule_group_3.mm_start_dim6,
                                dim_schedule_group_3.hh_start_dim7, dim_schedule_group_3.mm_start_dim7,
                                dim_schedule_group_3.hh_start_dim8, dim_schedule_group_3.mm_start_dim8,
                                dim_schedule_group_3.hh_start_dim9, dim_schedule_group_3.mm_start_dim9);

        cl->send_ex(cl, UWSC_OP_TEXT, 1, data_len3, data);
        printf("Schedule dim group 3 da dc gui den QT ok\r\n");
        flag_control.flag_groupschedulesetting = 1;
    }
    if (!strncmp(toppic, "request-power-schedule", 22))
    {
        printf("reach here\r\n");

        flag_control.flag_requestpowerschedule = 1;
    }
    if (!strncmp(toppic, "request-light-schedule", 22))
    {
        printf("reach here\r\n");

        flag_control.flag_requestlightschedule = 1;
    }
    if (!strncmp(toppic, "request-group-schedule", 22))
    {
        printf("reach here\r\n");

        flag_control.flag_requestgroupschedule = 1;
    }
    if (!strncmp(toppic, "sync-time", 10))
    {
        printf("reach here\r\n");
        printf("NTP_update: active\r\n");
        system("sudo timedatectl set-ntp true");
        char data[1000];
        int len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_CONTROL_UPDATE_NTP_RTC, 1);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
        memset(data, 0, sizeof(data));

        // printf("Founded keyword\r\n");
        len = sprintf(data, "CPU-{\"CMD\": %d, \"Value\": %d}", CMD_QT_TO_CPU_MODE_AUTO_RTC, 1);
        cl->send_ex(cl, UWSC_OP_TEXT, 1, len, data);
        memset(data, 0, sizeof(data));

        flag_control.flag_synctime = 1;
    }

    if (!strncmp(toppic, "dim-information", 15))
    {
        printf("reach here\r\n");
        struct json_object *new_obj, *o;
        new_obj = json_tokener_parse(data);
        printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
        if (!new_obj)
        {
            printf("Error Null");
            return;
        }

        if (!json_object_object_get_ex(new_obj, "port1", &o))
        {
            printf("Field %s does not exist\n", "port1");
            return;
        }
        fb_dim_port1.lampport = json_object_get_int(o);
        printf("So den port 1: %d\n\n", fb_dim_port1.lampport);
        if (!json_object_object_get_ex(new_obj, "port2", &o))
        {
            printf("Field %s does not exist\n", "port2");
            return;
        }
        fb_dim_port2.lampport = json_object_get_int(o);
        printf("So den port 2: %d\n\n", fb_dim_port2.lampport);
        for (int i = 1; i <= fb_dim_port1.lampport; i++)
        {
            send_byte(CMD_CPU_TO_MASTER_COUNT_LAMP_RF, i);
            delay(200);
        }
        flag_control.flag_diminformation = 1;
    }
    
    free(topic_name);
}

void *client_refresher(void *client)
{
    while (1)
    {
        mqtt_sync((struct mqtt_client *)client);
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
