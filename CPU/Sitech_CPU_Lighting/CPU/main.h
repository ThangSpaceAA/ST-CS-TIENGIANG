#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "uwsc/log.h"
#include "uwsc/uwsc.h"
#include "uwsc/utils.h"
#include "uwsc/buffer.h"
#include "uwsc/config.h"


#include <sys/time.h>
#include <time.h>
#include "modbus/modbus.h"
#include "MFM384/Modbus_Master_MFM384.h"
#include "MFM384/Modbus_Master_MFM384.c"
#include <stdint.h>

#include "Parameter_CPU.h"
#include "function_system.h"
#include <time.h>

#include "serial-c/Serial.h"
#include "serial-c/Serial.c"
#include "mqtt-c/mqtt.h"
#include "mqtt-c/mqtt.c"
#include "mqtt-c/mqtt_pal.h"
#include "mqtt-c/mqtt_pal.c"
#include "templates/posix_sockets.h"
#include <math.h>
#include "json-c/json.h"

#define DEBUG
#define DEBUG_WEBSOCKET
#define DEBUG_MQTT

static bool is_flag_opened_websocket = false;

#define REMOTE_ID_PM_SELECT         2
#define REMOTE_ID_SS_LIGHT_L        4
#define REMOTE_ID_SS_LIGHT_R        3
#define REMOTE_ID_SS_RAIN           5
#define REMOTE_ID_SS_TEMP_HUM       6

/* khoảng thời gian bật tắt sử dụng sensor ambient light*/
#define TIME_SET_NIGHT_ENABLE_CONTROL_AMBIENT_LIGHT     1170
#define TIME_SET_DAY_ENABLE_CONTROL_AMBIENT_LIGHT        330
/*---------------------------------------------------------OPTION MAIN---------------------------------------------------------------------------------------*/
// #define DEBUG_MAIN
pthread_t thread_Main;
void *main_process(void *threadArgs);
/*---------------------------------------------------------OPTION MAIN---------------------------------------------------------------------------------------*//*---------------------------------------------------------DEFINE CMD------------------------------------------------------------------------------------*/
pthread_t thread_Serial;
void *Serial(void *threadArgs);

pthread_t thread_Mqtt;
void *Mqtt(void *threadArgs);
/*---------------------------------------------------------OPTION MODBUS---------------------------------------------------------------------------------------*/
// #define MODBUS_SIMULATOR

#define MODBUS_MF384AC
#define DEBUG_MODBUS
pthread_t thread_ModBus;
void *ModeBus(void *threadArgs);
/*---------------------------------------------------------OPTION MODBUS---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------GUI DU LIEU LEN MAN HINH---------------------------------------------------------------------------------------*/
// #define MODBUS_SIMULATOR
//#define DEBUG_SEND_DATA_MONITOR
//#define DEBUG_MODBUS
pthread_t thread_send_parameter_to_Monitor;
void *send_parameter_to_Monitor(void *threadArgs);

pthread_t thread_check_mode_OFF;
void *check_mode_Off(void *threadArgs);
volatile bool is_Off_Mode_No_Control = false;
/*---------------------------------------------------------GUI DU LIEU LEN MAN HINH----------------------------------------------------------------------------------------*/
pthread_t thread_GPS; 
void *gps_process(void *threadArgs);

pthread_t thread_registor_modbus_sensor;
void *Register_Modbus_Sensor(void *threadArgs);

pthread_t thread_registor_check_reset;
void *check_reset_thread(void *threadArgs);

void mtlc_sensor_control(modbus_ss_light_t *sys_sensor, current_time_t *current_time);
void mtlc_process_value_sensor(modbus_ss_light_t *sys_sensor, current_time_t *current_time, type_mtlc_sensor_t *mtlc_sensor_struct, type_mtlc_force_on_t *mtlc_force_on);
/*---------------------------------------------------------OPTION WEBSOCKET---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------OPTION WEBSOCKET---------------------------------------------------------------------------------------*//*---------------------------------------------------------DEFINE CMD------------------------------------------------------------------------------------*/

/*---------------------------------------------------------DEFINE CMD------------------------------------------------------------------------------------*/

#define CMD_CPU_TO_QT_DOMAIN_SERVER                                             2
#define CMD_CPU_TO_QT_PARAMETER_PWR                                             3
#define CMD_CPU_TO_QT_MODE                                                      4
#define CMD_CPU_TO_QT_STATUS_PWR                                                5
#define CMD_QT_TO_CPU_CONTROL_ON_OFF                                            6
#define CMD_QT_TO_CPU_GET_MODE                                                  7
#define CMD_CPU_TO_QT_TIME_ACTIVE                                               8
#define CMD_QT_TO_CPU_TIME_ACTIVE                                               9
#define CMD_CPU_STATU_AUTO_CONTROL                                              10
#define CMD_QT_CALL_SCHEDULE_LOAD_EDIT                                          11
#define CMD_QT_TO_CPU_MODE_AUTO_RTC                                             12
#define CMD_QT_CONTROL_UPDATE_NTP_RTC                                           13
#define CMD_QT_SET_RTC_MANUAL                                                   14
#define CMD_APP_CONTROL_ON_OFF                                                  15
#define CMD_APP_RELEASE_CONTROL                                                 16
#define CMD_QT_TO_CPU_ACTIVE_SENSOR_AMBIENT_LIGHT                               17

