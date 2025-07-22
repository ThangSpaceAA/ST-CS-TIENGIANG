#ifndef _MAIN_H
#define _MAIN_H


/* khoảng thời gian bật tắt sử dụng sensor ambient light*/
#define TIME_SET_NIGHT_ENABLE_CONTROL_AMBIENT_LIGHT     1140
#define TIME_SET_DAY_ENABLE_CONTROL_AMBIENT_LIGHT        330

#define lamp_total_default                                                      40

#define ENABLE_GROUP1                                                           1
#define ENABLE_GROUP2                                                           2
#define ENABLE_GROUP3                                                           3

#define DISABLE_GROUP1                                                          99
#define DISABLE_GROUP2                                                          97
#define DISABLE_GROUP3                                                          98

#define total_lamp_now                                                          10 
#define EEPROM_ADDR                                                             0
#define ADD_SAVE_SETTING                                                        1

#define SIZE_QUEUE_ARRAY	                                                      100

#define LSV_NODE_CONNECT_FB                     	                              99
#define LSV_NODE_END_NUMLAMP_FB                 	                              100       
#define CMD_CPU_SET_DIM_ADDRESS                                                 201
#define CMD_CPU_COUNT_LAMP                                                      203
#define CMD_CPU_SET_GROUP                                                       204
#define CMD_CPU_DIM_GROUP                                                       205
#define CMD_CPU_TIMEOUT_ENABLE                                                  159

#define CMD_QT_TO_CPU_DIM_ALL                                                   255
#define CMD_CPU_TO_MASTER_DIM_ALL												                        254
#define CMD_CPU_TO_QT_DIM_ALL_SCHEDULE											                    253
#define CMD_QT_LOAD_DIM_ALL_SCHEDULE											                      252
#define CMD_QT_TO_CPU_DIM_ALL_SCHEDULE											                    251
#define CMD_QT_TO_CPU_DIM_ID													                          250
#define CMD_QT_TO_CPU_DIM_ID_DISABLE											                      249
#define CMD_CPU_TO_MASTER_DIM_ID												                        248 /*set group tung node lamp*/
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_1											                    247
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_1											                    246
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_2											                    245
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_2											                    244
#define CMD_QT_TO_CPU_SCHEDULE_GROUP_3											                    243
#define CMD_CPU_TO_QT_SCHEDULE_GROUP_3											                    242
#define CMD_MASTER_TO_SLAVE_DIM_ALL     										                    241
#define CMD_MASTER_TO_SLAVE_DIM_ID       										                    240
// #define CMD_MASTER_TO_SLAVE_DIM_GROUP    										                    239
#define CMD_CPU_TO_MASTER_DIM_GROUP    										                      238
#define CMD_QT_TO_CPU_COUNT_LAMP									                              237
#define CMD_CPU_TO_MASTER_COUNT_LAMP								                            236
#define CMD_MASTER_TO_SLAVE_COUNT_LAMP								                          235
#define CMD_QT_TO_CPU_SETUP_INFO									                              234
#define CMD_CPU_TO_MASTER_SETUP_INFO								                            233
#define CMD_MASTER_TO_SLAVE_SETUP_INFO								                          232
#define CMD_QT_TO_CPU_ADD_ID_TO_GROUP								                            231
#define CMD_CPU_TO_MASTER_ADD_ID_TO_GROUP								                        230
#define CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP								                      229
#define CMD_QT_TO_CPU_DIM_GROUP			                                       	    228
#define CMD_CPU_TO_MASTER_POWER_CONTROL	                                       	227
#define CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1                                   	226
#define CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2                                   	225
#define CMD_MASTER_TO_CPU_COUNT_LAMP_RF          	                         	    224
#define CMD_SLAVE_FB_DIM_TO_MASTER              	                         	    220
#define CMD_MASTER_FB_DIM_TO_CPU                	                         	    219
#define CMD_QT_TO_CPU_CHANGE_ID_RF 								     			                    218
#define CMD_CPU_TO_MASTER_CHANGE_ID_RF							     			                  217
#define CMD_MASTER_TO_SLAVE_CHANGE_ID_RF							     			                216
#define CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB							     	                  215
#define CMD_MASTER_TO_CPU_CHANGE_ID_RF_FB							     	                    214
#define CMD_CPU_TO_QT_CHANGE_ID_RF_FB								     	                      213
#define CMD_QT_TO_CPU_COUNT_LAMP_RF									     	                      212
#define CMD_CPU_TO_MASTER_COUNT_LAMP_RF								     	                    211
#define CMD_MASTER_TO_SLAVE_COUNT_LAMP_RF							     	                    210
#define CMD_SLAVE_TO_MASTER_COUNT_LAMP_RF_FB						     	                  209
#define CMD_MASTER_TO_CPU_COUNT_LAMP_RF_FB							     	                  208
#define CMD_CPU_TO_QT_COUNT_LAMP_RF_FB			    				     	                  207

