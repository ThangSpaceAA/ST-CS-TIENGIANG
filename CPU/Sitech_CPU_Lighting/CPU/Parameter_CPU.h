#ifndef PARAMETER_CPU_H
#define PARAMETER_CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>


#include "uwsc/log.h"
#include "uwsc/uwsc.h"
#include "uwsc/utils.h"
#include "uwsc/buffer.h"
#include "uwsc/config.h"

#define auto_Mode           4
#define manual_Mode         5
#define relay_1             0
#define relay_2             2

#define Size_Queue_Array	100

#define Dim_All_Group		200
#define Dim_Id_Group		1

#define MODE_OUTPUT_CPU_RS485	0
#define MODE_OUTPUT_CPU_RF		1

#define TEMPLATE_DOMAIN_SERVER "CPU-{\"CMD\": %d,\
\"domain\":\"%s\"}"

#define TEMPLATE_SETTING "CPU-{\"CMD\": %d,\
\"server\": %d,\
\"type\": %d}"

#define TEMPLATE_TYPE "CPU-{\"CMD\": %d,\
\"type\": %d}"

#define TEMPLATE_MODE "CPU-{\"CMD\": %d,\
\"mode\": %d}"

#define TEMPLATE_NUMBER_GROUP_DIM1 "CPU-{\"CMD\": %d,\
\"slot1\":\"%s\"}"

#define TEMPLATE_NUMBER_DIM_GROUP1 "CPU-{\"CMD\": %d,\
\"slot1\":\"%s\",\
\"slot2\":\"%s\"}"

#define TEMPLATE_NUMBER_DIM_GROUP2 "CPU-{\"CMD\": %d,\
\"slot1\":\"%s\",\
\"slot2\":\"%s\"}"

#define TEMPLATE_NUMBER_DIM_GROUP3 "CPU-{\"CMD\": %d,\
\"slot1\":\"%s\",\
\"slot2\":\"%s\"}"

#define TEMPLATE_NUMBER_GROUP_PORT1 "CPU-{\"CMD\": %d,\
\"slot1\":\"%s\"}"

#define TEMPLATE_NUMBER_GROUP_PORT2 "CPU-{\"CMD\": %d,\
\"slot2\":\"%s\"}"

#define TEMPLATE_STATUS_MANUAL_GPS "CPU-{\"CMD\": %d,\
\"status\":\"%d\"}"

#define TEMPLATE_FB_ADD_IN_GROUP "CPU-{\"CMD\": %d,\
\"group\": %d,\
\"add1\": %d,\
\"add2\": %d,\
\"add3\": %d,\
\"add4\": %d,\
\"add5\": %d,\
\"add6\": %d,\
\"add7\": %d,\
\"add8\": %d,\
\"add9\": %d,\
\"add10\": %d,\
\"add11\": %d,\
\"add12\": %d,\
\"add13\": %d,\
\"add14\": %d,\
\"add15\": %d,\
\"add16\": %d}"

#define TEMPLATE_STATUS_PWR "CPU-{\"CMD\": %d,\
\"power\":%d}"

#define TEMPLATE_TIME_ACTIVE "CPU-{\"CMD\": %d,\
\"hh_start1\": %d,\
\"mm_start1\": %d,\
\"hh_end1\": %d,\
\"mm_end1\": %d,\
\"hh_start2\": %d,\
\"mm_start2\": %d,\
\"hh_end2\": %d,\
\"mm_end2\": %d,\
\"hh_start3\": %d,\
\"mm_start3\": %d,\
\"hh_end3\": %d,\
\"mm_end3\": %d}"

#define TEMPLATE_AUTO_CONTROL "CPU-{\"CMD\": %d,\
\"staus\": %s}"

#define TEMPLATE_DIM_ACTIVE "CPU-{\"CMD\": %d,\
\"dim\": %d}"

#define TEMPLATE_COUNTER_LAMP "CPU-{\"CMD\": %d,\
\"val1\": %d,\
\"val2\": %d}"

#define TEMPLATE_COUNTER_LAMP2 "CPU-{\"CMD\": %d,\
\"val2\": %d}"

