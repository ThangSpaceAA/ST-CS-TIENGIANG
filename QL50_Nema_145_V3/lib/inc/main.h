#ifndef _MAIN_H
#define _MAIN_H


#define lamp_total_default                                                      70

#define EEPROM_ADDR           0   //<----------------------------User address Eeprom
#define ADD_SAVE_SETTING      1  //<----------------------------Dia chi luu cau hinh cai dat

#define CMD_CPU_SET_DIM_VALUE                                                 	200
#define CMD_CPU_SET_DIM_ADDRESS                                                 201


#define VALUE_DIM_DEFAULT                                                       100
#define ID_DEFAULT                                                              99
#define PORT_DEFAULT                                                            1
#define GROUP_DEFAULT                                                           200
#define POWER_LAMP_DEFAULT                                                      100

#define CMD_QT_TO_CPU_DIM_ALL                                                   255
#define CMD_CPU_TO_MASTER_DIM_ALL												                        254
#define CMD_CPU_TO_QT_DIM_ALL_SCHEDULE											                    253
#define CMD_QT_LOAD_DIM_ALL_SCHEDULE											                      252
#define CMD_QT_TO_CPU_DIM_ALL_SCHEDULE											                    251
#define CMD_QT_TO_CPU_DIM_ID													                          250
#define CMD_QT_TO_CPU_DIM_ID_DISABLE											                      249
#define CMD_CPU_TO_MASTER_DIM_ID												                        248
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
#define CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP							                        229
#define CMD_MASTER_TO_SLAVE_CHANGE_ID_RF							     	                    216
#define CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB							     	                  215
#define CMD_MASTER_TO_CPU_CHANGE_ID_RF_FB							     	                    214
#define CMD_CPU_TO_QT_CHANGE_ID_RF_FB								     	                      213
#define CMD_QT_TO_CPU_COUNT_LAMP_RF									     	                      212
#define CMD_CPU_TO_MASTER_COUNT_LAMP_RF								     	                    211
#define CMD_MASTER_TO_SLAVE_COUNT_LAMP_RF							     	                    210
#define CMD_SLAVE_TO_MASTER_COUNT_LAMP_RF_FB					     	                    209


#define CMD_MASTER_TO_SLAVE_CHANGE_POWER_LAMP                                   135
#define CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM                                      131
#define CMD_MASTER_TO_SLAVE_DIM_GROUP                                           132

#define ADDR_STORAGE_DIM_CONTROL                                                0
#define ADDR_STORAGE_ADDRESS_LAMP                                             0x10000 
#define ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP                                0x20000 

typedef enum
{
  NORMAL_STATE = 0,
  CONFIG_STATE
} deviceState_t;

typedef enum
{
    PORT_IDLE = 0,
    PORT_COUNTER,
    PORT_SAVE_ADD,
    PORT_SAVE_GROUP
} portConfigState_t;

typedef struct __attribute__((packed))
{
  volatile bool isEnterConfigMode;
  volatile bool isExitConfigMode;
  volatile bool isCounterAddLamp;
  volatile bool isTimeOutConterLamp;
  volatile bool isNextNodefbConnect;
  volatile bool forwardCounterfb;
  volatile bool IsDimAdd;
  volatile bool IsDimGroup;
  volatile bool IsDimListGroup;  
  volatile bool IsSaveAdd;
  volatile bool IsSaveGroup;
  volatile bool IsSaveAddTimeout;
  volatile bool forwardAddSave;
  volatile bool IsSaveGroupTimeout;
  volatile bool forwardGroupSave;
  volatile bool saveEepAdd;
  volatile bool saveEepGroup;

  /////////////
  volatile bool setdimvalue;
  volatile bool setdimid;
  volatile bool setdimgroup;
  volatile bool setdimaddress;
  volatile bool setaddress;
  volatile bool countlamp;
  volatile bool setgroup;
  volatile bool dimgroup;
  volatile bool setupinfor;
  volatile bool addidtogorup;
  //////////////////
  volatile bool forwardID;
  volatile bool changeidrf;
  volatile bool countlamprf;
} type_flagIncom_t;



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


typedef struct __attribute__((packed)){
	uint16_t 		add;
	uint8_t 	  val;
}type_lightDimmerPackge_t;

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

typedef struct __attribute__((packed))
{
  int dim_value;
  uint8_t id_dim[100];
	uint8_t cabinetid;
}add_lamp_dim_group_t;

typedef struct __attribute__((packed))
{
	uint8_t value_power_lamp;
} change_power_lamp_t;
#endif