#define CMD_MASTER_TO_SLAVE_CHANGE_POWER_LAMP                                   135

#define CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM                                      131

#define CMD_MASTER_TO_SLAVE_DIM_GROUP                                           132

#define CMD_MASTER_TO_CPU_SYN_RTC_TIME                                          134

#define CMD_CPU_TO_MASTER_KEEP_ALLIVE_CPU                                       137

#define CMD_MASTER_TO_CPU_STATUS_FORCE_CABINET                                  136

#define CMD_MASTER_CPU_TIME_SET_CURRENT                                         130
#define CMD_CPU_TO_MASTER_SCHEDULE_POWER                                        110
#define CMD_CPU_TO_MASTER_SETTING_RTC                                           111
#define CMD_CPU_TO_MASTER_SETTING_SCHEDULE_DIM_ALL_CONTROL                      112
#define CMD_CPU_TO_MASTER_FB_DIM_PORT1                                          113
#define CMD_CPU_TO_MASTER_FB_DIM_PORT2                                          114
#define CMD_CPU_TO_MASTER_MODE_ACTIVE                                           115
#define CMD_CPU_TO_MASTER_SETTINGS_LAMP_JOIN_TO_GROUP_PORT1                     116
#define CMD_CPU_TO_MASTER_SETTINGS_LAMP_JOIN_TO_GROUP_PORT2                     117
#define CMD_CPU_TO_MASTER_CHANGE_POWER_LAMP                                     90

#define CMD_CPU_TO_MASTER_FORCE_ON_CABINET                                      32
#define CMD_CPU_TO_MASTER_STATUS_ACTIVE_SENSOR                                  33

#define CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP                               126

#define CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1                   127
#define CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2                   128

#define CMD_SLAVE_TO_MASTER_STATUS_LAMP					     	                          104


#define ADDR_STORAGE_DIM_ALL_CONTROL                                                   0
#define ADDR_STORAGE_SCHEDULE_DATA                                                  0x10000  
#define ADDR_STORAGE_DIM_PORT_1                                                     0x20000  
#define ADDR_STORAGE_DIM_PORT_2                                                     0x30000  
#define ADDR_STORAGE_SET_SCHEDULE_DIM_GROUP_PORT1                                   0x40000  
#define ADDR_STORAGE_SET_SCHEDULE_DIM_GROUP_PORT2                                   0x50000   
#define ADDR_STORAGE_SET_SCHEDULE_GROUP1                                            0x60000  
#define ADDR_STORAGE_SET_SCHEDULE_GROUP2                                            0x70000  
#define ADDR_STORAGE_SET_SCHEDULE_GROUP3                                            0x80000 

#define ADDR_STORAGE_SET_LAMP_GROUP1_PORT1                                          0x90000
#define ADDR_STORAGE_SET_LAMP_GROUP2_PORT1                                          0xB0000
#define ADDR_STORAGE_SET_LAMP_GROUP3_PORT1                                          0xD0000