#define TEMPLATE_NOTIFICATION "CPU-{\"CMD\": %d,\
\"notification\": %s}"

#define TEMPLATE_SENSOR_ON_DASHBOARD "CPU-{\"CMD\": %d,\
\"timehh_start\": %d,\
\"timemm_start\": %d,\
\"timehh_end\": %d,\
\"timemm_end\": %d,\
\"checkbox\": %d,\
\"threshold_ambient_light\": %d}"

#define TEMPLATE_TIME_MASTER_MONITOR "CPU-{\"CMD\": %d,\
\"timehh\": %d,\
\"timemm\": %d}"

#define TEMPLATE_SENSOR_MONITOR "CPU-{\"CMD\": %d,\
\"ambientlight1\": %d,\
\"ambientlight2\": %d,\
\"pm2.5\": %d,\
\"co2\": %d,\
\"temp\": %d,\
\"hum\": %d}"

#define TEMPLATE_STATUS_SENSOR_MONITOR "CPU-{\"CMD\": %d,\
\"status_ambientlight1\": %d,\
\"status_ambientlight2\": %d,\
\"status_pm2.5\": %d,\
\"status_co2\": %d,\
\"status_temp\": %d,\
\"status_hum\": %d,\
\"status_rain\": %d}"

#define TEMPLATE_TIMEACTIVE_SENSOR "CPU-{\"CMD\": %d,\
\"hh_start_ss_light\": %d,\
\"mm_start_ss_light\": %d,\
\"hh_end_ss_light\": %d,\
\"mm_end_ss_light\": %d,\
\"threshold_ambient_light\": %d,\
\"check\": %d}"

#define TEMPLATE_DIM_SCHEDULE_ACTIVE "CPU-{\"CMD\": %d,\
\"group\": %d,\
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
\"mm_start_dim9\": %d}"	

#define ADDR_HOME	"/home/pi"

#define ADDR_GROUP1_PORT1 "lamp_schedule_gr1_port1.txt"
#define ADDR_STORAGE_GROUP1_PORT1 ADDR_HOME "/" ADDR_GROUP1_PORT1

#define ADDR_GROUP1_PORT2 "lamp_schedule_gr1_port2.txt"
#define ADDR_STORAGE_GROUP1_PORT2 ADDR_HOME "/" ADDR_GROUP1_PORT2

#define ADDR_GROUP2_PORT1 "lamp_schedule_gr2_port1.txt"
#define ADDR_STORAGE_GROUP2_PORT1  ADDR_HOME "/" ADDR_GROUP2_PORT1 

#define ADDR_GROUP2_PORT2 "lamp_schedule_gr2_port2.txt"
#define ADDR_STORAGE_GROUP2_PORT2 ADDR_HOME "/" ADDR_GROUP2_PORT2

#define ADDR_GROUP3_PORT1 "lamp_schedule_gr3_port2.txt"
#define ADDR_STORAGE_GROUP3_PORT1 ADDR_HOME "/" ADDR_GROUP3_PORT1

#define ADDR_GROUP3_PORT2 "lamp_schedule_gr3_port2.txt"
#define ADDR_STORAGE_GROUP3_PORT2 ADDR_HOME "/" ADDR_GROUP3_PORT2

#define NONE                        0
#define AUTO_MODE                   1
#define MANUAL_MODE                 2
uint8_t mode_active;

typedef struct __attribute__((packed))
{
	uint8_t server;
	uint8_t type;
	char urlmqtt[100];
} setting_t; 
setting_t setting;

typedef struct __attribute__((packed))
{
	uint8_t dim;
} dim_active_t;
dim_active_t dim_active;