#define CMD_QT_SET_DIM_SCHEDULE                                           		18
#define CMD_QT_COUNTER_LAMP                                           			19
#define CMD_QT_TO_CPU_GET_VALUE_AMBIENT_SENSOR                                  20
#define CMD_QT_CALL_LOAD_PARAMETERS_SENSOR                                      21

#define CMD_QT_TO_CPU_CHECK_LAMP_IN_GROUP                                       50
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CMD_QT_TO_CPU_DIM_ALL                                                   255
#define CMD_CPU_TO_MASTER_DIM_ALL												254
#define CMD_CPU_TO_QT_DIM_ALL_SCHEDULE											253
#define CMD_QT_LOAD_DIM_ALL_SCHEDULE											252
#define CMD_QT_TO_CPU_DIM_ALL_SCHEDULE											251
#define CMD_QT_TO_CPU_DIM_ID													250
#define CMD_QT_TO_CPU_DIM_ID_DISABLE											249
#define CMD_CPU_TO_MASTER_DIM_ID												248
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_1											247
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_1											246
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_2											245
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_2											244
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_3											243
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_3											242
#define CMD_MASTER_TO_SLAVE_DIM_ALL     					                    241
#define CMD_MASTER_TO_SLAVE_DIM_ID       							            240
#define CMD_MASTER_TO_SLAVE_DIM_GROUP    							            239
#define CMD_CPU_TO_MASTER_DIM_GROUP    								            238
#define CMD_QT_TO_CPU_COUNT_LAMP									            237
#define CMD_CPU_TO_MASTER_COUNT_LAMP								            236
#define CMD_MASTER_TO_SLAVE_COUNT_LAMP								            235
#define CMD_QT_TO_CPU_SETUP_INFO									            234
#define CMD_CPU_TO_MASTER_SETUP_INFO								            233
#define CMD_MASTER_TO_SLAVE_SETUP_INFO								            232
#define CMD_QT_TO_CPU_ADD_ID_TO_GROUP								            231
#define CMD_CPU_TO_MASTER_ADD_ID_TO_GROUP								        230
#define CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP							            229
#define CMD_QT_TO_CPU_DIM_GROUP			                                       	228
#define CMD_CPU_TO_MASTER_POWER_CONTROL	                                       	227
#define CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1                                   	226
#define CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2                                   	225
#define CMD_CPU_TO_QT_CONNECTION    											224
#define CMD_QT_TO_CPU_OFF_SCREEN    											223
#define CMD_QT_TO_CPU_ON_SCREEN    								     			222
#define CMD_QT_TO_CPU_DKTB	    								     			221
#define CMD_SLAVE_FB_DIM_TO_MASTER 								     			220
#define CMD_MASTER_FB_DIM_TO_CPU                	                         	219
#define CMD_QT_TO_CPU_CHANGE_ID_RF 								     			218
#define CMD_CPU_TO_MASTER_CHANGE_ID_RF							     			217
#define CMD_MASTER_TO_SLAVE_CHANGE_ID_RF							     	    216
#define CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB							     	    215
#define CMD_MASTER_TO_CPU_CHANGE_ID_RF_FB							     	    214
#define CMD_CPU_TO_QT_CHANGE_ID_RF_FB								     	    213
#define CMD_QT_TO_CPU_COUNT_LAMP_RF									     	    212
#define CMD_CPU_TO_MASTER_COUNT_LAMP_RF								     	    211
#define CMD_MASTER_TO_SLAVE_COUNT_LAMP_RF							     	    210
#define CMD_SLAVE_TO_MASTER_COUNT_LAMP_RF_FB						     	    209
#define CMD_MASTER_TO_CPU_COUNT_LAMP_RF_FB							     	    208
#define CMD_CPU_TO_QT_COUNT_LAMP_RF_FB			    				     	    207
#define CMD_QT_TO_CPU_OPEN_SERVER 								     			206
#define CMD_CPU_TO_QT_OPEN_SERVER 								     			205
#define CMD_QT_TO_CPU_TYPE										     			204
#define CMD_CPU_TO_QT_TYPE										     			203
#define CMD_QT_TO_CPU_CONFIG_POWER_LAMP									        202

#define CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP1                                     201
#define CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP2                                     200
#define CMD_CPU_TO_QT_SET_LAMP_RS485_GROUP3                                     199

#define CMD_CPU_TO_MASTER_SET_LAMP_RS485_PORT2_TO_GROUP                         30
#define CMD_CPU_TO_MASTER_SET_LAMP_RS485_PORT_TO_GROUP                          31
#define CMD_CPU_TO_MASTER_FORCE_ON_CABINET                                      32
#define CMD_CPU_TO_MASTER_STATUS_ACTIVE_SENSOR                                  33