#define ADDR_STORAGE_SET_LAMP_GROUP1_PORT2                                          0xA0000
#define ADDR_STORAGE_SET_LAMP_GROUP2_PORT2                                          0xC0000
#define ADDR_STORAGE_SET_LAMP_GROUP3_PORT2                                          0xE0000

#define ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1                                       0xF0000

#define ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2                                       0x100000
#define ADDR_STORAGE_CHANGE_ID_DIM                                                  0x110000

typedef enum
{
    PORT_IDLE = 0,
    TURN_OFF_MASTER,
    TURN_OFF_COUNTER
} portConfigState_t;

typedef enum
{
  TASK_CONTROL_NULL = 0,
  TASK_CONTROL_1,
  TASK_CONTROL_2
}taskControlSchedule_2;

// Trang thai chinh khi moi khoi dong
typedef enum _mtlc_main_state_
{
  mtlc_startup_state = 0,
  mtlc_normal_state,
}_mtlc_main_state_;

typedef enum _mtlc_main_work_state_
{
  mtlc_state_off = 0,
  mtlc_state_auto,
  mtlc_state_manual,
}_mtlc_main_work_state_;

typedef struct __attribute__((packed))
{
  volatile bool isNextNodefbConnect;
  volatile bool forwardCounterfb;
  volatile bool setdimvalue;
  volatile bool setdimid;
  volatile bool setaddress;
  volatile bool countlamp;
  volatile bool forwardCounterfb2;
  volatile bool Istimeout;
  volatile bool setgroup;
  volatile bool dimgroup;
  volatile bool setdimgroup;
  volatile bool setupinfor;
  volatile bool addidtogorup;
  volatile bool forwardCounterfb3;
  volatile bool Istimeoutcounter;
  volatile bool fbdimtocpu;
  volatile bool flatcounterport1;
  volatile bool flatcounterport2;
  volatile bool changeidrf;
  volatile bool countlamprf;
  volatile bool countlamprf_fb;
  volatile bool setRTC;
  volatile bool setScheduleDim;
  volatile bool setdimIdPort1;
  volatile bool setdimIdPort2;
  volatile bool setModeActive;
  volatile bool checkDevStatus;
  volatile bool setdimIdGroupPort1;
  volatile bool setdimIdGroupPort2;
  volatile bool setScheduleGroup;
  volatile bool setChangeIdGroupPort1;
  volatile bool setChangeIdGroupPort2;
  volatile bool is_check_status_sensor;
} type_flagIncom_t;

typedef struct __attribute__((packed))
{
  volatile bool is_ticker_one_seconds_main;
  volatile bool is_ticker_one_seconds_keepallive_cpu; // biến giữ trạng thái kết nối với cpu
  volatile bool is_update_rtc;              // cập nhật thời gian
  volatile bool is_require_update_rtc_ex;   // yeu cau update lai thoi gian thiet bi do bi loi
  volatile bool is_enable_check_time_power; // biến kiểm tra xem thời gian thực đúng
  volatile bool is_response_force_cabinet;
  volatile bool is_progess_force_cabinet;
  volatile bool is_time_active_cabinet;
  volatile bool is_sensor_active_cabinet;
  volatile bool is_ticker_force_cabinet;

  volatile bool is_switch_control;
}type_flag_system_t;

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

typedef struct __attribute__((packed))
{
  uint8_t is_startup_state;
  uint8_t is_read_signal_switch_cpu;
  volatile uint8_t mtlc_main_state;
  volatile uint8_t mtlc_working_state;
}type_mtlc_working_pakage_t;

typedef struct __attribute__((packed))
{
    uint8_t dim_fb[70];
    uint8_t lampport;
    uint8_t group[70];
    uint8_t id[70];
    uint8_t port;
} fb_dim_id_t;

typedef struct __attribute__((packed))
{
    uint8_t dim_fb[70];
    uint8_t lampport;
    uint8_t group[70];
    uint8_t id[70];
    uint8_t port;
} fb_dim_id_update_t;

