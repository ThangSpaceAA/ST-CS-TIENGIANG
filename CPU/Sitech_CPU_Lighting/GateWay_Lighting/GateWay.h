#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wsserver/ws.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include <string.h>
#include <unistd.h>     /* for close() */
#include <errno.h>

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../main_proto.pb-c.h"
#include "../device_proto.pb-c.h"
#include "../common_proto.pb-c.h"
#include "../user_proto.pb-c.h"

#include "uwsc/log.h"
#include "uwsc/uwsc.h"
#include "uwsc/ssl.h"
#include "uwsc/utils.h"
#include "uwsc/buffer.h"
#include "uwsc/config.h"

//#include "../CPU/Sitech_Json.h"

#define CMD_CPU_TO_QT_MODE                                                      4
#define CMD_CPU_TO_QT_STATUS_PWR                                                5
#define CMD_CPU_TO_QT_TIME_ACTIVE                                               8
#define CMD_CPU_TO_QT_PARAMETER_PWR                                             3


#define DEBUG_SENDER_WEBSOCKET_ONLINE       
#define DEBUG_OPEN_WEBSOCKET_ONLINE    

// #define TEMPLATE_HARDWARE_ID "CPU-{\"CMD\": %d,\
//                             \"harwareid\": \"%s\",\
//                             \"firmware_version\" \"%s\",\
//                             \"project_id\": \"%s\"}"

#define TEMPLATE_HARDWARE_ID "CPU-{\"CMD\": %d,\
                             \"harwareid\":\"%.*s\",\
                             \"firmware_version\":\"%.*s\",\
                             \"project_id\":\"%.*s\"}"

#define CMD_SERVER_TO_QT_HARDWARE_ID                                            11                            

typedef struct __attribute__((packed))
{
    char *nameClient;
    int ID;
}listDevice_t;
listDevice_t listDevice[2];

#define qt_packet                listDevice[0].ID
#define Cpu_packet               listDevice[1].ID

volatile bool is_flag_register_qtApp = false;
pthread_t thread_Test_Send;
void *Send_Message_Test(void *threadArgs);

pthread_t thread_Open_Websocket_Online;
void *open_Websocket_Online(void *threadArgs);
volatile is_flag_opend_weboscket_online = false;

pthread_t thread_send_Websocket_Online;
void *sender_Websocket_Online(void *threadArgs);



FILE *fp; // File luu macid Gateway
FILE *fw_version_GW;

char mac[18];
char re_mac[100] = "";
char *mac_id_gateway = NULL;
char *mac_final = NULL;
char *tmp_firmware = NULL;

char firmware_GW_version[1000] = "";
char *substr(char *s, int start, int end);

volatile bool isconnected_websocket_local = false; 


uint8_t tx_buffer[4096] = { 0 };
volatile bool is_register_device_to_server_online_success = false;
volatile bool is_login_success = false;
volatile bool is_connected_socket =false;

#endif