typedef struct __attribute__((packed))
{
	uint8_t group;
	uint8_t dim1;
	uint8_t dim2;
	uint8_t dim3;
	uint8_t dim4;
	uint8_t dim5;
	uint8_t dim6;
	uint8_t dim7;
	uint8_t dim8;
	uint8_t dim9;
	uint8_t hh_start_dim1;
	uint8_t mm_start_dim1;
	uint8_t hh_start_dim2;
	uint8_t mm_start_dim2;
	uint8_t hh_start_dim3;
	uint8_t mm_start_dim3;
	uint8_t hh_start_dim4;
	uint8_t mm_start_dim4;
	uint8_t hh_start_dim5;
	uint8_t mm_start_dim5;
	uint8_t hh_start_dim6;
	uint8_t mm_start_dim6;
	uint8_t hh_start_dim7;
	uint8_t mm_start_dim7;
	uint8_t hh_start_dim8;
	uint8_t mm_start_dim8;
	uint8_t hh_start_dim9;
	uint8_t mm_start_dim9;
} dim_active_schedule_t;

dim_active_schedule_t dim_schedule_active;
dim_active_schedule_t dim_schedule_group_1;
dim_active_schedule_t dim_schedule_group_2;
dim_active_schedule_t dim_schedule_group_3;

typedef struct __attribute__((packed))
{
	uint8_t hh_start1;
	uint8_t mm_start1;
	uint8_t hh_end1;
	uint8_t mm_end1;
	uint8_t hh_start2;
	uint8_t mm_start2;
	uint8_t hh_end2;
	uint8_t mm_end2;
	uint8_t hh_start3;
	uint8_t mm_start3;
	uint8_t hh_end3;
	uint8_t mm_end3;
} time_active_t;
time_active_t time_active;

typedef struct __attribute__((packed))
{
	char hh_start[10];
	char mm_start[10];
	char hh_end[10];
	char mm_end[10];
	uint8_t size_array_on_off_schedule;
} time_on_off_t;
time_on_off_t time_on_off;

typedef struct __attribute__((packed)){
	uint8_t hh_current;
	uint8_t mm_current;
	uint8_t ss_current;
} current_time_t;
current_time_t current_time;
current_time_t current_time_master;
current_time_t getCurrent_Time(time_t rawtime, struct tm *timeinfo);

typedef struct __attribute__((packed))
{
	int rear;
	int front;
	uint8_t inp_arr[100];
} xqueue_t;
xqueue_t queue;

typedef struct __attribute__((packed))
{
	int rear;
	int front;
	uint8_t inp_arr[100];
} xqueue_lamp_port1_t;
xqueue_lamp_port1_t queue_lamps_port1;

typedef struct __attribute__((packed))
{
	int rear;
	int front;
	uint8_t inp_arr[100];
} xqueue_lamp_port2_t;
xqueue_lamp_port2_t queue_lamps_port2;

/** variable monitoring lamp to qt and storage*/
/**
 * Gr1 : port 1, port 2
 * Gr2 : port 1, port 2
 * Gr3 : port 1, port 2
 */

typedef struct __attribute__ ((packed))
{
	int group;
	int front;
	int rear;
	uint8_t inp_lamp_arr[150];
}xqueue_lamp_group_t;
xqueue_lamp_group_t lamp_gr1_port1;
xqueue_lamp_group_t lamp_gr1_port2;
xqueue_lamp_group_t lamp_gr2_port1;
xqueue_lamp_group_t lamp_gr2_port2;
xqueue_lamp_group_t lamp_gr3_port1;
xqueue_lamp_group_t lamp_gr3_port2;


FILE *fp; // File luu macid Gateway
FILE *f_time_update; // File luu macid Gateway
FILE *f_date_update; // File luu macid Gateway
FILE *fptr; // File luu du lieu cam bien

FILE *time_active_file;
void Write_struct_time_active_toFile(FILE *file, char *link, time_active_t data_Write);
time_active_t Read_struct_time_active_toFile(FILE *file, char *link, time_active_t data_Write);

FILE *time_on_off_file;
void Write_struct_time_on_off_toFile(FILE *file, char *link, time_on_off_t data_Write);
time_on_off_t Read_struct_time_on_off_toFile(FILE *file, char *link, time_on_off_t data_Write);