typedef struct __attribute__((packed)){
	uint8_t hour;
  uint8_t min;
}type_RTC_monitoring_t;

typedef struct __attribute__((packed))
{
  uint8_t hour[9];
  uint8_t min[9];
  uint8_t dim_value[9];
}add_param_control;

typedef struct __attribute__((packed))
{
  uint8_t hh_start[10] ;
  uint8_t mm_start[10];
  uint8_t hh_end[10];
  uint8_t mm_end[10];
  uint8_t size_array_schedule;
  bool flag_schedule = true;
  bool flag_schedule_end = true;
  bool flag_open_all = false;
}add_param_time_schedule_on_off;

typedef struct __attribute__((packed))
{
  uint8_t id;
  uint8_t group;
} type_lightAddConfig_t;

typedef struct __attribute__((packed))
{
  uint16_t val1;
  uint16_t val2;
} type_configPackage_t;

typedef struct __attribute__((packed))
{
    uint8_t dim;
} dim_active_t;

typedef struct __attribute__((packed))
{
  uint8_t add;
	uint8_t dim;
} add_t;

typedef struct __attribute__((packed))
{
  uint8_t id;
  uint8_t port;
  uint8_t group;
  uint8_t dim;
	uint8_t cabinetid;
} add_p_t;

typedef struct __attribute__((packed))
{
  uint8_t group;
  uint8_t dim;
} group_t;

typedef struct __attribute__((packed))
{
	uint8_t oldid;
	uint8_t newid;
	uint8_t cabinetid;
} change_id_t;

// change power with lamp range 0 -> 255
typedef struct __attribute__((packed))
{
	uint8_t value_power_lamp;
} change_power_lamp_t;

typedef struct __attribute__((packed))
{
  int years;
  int month;
  int day;
  int hour;
  int min;
  int sec;
}rtc_time;

typedef struct __attribute__((packed))
{
  uint16_t years;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
}rtc_set;

typedef struct __attribute__((packed))
{
  uint8_t id;
  uint8_t port;
  uint8_t code;
} add_err_t;

typedef struct __attribute__((packed))
{
	int rear;
	int front;
	int inp_arr[100];
} xqueue_t;

typedef struct __attribute__((packed))
{
	uint8_t rear;
	int front;
	uint8_t inp_arr[100];
} xqueue_lamp_port_t;

typedef struct __attribute__((packed))
{
  int group;
	int front;
	int rear;
	uint8_t inp_id[100];
} group_lamp_tmp_recv_cpu_t;

typedef struct __attribute__((packed))
{
	int rear;
  uint8_t inp_id[100];
}change_lamp_to_group_t;

typedef struct __attribute__((packed))
{
  int dim_value;
  uint8_t id_dim[100];
	uint8_t cabinetid;
}add_lamp_dim_group_t;

typedef struct __attribute__((packed))
{
  int hh_start;
  int mm_start;
  int hh_end;
  int mm_end;
	int threshold_ambient_light;
	bool status;
}type_mtlc_sensor_t;

typedef struct __attribute__((packed))
{
  int hh_start;
  int mm_start;
  int hh_end;
  int mm_end;
	int check_time_current;
	uint8_t force_cabinet;
}type_mtlc_force_on_t;

typedef struct __attribute__((packed))
{
  uint8_t force_cabinet_status;
}type_mtlc_data_response_t;

typedef struct __attribute__((packed))
{
  uint8_t id[total_lamp_now];
}add_lamp_dim_to_group_t;

// goi tin 1s gui cpu
typedef struct __attribute__((packed))
{
  time_t rtc_epoch;
}type_mtlc_working_message_t;

//<----------------------------------------Bien lu kieu gio phut
typedef struct __attribute__((packed)){
  uint8_t hour;
  uint8_t minute;
}type__mtfc_hm_time_t;
#endif