#define CMD_CPU_TO_MASTER_KEEP_ALLIVE_CPU                                       137
#define CMD_MASTER_TO_CPU_STATUS_FORCE_CABINET                                  136

#define CMD_CPU_TO_QT_SENSOR_ON_DASHBOARD                                       135
#define CMD_CPU_TO_QT_STATUS_SENSOR_MONITORING                                  133
#define CMD_CPU_TO_QT_SENSOR_MONITORING                                         132 
#define CMD_CPU_TO_QT_TIME_MONITORING_FROM_MASTER                               131

#define CMD_MASTER_CPU_TIME_CURRENT                                             130

#define CMD_QT_TO_CPU_MANUAL_GPS                                                129
#define CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2                   128
#define CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1                   127
#define CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP                               126
#define CMD_QT_TO_CPU_MONITORING_LAMP_RF                                        125
#define CMD_QT_TO_CPU_MONITORING_LAMP_RS485                                     124
#define CMD_QT_TO_CPU_DELETE_LAMP_RS485_PORT_TO_GROUP                           123
#define CMD_QT_TO_CPU_DELETE_LAMP_RS485_TO_GROUP                                122
#define CMD_QT_TO_CPU_SET_LAMP_RS485_PORT_TO_GROUP                              121
#define CMD_QT_TO_CPU_SET_LAMP_RF_TO_GROUP                                      120
#define CMD_QT_TO_CPU_DELETE_LAMP_RF_GROUP_DIM1                                 119
#define CMD_CPU_TO_QT_LAMP_RF_GROUP_DIM1                                        118
#define CMD_CPU_TO_QT_DIM_VALUE                                                 100
#define CMD_QT_TO_CPU_DIM_VALUE                                                 101
#define CMD_CPU_TO_QT_DIM_SCHEDULE_ACTIVE                                       102
#define CMD_QT_TO_CPU_DIM_SCHEDULE_ACTIVE                                       103
#define CMD_QT_TO_CPU_DIM_ADD			                                       	104
#define CMD_QT_TO_CPU_SET_ADD_AND_GROUP			                                105
//#define CMD_QT_TO_CPU_COUNT_LAMP		                                       	106
#define CMD_QT_TO_CPU_SET_GROUP			                                       	107
//#define CMD_QT_TO_CPU_DIM_GROUP			                                    108
#define CMD_CPU_TO_QT_LOAD_DIM_VALUE                                       		109


#define CMD_CPU_TO_MASTER_CHANGE_POWER_LAMP                                     90
#define CMD_CPU_TO_MASTER_SCHEDULE_POWER                                        110
#define CMD_CPU_TO_MASTER_SETTING_SCHEDULE_DIM_ALL_CONTROL                      112
#define CMD_CPU_TO_MASTER_FB_DIM_PORT1                                          113
#define CMD_CPU_TO_MASTER_FB_DIM_PORT2                                          114
#define CMD_CPU_TO_MASTER_MODE_ACTIVE                                           115
#define CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT1                          116
#define CMD_CPU_TO_MASTER_SETTINGS_LAMP_TO_GROUP_PORT2                          117


#define CMD_SLAVE_TO_MASTER_STATUS_LAMP					     	                104
#define CMD_MASTER_TO_CPU_SYN_RTC_TIME                                          134

#include "gps.h"

/*---------------------------------------------------------DEFINE CMD------------------------------------------------------------------------------------*/
// #define CMD_CPU_TO_QT_HOME                                                      2
// #define CMD_CPU_TO_QT_ACTIVE_TIME                                               4
// #define CMD_CPU_TO_QT_STATUS_PWR                                                5

// #define CMD_QT_TO_CPU_MODE_AUTO_RTC                                             6   
// #define CMD_QT_CONTROL_UPDATE_NTP_RTC                                           7
// #define CMD_QT_CONTROL_SET_TIME_RTC                                             8         

volatile int mode_pre = -1;
volatile int status_pwr_re = -1;
volatile bool is_status_power = false;
volatile bool is_status_power_tmp = true;
volatile int status_power = -1;
volatile int power_mode_status = -1;
volatile int type_check_lamp_in_group = 0;
volatile bool is_app_connected = false;
volatile bool is_receive_update_rtc_ex = false;

volatile bool is_check_timeout_sensor_light_1   = false;
volatile bool is_check_timeout_sensor_light_2   = false;
volatile bool is_enable_time_delay_level_1      = false;
volatile bool is_enable_time_delay_level_2      = false;

volatile bool is_check_first_on_power = true;
volatile bool is_check_second_after_power = false;

volatile bool is_read_again_threshold_ambient_light = false;
volatile bool is_check_ss_ambient_light = false;
volatile bool is_connect_sensor = false;
volatile bool is_check_reset_thread_modbus = false;

long long previousMillis_CPU_1s = 0;
long long previousMillis_CPU_60s = 0;
long long previousMillis_CPU_1800s = 0;
long long previousMillis_CPU_2000s = 0;
long long interval_CPU_1s = 1000;
long long interval_CPU_60s = 3000;
long long interval_CPU_1800s = 5000;
long long interval_CPU_2000s = 5000;
#endif