FILE *dim_active_file;
void Write_struct_dim_active_toFile(FILE *file, char *link, dim_active_t data_Write);
dim_active_t Read_struct_dim_active_toFile(FILE *file, char *link, dim_active_t data_Write);

FILE *dim_schedule_active_file;
void Write_struct_dim_schedule_active_toFile(FILE *file, char *link, dim_active_schedule_t data_Write);
dim_active_schedule_t Read_struct_dim_schedule_active_toFile(FILE *file, char *link, dim_active_schedule_t data_Write);

FILE *setting_file;
void Write_struct_setting_toFile(FILE *file, char *link, setting_t data_Write);
setting_t Read_struct_setting_toFile(FILE *file, char *link, setting_t data_Write);

FILE *queue_file;
void Write_struct_queue_toFile(FILE *file, char *link, xqueue_t data_Write);
xqueue_t Read_struct_queue_toFile(FILE *file, char *link, xqueue_t data_Write);

FILE *queue_lamp_port1_file;
void Write_struct_queue_lamps_port1_toFile(FILE *file, char *link, xqueue_lamp_port1_t data_Write);
xqueue_lamp_port1_t Read_struct_queue_lamps_port1_toFile(FILE *file, char *link, xqueue_lamp_port1_t data_Write);

FILE *queue_lamp_port2_file;
void Write_struct_queue_lamps_port2_toFile(FILE *file, char *link, xqueue_lamp_port2_t data_Write);
xqueue_lamp_port2_t Read_struct_queue_lamps_port2_toFile(FILE *file, char *link, xqueue_lamp_port2_t data_Write);

FILE *queue_lamp_gr1_port1_file;
FILE *queue_lamp_gr1_port2_file;
FILE *queue_lamp_gr2_port1_file;
FILE *queue_lamp_gr2_port2_file;
FILE *queue_lamp_gr3_port1_file;
FILE *queue_lamp_gr3_port2_file;
void Write_struct_queue_lamps_group_toFile(FILE *file, char *link, xqueue_lamp_group_t data_Write);
xqueue_lamp_group_t Read_struct_queue_lamps_group_toFile(FILE *file, char *link, xqueue_lamp_group_t data_Write);

void sort_down_lamp_group(uint8_t a[], int n);
void sort_up_lamp_group(uint8_t number[], int n);
void show_queue_lamp_group(xqueue_lamp_group_t *lamp_group, int port);
void insert_queue_lamp_group(uint8_t insert_item, xqueue_lamp_group_t *lamp_group, int port);
void descrease_queue_lamp_group(xqueue_lamp_group_t *lamp_group, int port);
void send_lamp_group_cpu_to_qt(xqueue_lamp_group_t *lamp_gr1, xqueue_lamp_group_t *lamp_gr2, const char* template, const char cmd);
void reset_settings_lamp_group(void);
void get_all_settings_storage_lamp_group(void);

typedef struct __attribute__((packed)){
	uint16_t years;
	uint8_t month;
	uint8_t day;
	uint8_t hh;
	uint8_t mm;
	uint8_t ss;
}type_RTC_set_CPU_CM4_t;

typedef struct __attribute__((packed)){
    uint8_t     day;//0-6
    uint8_t     hour;//0-23
    uint8_t     minute;
    uint8_t     seconds;
    uint8_t     date;//1-31
    uint8_t     month;//1-12
    uint16_t    year;//20xx
}type_date_time_t;//

typedef struct __attribute__((packed)){
	uint8_t hour;
  	uint8_t min;
}type_RTC_monitoring_t;

typedef struct __attribute__((packed))
{
	uint8_t add;
	uint8_t group;
} type_lightAddConfig_t;

typedef struct __attribute__((packed))
{
	uint16_t val1;
	uint16_t val2;
} type_configPackage_t;
type_configPackage_t counters_package;
type_configPackage_t counters_package_p2;

FILE *counter_lamp_file;
void Write_struct_counter_lamp_toFile(FILE *file, char *link, type_configPackage_t data_Write);
type_configPackage_t Read_struct_counter_lamp_toFile(FILE *file, char *link, type_configPackage_t data_Write);

typedef struct __attribute__((packed))
{
	int hh_start;
	int mm_start;
	int hh_end;
	int mm_end;
	int threshold_ambient_light;
	bool check_enable;
}type_mtlc_sensor_t;
type_mtlc_sensor_t mtlc_sensor;

FILE *sensor_value_file;
void Write_struct_sensor_value_toFile(FILE *file, char *link, type_mtlc_sensor_t data_Write);
type_mtlc_sensor_t Read_struct_sensor_value_toFile(FILE *file, char *link, type_mtlc_sensor_t data_Write);

typedef struct __attribute__((packed))
{
	int hh_start;
	int mm_start;
	int hh_end;
	int mm_end;
	int check_time_current;
	uint8_t force_cabinet;
}type_mtlc_force_on_t;
type_mtlc_force_on_t mtlc_force_cabinet;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t port;
	uint8_t group;
	uint8_t dim;
} add_t;
add_t add;
add_t add_fb;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t group;
	uint8_t trigger_delele_all;
} group_lamp_t;
group_lamp_t add_lamp_rf_group;

typedef struct __attribute__((packed))
{
	uint8_t oldid;
	uint8_t newid;
	uint8_t cabinetid;
} changeid_t;
changeid_t changeid;

typedef struct __attribute__((packed))
{
	uint8_t group;
	uint8_t dim;
} group_t;
group_t group;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t port;
	uint8_t group;
	uint8_t dim;
} change_group_lamp_t;
change_group_lamp_t lamp_rf_group;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t group;
	uint8_t port;
	uint8_t trigger_delete_all;
} group_lamp_port1_t;
group_lamp_port1_t add_lamp_port_group;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t group;
	uint8_t trigger_delele_all;
} group_lamp_port2_t;
group_lamp_port2_t add_lamp_port2_group;

typedef struct __attribute__((packed))
{
	uint8_t id;
	uint8_t group;
	uint8_t port;
} group_lamp_tmp_send_master_t;
group_lamp_tmp_send_master_t lamp_send_master;

typedef struct __attribute__((packed))
{
	uint8_t param;
}cpu_require_update_rtc_t;
cpu_require_update_rtc_t cpu_update_rtc;

typedef struct __attribute__((packed))
{
	uint8_t value_power;
} change_power_lamp_t;
change_power_lamp_t system_control_lamp;

typedef struct __attribute__((packed))
{
	uint8_t add1;
	uint8_t add2;
	uint8_t add3;
	uint8_t add4;
	uint8_t add5;
	uint8_t add6;
	uint8_t add7;
	uint8_t add8;
	uint8_t add9;
	uint8_t add10;
	uint8_t add11;
	uint8_t add12;
	uint8_t add13;
	uint8_t add14;
	uint8_t add15;
	uint8_t add16;
} set_add_to_group;
set_add_to_group group1;
set_add_to_group group2;
set_add_to_group group3;

typedef struct __attribute__((packed))
{
	uint8_t flag_power;
	uint8_t flag_info;
	uint8_t flag_setgroup;
	uint8_t flag_dimid;
	uint8_t flag_setgroupschedule;
	uint8_t flag_groupschedulesetting;
	uint8_t flag_lightschedulesetting;
	uint8_t flag_requestpowerschedule;
	uint8_t flag_requestlightschedule;
	uint8_t flag_requestgroupschedule;
	uint8_t flag_synctime;
	uint8_t flag_diminformation;
}flag_t;
flag_t flag_control;

typedef struct __attribute__((packed))
{
	/* Thoi gian su dung cam bien*/
	int hh_start;
	int mm_start;
	int hh_end;
	int mm_end;
  	uint8_t force_cabinet_status;
}type_mtlc_data_response_t;
type_mtlc_data_response_t mtlc_system_response;

typedef struct __attribute__((packed))
{
	int value_sensor_present_1;
	int value_sensor_present_2;
	volatile bool value_sensor_rain;
	int value_temp_sensor;
	int  value_hum_sensor;	
}modbus_ss_light_t;
modbus_ss_light_t sys_sensor;
#endif