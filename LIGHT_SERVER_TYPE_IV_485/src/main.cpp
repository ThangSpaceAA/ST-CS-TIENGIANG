#include "mbed.h"

#include "debug.h"
#include "main.h"
#include "millis.h"
#include "lightAddCom.h"
#include "singleMSP.h"
#include "eeprom.h"
#include "ds3231.h"
#include "w25q32.h"
#include "flash.h"
#include <iwdg.h>
#include <rtc_api_cus.h>

#pragma region[Declare OOP objects---------------------------------------------------------------------------]
DigitalOut ledAct(PA_10);
DigitalOut relay1(PA_9);
DigitalOut relay2(PA_8);
DigitalOut relay3(PC_9);
DigitalInOut ss(PA_4);

Serial com2(PC_6, PC_7);
Serial com1(PA_0, PA_1);
Serial com3(PB_10, PB_11);
Serial com4(PC_12, PD_2);
Serial com5(PA_2, PA_3);
Timeout timeout;

I2C i2c(PB_7, PB_6);
DS3231 rtc_cus(PB_7, PB_6);
rtc mtfc_rtc(PB_7, PB_6, PB_8);

W25Q32 mem(PA_7, PA_6, PA_5, PA_4);

const int FlashChipSelect = PA_4; // digital pin for flash chip CS pin

lightAddCom inCom(com1, com5, com4, 115200);
lightAddCom outCom(com3, com2, com1, 115200);
lightAddCom out2Com(com2, com3, com1, 115200);
lightAddCom out3Com(com5, com3, com1, 115200);
singleMSP outCom5(com5, 115200);
singleMSP outCom3(com3, 115200);
singleMSP outCom2(com2, 115200);
singleMSP outCom1(com1, 115200);

EEPROM memory(PB_7, PB_6, EEPROM_ADDR, EEPROM::T24C02); // SDA,SCL

#pragma endregion

#pragma region[Declare Functions-----------------------------------------------------------------------------]
void send_time_to_cpu_monitoring(type__mtfc_hm_time_t *mtfc_hm_present, type_RTC_monitoring_t *mtlc_time_master_processing);
void initizaline_all_lamp_in_branch(void);
void reset_lamp_group_all_in_branch1(void);
void reset_lamp_group_all_in_branch2(void);
void compare_lamp_group_with_group_all(change_lamp_to_group_t number_group_port, change_lamp_to_group_t group_all_port, int port);
void control_relay(int relay_out);
/**
 * @brief dim all and dim follow id
 */
void control_dim_lamp_all(void);

void control_dim_group(dim_active_schedule_t *dim_schedules, add_lamp_dim_group_t *add_id_group_dim_port1, add_lamp_dim_group_t *add_id_group_dim_port2, uint8_t cmd_dim_group);
/**
 *
 */
void control_power_slave_dim(type__mtfc_hm_time_t *mtlc_time_present);
/**
 * @brief save and read data storage
 */
void get_all_settings_storage(void);

/**
 * @brief setup and get time, day from RTC DS3231
 */
void getCurrentRTC(void);

/**
 * @brief scan i2c
 */
void ScanI2C(void);

/**
 * @brief Chop led trang thai: flashing hoat dong binh thuong- soild che do cai dat dia chi
 *
 */
void mtlc_led_status_processing(void);

/**
 * @brief Ngắt nhận dữ liệu cổng nối tiếp vào
 *
 * @param siz số byte nhận được
 */
void inComReceiveEventHandler(uint8_t siz);

/**
 * @brief Ngắt nhận dữ liệu cổng nối tiếp vào
 *
 * @param siz số byte nhận được
 */
void outComReceiveEventHandler(uint8_t siz);

void out2ComReceiveEventHandler(uint8_t siz);

void out3ComReceiveEventHandler(uint8_t siz);

void mtlc_update_mtlc_parameters(void);

void mtlc_output_mtlc_parameters(void);

/**
 *
 */
void insert_queue_lamp_port(uint8_t insert_item, int port);

/**
 *
 */
void descrease_queue_lamp_port(int port);

/**
 *
 */
void show_queue_lamp_port(int port);
/**
 * @brief Đoc cài đặt cấu hình của hiết bị
 *
 * @return type_lightAddConfig_t
 */
add_p_t ReadMemory(void);
/**
 * @brief clear bien tam
 */
void mtlc_memory_startup_config(void);
/**
 * @brief lay du lieu rtc 1s 1lan
 */
void mtlc_main_ticker_one_seconds(void);
/**
 * @brief kiem tra trang thai ket noi cpu
 */
void mtlc_keepallive_cpu_ticker_one_seconds(void);
/**
 * @brief check trang thai bien 1s 1lan
 */
void mtlc_one_seconds_ticker_event_handler(void);
/**
 * @brief cap nhat lai rtc
 */
void mtlc_rtc_update(type_date_time_t *t);
/**
 * @brief cuong che tu bat theo cam bien anh sang
 */
void mtlc_force_on_cabinet(type_mtlc_force_on_t *force_cabinet, type_flag_system_t *flag_system, type__mtfc_hm_time_t *time_current);

/**
 * @brief khoi tao cac thong so dau tien cho he thong
 */
void mtlc_initialize_parameters(type_mtlc_working_pakage_t *obj);
/**
 * @brief cac hoat dong dieu khien den
 */
void mtlc_normal_master_processing(void);

void timeOutProcessing(void);
void timeOutEnabled(float t, uint8_t stateSet);
void timeOutDisable(void);
// Xử lý các lệnh dim công suất đèn
void DimLampProcessing(void);
void dimmerLamp(uint8_t val);
void updateConfig(void);

#pragma endregion

#pragma region[STRUCT VARIABLES------------------------------------------------------------------------------]
/**
 * @brief Cờ báo tín hiệu cổng dữ liệu vào
 *
 */
type_flagIncom_t flagIncom;

/**
 * @brief Lưu thông tin của bộ định địa chỉ
 *
 */
type_lightAddConfig_t lightAddConfig;
type_lightAddConfig_t lightAddConfigBuff;

/**
 * @brief gói tin cài đặt
 *
 */
type_configPackage_t counters_package;
type_configPackage_t setGroup_package;

/**
 * @brief struct mang thong so cap nhat rtc_cus
 */
type_flag_system_t mtfc_flag_system;

type_lightAddConfig_t type_lightAddConfig;
dim_active_t dim_active;
add_t add;
add_p_t add_p;
add_p_t add_defaut;
add_p_t add_defaut2;

add_p_t change_id_to_group = {};

add_err_t add_err_node;
group_t group;
change_id_t change_id;
add_p_t add_fb;
rtc_time add_rtc;
rtc_set rtc_set_master;
add_param_control meter_control;
add_param_time_schedule_on_off meter_schedule;
dim_active_schedule_t dim_active_schedule_control;

dim_active_schedule_t add_dim_schedule_group;

dim_active_schedule_t dim_schedule_group1;
dim_active_schedule_t dim_schedule_group2;
dim_active_schedule_t dim_schedule_group3;

fb_dim_id_t fb_dim_port1;
fb_dim_id_t fb_dim_port2;
fb_dim_id_update_t fb_dim_update_port1;
fb_dim_id_update_t fb_dim_update_port2;

xqueue_t queue;
xqueue_lamp_port_t queue_lamps_port1;
xqueue_lamp_port_t queue_lamps_port2;

group_lamp_tmp_recv_cpu_t lamp_recv_cpu_port1 = {};
group_lamp_tmp_recv_cpu_t lamp_recv_cpu_port2 = {};
change_lamp_to_group_t insert_lamp_to_group = {};
change_lamp_to_group_t lamp_group_port1;
change_lamp_to_group_t lamp_group_port2;

change_lamp_to_group_t lamp_group_all_p1;
change_lamp_to_group_t lamp_group_all_p2;

change_lamp_to_group_t lamp_group1_p1;
change_lamp_to_group_t lamp_group1_p2;
change_lamp_to_group_t lamp_group2_p1;
change_lamp_to_group_t lamp_group2_p2;
change_lamp_to_group_t lamp_group3_p1;
change_lamp_to_group_t lamp_group3_p2;

add_lamp_dim_group_t out_dim_lamp_p1;
add_lamp_dim_group_t out_dim_lamp_p2;

add_lamp_dim_group_t out_dim_lamp_gr1_p1;
add_lamp_dim_group_t out_dim_lamp_gr1_p2;
add_lamp_dim_group_t out_dim_lamp_gr2_p1;
add_lamp_dim_group_t out_dim_lamp_gr2_p2;
add_lamp_dim_group_t out_dim_lamp_gr3_p1;
add_lamp_dim_group_t out_dim_lamp_gr3_p2;

// luu thoi gian cai dat gio he thong
type_date_time_t mtfc_rtc_new_update;

type__mtfc_hm_time_t mtfc_hm_present;
type_RTC_monitoring_t mtlc_time_master_processing;
// luu thoi gian hien tai cua he thong
type_date_time_t mtfc_rtc_val;
// thay doi cong suat den
change_power_lamp_t change_lamp_power;

Ticker mtfc_one_seconds_ticker;
// cuong che bat tu theo cam bien (1)
// (1) cam bien anh sang
type_mtlc_force_on_t mtlc_force_cabinet;
type_mtlc_sensor_t mtlc_sensor_allive;

type_mtlc_data_response_t mtlc_system_response;

// goi tin 1s gui cpu
type_mtlc_working_message_t mtlc_working_message;
// goi tin lam viec
type_mtlc_working_pakage_t mtlc_working_current_base;

#pragma endregion

#pragma region[Declare variable------------------------------------------------------------------------------]
volatile uint8_t stateTimeOut = PORT_IDLE;
volatile uint8_t taskControlschedule2 = TASK_CONTROL_1;

volatile bool flag_task_ctrlRL_1 = false;
volatile bool flag_task_ctrlRL_2 = false;

volatile int keepalliveCPU;

uint8_t tempAddress = 0;
uint8_t datDimListGroup[11];
uint8_t maxDeviceCount = 0;
uint8_t dataForwardAddSave = 0;
uint8_t dataForwardGroup = 0;
uint8_t countlamprf = 0;
uint8_t a = 0;
uint8_t Mode_Active = 1;

uint8_t tmp_current_time_set_hh = 0;
uint8_t tmp_current_time_set_mm = 0;

#pragma endregion

#pragma region[Interrupts Timer------------------------------------------------------------------------------]

#pragma endregion

#pragma region[INTERRUPTS INCOM------------------------------------------------------------------------------]
void inComReceiveEventHandler(uint8_t siz)
{
  // debug(MAIN_DEBUG, "\r\ninCom.get_cmd(): %d\r\n", inCom.get_cmd());
  switch (inCom.get_cmd())
  {
  case CMD_CPU_TO_MASTER_DIM_ALL:
    inCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    add_p.cabinetid = change_id.cabinetid;
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    // flagIncom.setdimvalue = true;
    break;
  case CMD_CPU_TO_MASTER_DIM_ID:
    inCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setdimid = true;
    break;
  case CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT1:
    inCom.readstruct((uint8_t *)&change_id_to_group, sizeof(add_p_t));
    flagIncom.setChangeIdGroupPort1 = true;
    break;
  case CMD_CPU_TO_MASTER_SETTINGS_CHANGE_ID_LAMP_GROUP_PORT2:
    inCom.readstruct((uint8_t *)&change_id_to_group, sizeof(add_p_t));
    flagIncom.setChangeIdGroupPort2 = true;
    break;
  case CMD_CPU_TO_MASTER_COUNT_LAMP:
    counters_package.val1 = 0;
    outCom1.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    outCom1.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    inCom.readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.countlamp = true;
    flagIncom.flatcounterport1 = true;
    break;
  case CMD_CPU_TO_MASTER_SETUP_INFO:
    inCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setupinfor = true;
    break;
  case CMD_CPU_TO_MASTER_ADD_ID_TO_GROUP:
    inCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.addidtogorup = true;
    break;
  case CMD_CPU_TO_MASTER_POWER_CONTROL:
    debug(MAIN_DEBUG, " data cpu to master: %d\r\n", inCom.read8());
    relay1 = inCom.read8();
    relay2 = relay1;
    relay3 = relay1;
    break;
  case LSV_NODE_CONNECT_FB:
    flagIncom.isNextNodefbConnect = true;
    break;
  case CMD_CPU_SET_GROUP:
    inCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setgroup = true;
    break;
  case CMD_CPU_DIM_GROUP:
    inCom.readstruct((uint8_t *)&group, sizeof(group_t));
    flagIncom.dimgroup = true;
    break;
  case CMD_CPU_TO_MASTER_CHANGE_ID_RF:
    inCom.readstruct((uint8_t *)&change_id, sizeof(change_id_t));
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_CHANGE_ID_RF, (uint8_t *)&change_id, sizeof(change_id_t));
    SerialFlash.eraseBlock(ADDR_STORAGE_CHANGE_ID_DIM);
    SerialFlash.write(ADDR_STORAGE_CHANGE_ID_DIM, (char *)&change_id, sizeof(change_id_t));
    flagIncom.changeidrf = true;
    break;
  case CMD_CPU_TO_MASTER_CHANGE_POWER_LAMP:
    inCom.readstruct((uint8_t *)&change_lamp_power, sizeof(change_power_lamp_t));
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_CHANGE_POWER_LAMP, (uint8_t *)&change_lamp_power, sizeof(change_power_lamp_t));
    debug(MAIN_DEBUG, "got value power change lamp: %d\r\n", change_lamp_power.value_power_lamp);
    break;
  case CMD_CPU_TO_MASTER_COUNT_LAMP_RF:
    countlamprf = inCom.read8();
    flagIncom.countlamprf = true;
    break;
  case CMD_CPU_TO_MASTER_SCHEDULE_POWER:
    inCom.readstruct((uint8_t *)&meter_schedule, sizeof(meter_schedule));
    SerialFlash.eraseBlock(ADDR_STORAGE_SCHEDULE_DATA);
    SerialFlash.write(ADDR_STORAGE_SCHEDULE_DATA, (char *)&meter_schedule, sizeof(meter_schedule));
    wait(0.01);
    for (int i = 0; i < 2; i++)
    {
      debug(MAIN_DEBUG, "hour start: %d, min start: %d , hour end: %d, min end: %d, size of array schedule: %d\r\n", meter_schedule.hh_start[i], meter_schedule.mm_start[i],
            meter_schedule.hh_end[i], meter_schedule.mm_end[i], meter_schedule.size_array_schedule);
    }
    break;
  case CMD_CPU_TO_MASTER_SETTING_RTC:
    inCom.readstruct((uint8_t *)&mtfc_rtc_new_update, sizeof(type_date_time_t));
    mtfc_flag_system.is_update_rtc = true;
    break;
  case CMD_CPU_TO_MASTER_SETTING_SCHEDULE_DIM_ALL_CONTROL: // đoi ten nay lai dung voi nhiem vu la cap nhat thoi gian dim theo lich
    inCom.readstruct((uint8_t *)&dim_active_schedule_control, sizeof(dim_active_schedule_t));
    flagIncom.setScheduleDim = true;
    break;
  case CMD_CPU_TO_MASTER_FB_DIM_PORT1:
    inCom.readstruct((uint8_t *)&fb_dim_update_port1, sizeof(fb_dim_id_update_t));
    // for (int i = 0; i < total_lamp_now; i++)
    // {
    //   debug(MAIN_DEBUG, "dim fb: %d, lamp port: %d, group: %d, id: %d, PORT: %d\r\n", fb_dim_update_port1.dim_fb[i],
    //         fb_dim_update_port1.lampport, fb_dim_update_port1.group[i], fb_dim_update_port1.id[i], fb_dim_update_port1.port);
    // }
    debug(MAIN_DEBUG, "%s\r\n", "PORT1-----------------------------------------");
    flagIncom.setdimIdPort1 = true;
    break;
  case CMD_CPU_TO_MASTER_FB_DIM_PORT2:
    inCom.readstruct((uint8_t *)&fb_dim_update_port2, sizeof(fb_dim_id_update_t));
    // for (int i = 0; i < total_lamp_now; i++)
    // {
    //   debug(MAIN_DEBUG, "dim fb: %d, lamp port: %d, group: %d, id: %d, PORT: %d\r\n", fb_dim_update_port2.dim_fb[i],
    //         fb_dim_update_port2.lampport, fb_dim_update_port2.group[i], fb_dim_update_port2.id[i], fb_dim_update_port2.port);
    // }
    debug(MAIN_DEBUG, "%s\r\n", "PORT2-----------------------------------------");
    flagIncom.setdimIdPort2 = true;
    break;
  case CMD_CPU_TO_MASTER_MODE_ACTIVE:
    mtlc_working_current_base.is_read_signal_switch_cpu = inCom.read8();
    debug(MAIN_DEBUG, "Mode active: %d\r\n", mtlc_working_current_base.is_read_signal_switch_cpu);
    mtfc_flag_system.is_switch_control = true;
    break;
  case CMD_CPU_TO_MASTER_SETTINGS_LAMP_JOIN_TO_GROUP_PORT1:
    inCom.readstruct((uint8_t *)&lamp_recv_cpu_port1, sizeof(group_lamp_tmp_recv_cpu_t));
    debug(MAIN_DEBUG, "Recevie lamp from cpu group PORT1 GROUP: %d\r\n", lamp_recv_cpu_port1.group);
    debug(MAIN_DEBUG, "Recevie lamp from cpu group PORT1 REAR: %d\r\n", lamp_recv_cpu_port1.rear);
    flagIncom.setdimIdGroupPort1 = true;
    break;
  case CMD_CPU_TO_MASTER_SETTINGS_LAMP_JOIN_TO_GROUP_PORT2:
    inCom.readstruct((uint8_t *)&lamp_recv_cpu_port2, sizeof(group_lamp_tmp_recv_cpu_t));
    debug(MAIN_DEBUG, "Recevie lamp from cpu group PORT2 GROUP: %d\r\n", lamp_recv_cpu_port2.group);
    debug(MAIN_DEBUG, "Recevie lamp from cpu group PORT2 REAR: %d\r\n", lamp_recv_cpu_port2.rear);
    flagIncom.setdimIdGroupPort2 = true;
    break;
  case CMD_CPU_TO_MASTER_SETTINGS_SCHEDULE_GROUP:
    inCom.readstruct((uint8_t *)&add_dim_schedule_group, sizeof(dim_active_schedule_t));
    flagIncom.setScheduleGroup = true;
    break;
  case CMD_CPU_TO_MASTER_FORCE_ON_CABINET:
    inCom.readstruct((uint8_t *)&mtlc_force_cabinet, sizeof(type_mtlc_force_on_t));
    mtfc_flag_system.is_response_force_cabinet = true;
    mtfc_flag_system.is_progess_force_cabinet = true;
    break;
  case CMD_CPU_TO_MASTER_STATUS_ACTIVE_SENSOR:
    inCom.readstruct((uint8_t *)&mtlc_sensor_allive, sizeof(type_mtlc_sensor_t));
    flagIncom.is_check_status_sensor = true;
    break;
  case CMD_CPU_TO_MASTER_KEEP_ALLIVE_CPU:
    keepalliveCPU = keepalliveCPU - 5;
    if(keepalliveCPU < 0)
    {
      keepalliveCPU = 0;
    }
    break; 
  default:
    break;
  }
}
#pragma endregion

#pragma region[INTERRUPTS OUTCOM-----------------------------------------------------------------------------]
// Port 2
void outComReceiveEventHandler(uint8_t siz)
{
  // debug(MAIN_DEBUG, "\r\nforward Counter PORT 2 fb = %d", counters_package.val1);
  debug(MAIN_DEBUG, "\r\nget out com PORT 2 fb = %d", outCom.get_cmd());
  switch (outCom.get_cmd())
  {
  case LSV_NODE_END_NUMLAMP_FB:
    outCom.readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.forwardCounterfb = true;
    flagIncom.flatcounterport2 = 1;
    break;
  case CMD_SLAVE_FB_DIM_TO_MASTER:
    outCom.readstruct((uint8_t *)&add_p, sizeof(add_p));
    debug(MAIN_DEBUG, "\r\nforward id = %d", add_p.id);
    debug(MAIN_DEBUG, "\r\nforward port = %d", add_p.port);
    debug(MAIN_DEBUG, "\r\nforward dim = %d", add_p.dim);
    flagIncom.fbdimtocpu = true;
  default:
    break;
  }
}
// Port 1
void out2ComReceiveEventHandler(uint8_t siz)
{
  debug(MAIN_DEBUG, "\r\nforward Counter PORT 1 fb = %d", out2Com.get_cmd());
  debug(MAIN_DEBUG, "\r\nget out com PORT 1 fb = %d", out2Com.get_cmd());
  switch (out2Com.get_cmd())
  {
  case LSV_NODE_END_NUMLAMP_FB:
    out2Com.readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.forwardCounterfb2 = true;
    break;
  case CMD_SLAVE_FB_DIM_TO_MASTER:
    out2Com.readstruct((uint8_t *)&add_p, sizeof(add_p));
    debug(MAIN_DEBUG, "\r\nforward id = %d", add_p.id);
    debug(MAIN_DEBUG, "\r\nforward port = %d", add_p.port);
    debug(MAIN_DEBUG, "\r\nforward dim = %d", add_p.dim);
    flagIncom.fbdimtocpu = true;
    break;
  case CMD_SLAVE_TO_MASTER_STATUS_LAMP:
    out2Com.readstruct((uint8_t *)&add_err_node, sizeof(add_err_t));
    debug(MAIN_DEBUG, "\r\nid = %d", add_err_node.id);
    debug(MAIN_DEBUG, "\r\nport = %d", add_err_node.port);
    debug(MAIN_DEBUG, "\r\ncode = %d\r\n", add_err_node.code);
    break;
  default:
    break;
  }
}

void out3ComReceiveEventHandler(uint8_t siz)
{
  debug(MAIN_DEBUG, "\r\nforward Counter RF333333333333 fb = %d", out3Com.get_cmd());
  switch (out3Com.get_cmd())
  {
  case LSV_NODE_END_NUMLAMP_FB:
    out3Com.readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.forwardCounterfb3 = true;
    break;
  case CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB:
    // a = out3Com.read8();
    // inCom.send_byte(CMD_MASTER_TO_CPU_CHANGE_ID_RF_FB, a);
    break;
  case CMD_SLAVE_TO_MASTER_COUNT_LAMP_RF_FB:
    out3Com.readstruct((uint8_t *)&add_fb, sizeof(add_p_t));
    flagIncom.countlamprf_fb = true;
    break;
  default:
    break;
  }
}
#pragma endregion

#pragma region[INTERRUPTS LAMPCOM----------------------------------------------------------------------------]

void mtfc_watdog_init(float time_wd)
{
  debug(USING_WATCHDOG_TIMER, "\r\nmtfc watchdog reload: %0.1f (s)", time_wd);
  uint16_t time_wd_reload = (uint16_t)((time_wd * 40000.0) / 256);
  debug(USING_WATCHDOG_TIMER, "\r\nmtfc watchdog init reload: %d", time_wd_reload);
}
#pragma endregion

#pragma region[CHUONG TRINH CHINH----------------------------------------------------------------------------]

void StartUpConfig(void)
{
  inCom.attach(inComReceiveEventHandler);
  outCom.attach(outComReceiveEventHandler);
  out2Com.attach(out2ComReceiveEventHandler);
  out3Com.attach(out3ComReceiveEventHandler);

  control_relay(0);

  com1.printf("\r\nCOM1\r\n");
  com2.printf("\r\nCOM2\r\n");
  com3.printf("\r\nCOM3\r\n");
  com4.printf("\r\nCOM4\r\n");
  com5.printf("\r\nCOM5\r\n");
  int rc;

  // khởi tạo flash
  rc = SerialFlash.begin(FlashChipSelect);
  if (!rc)
  {
    while (1)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Unable to access SPI Flash chip");
    }
  }
  else
  {
    debug(MAIN_DEBUG, "%s\r\n", "Initizaline SPI chip Success");
  }

  // khởi tạo rtc ds3231
  if(mtfc_rtc.rtc_ic_init() != 0){
    debug(MAIN_DEBUG, "%s\r\n", "RTC startup failed");
  }
  else {
    debug(MAIN_DEBUG, "%s\r\n", "RTC startup success");
  }
  wait(1);
  // clear bien tam
  mtlc_memory_startup_config();
  mtlc_initialize_parameters(&mtlc_working_current_base);
  initizaline_all_lamp_in_branch();
  get_all_settings_storage();
  millisStart();
  wait(1.0);
}

int main()
{
  StartUpConfig();
  mtfc_one_seconds_ticker.attach(mtlc_one_seconds_ticker_event_handler, 1.0);
  mtfc_rtc_val = mtfc_rtc.rtc_get_time();

  mtfc_flag_system.is_time_active_cabinet = true;
  mtfc_flag_system.is_sensor_active_cabinet = true;

  keepalliveCPU = 0;

  debug(MAIN_DEBUG, "\r\nSET RTC: %02d %02d %02d - %d %02d/%02d/%04d",
        mtfc_rtc_val.hour,
        mtfc_rtc_val.minute,
        mtfc_rtc_val.seconds,
        mtfc_rtc_val.day,
        mtfc_rtc_val.date,
        mtfc_rtc_val.month,
        mtfc_rtc_val.year);

  if (mtfc_rtc_val.year < 2025)
  {
    debug(MAIN_DEBUG, "%s\r\n", "Error rtc time check hardware?");
    mtfc_flag_system.is_require_update_rtc_ex = true;
  }
#ifdef USING_WATCHDOG_TIMER
  // mtfc_watdog_init(3.0);
#endif 
  while (true)
  {
    mtlc_main_ticker_one_seconds();

    mtlc_keepallive_cpu_ticker_one_seconds();

    switch (mtlc_working_current_base.mtlc_main_state)
    {
    case mtlc_normal_state:
      mtlc_normal_master_processing();
      break;
    default:
      break;
    }
    mtlc_update_mtlc_parameters();  // Xử lý các vấn đề liên quan đến dữ liệu cổng com vào
    mtlc_output_mtlc_parameters(); // Xử lý các vấn đề liên quan đến dữ liệu cổng com ra
    mtlc_led_status_processing(); // đèn flashing trạng thái hoạt động
#ifdef USING_WATCHDOG_TIMER
    // iwdg_feed();
#endif
  }
}

#pragma endregion

#pragma region[INCOM PROCESSING------------------------------------------------------------------------------]
void mtlc_update_mtlc_parameters(void)
{
  if (flagIncom.setdimvalue)
  {
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    debug(MAIN_DEBUG, "%d, %d, %d, %d\r\n", add_p.port, add_p.id, add_p.group, add_p.dim);
    flagIncom.setdimvalue = false;
  }

  if (flagIncom.setdimid)
  {
    flagIncom.setdimid = false;
    if (add_p.port == 2)
    {
      outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
    }
    if (add_p.port == 1)
    {
      outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
    }
    debug(MAIN_DEBUG, "id: %d, port: %d, group: %d, dim: %d\r\n", add_p.id, add_p.port, add_p.group, add_p.dim);
    add_p.cabinetid = change_id.cabinetid;
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
  }

  if (flagIncom.setupinfor)
  {
    // add.add = add_p.id;
    // add.dim = add_p.dim;
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_SETUP_INFO, (uint8_t *)&add_p, sizeof(add_p_t));
    add_p.port = 2;
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_SETUP_INFO, (uint8_t *)&add_p, sizeof(add_p_t));
    add_p.port = 1;
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_SETUP_INFO, (uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setupinfor = false;
  }

  if (flagIncom.addidtogorup)
  {
    if (add_p.port == 2)
    {
      outCom3.send_struct(CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));
    }
    if (add_p.port == 1)
    {
      outCom2.send_struct(CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));
    }
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.addidtogorup = false;
  }

  if (flagIncom.setdimgroup)
  {
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_GROUP, (uint8_t *)&add_p, sizeof(add_p_t));

    flagIncom.setdimgroup = false;
  }

  if (flagIncom.countlamp)
  {
    // Count port 1
    debug(MAIN_DEBUG, "\r\n%s\r\n", "Bat dau dem den o port 1");
    counters_package.val1 = 0;
    outCom2.send_struct(CMD_MASTER_TO_SLAVE_COUNT_LAMP, (uint8_t *)&counters_package, sizeof(type_configPackage_t));

    // Count port RF
    counters_package.val1 = 0;
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_COUNT_LAMP, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.countlamp = false;
  }

  if (flagIncom.flatcounterport1 == 1)
  {
    // Count port 2
    debug(MAIN_DEBUG, "\r\n%s\r\n", "Bat dau dem den o port 2");
    counters_package.val1 = 0;
    outCom3.send_struct(CMD_MASTER_TO_SLAVE_COUNT_LAMP, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.flatcounterport1 = 0;
  }

  if (flagIncom.isNextNodefbConnect)
  {
    timeOutDisable();
    flagIncom.isNextNodefbConnect = false;
  }

  if (flagIncom.setgroup)
  {
    if (add_p.port == 2)
    {
      outCom3.send_struct(155, (uint8_t *)&add_p, sizeof(add_p_t));
      com3.printf("Port %d Add %d Group %d\n", add_p.port, add_p.id, add_p.group);
    }
    if (add_p.port == 1)
    {
      outCom2.send_struct(155, (uint8_t *)&add_p, sizeof(add_p_t));
      com2.printf("Port %d Add %d Group %d\n", add_p.port, add_p.id, add_p.group);
    }
    outCom5.send_struct(155, (uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setgroup = false;
  }

  if (flagIncom.dimgroup)
  {
    outCom3.send_struct(156, (uint8_t *)&group, sizeof(group_t));
    outCom2.send_struct(156, (uint8_t *)&group, sizeof(group_t));
    outCom5.send_struct(156, (uint8_t *)&group, sizeof(group_t));
    flagIncom.dimgroup = false;
  }

  if (flagIncom.changeidrf)
  {
    outCom5.send_struct(CMD_MASTER_TO_SLAVE_CHANGE_ID_RF, (uint8_t *)&change_id, sizeof(change_id_t));
    flagIncom.changeidrf = false;
  }
 
  if (flagIncom.countlamprf)
  {
    debug(MAIN_DEBUG, "\r\n%s:%d", "Bat dau dem den rf thu", countlamprf);
    outCom5.send_byte(CMD_MASTER_TO_SLAVE_COUNT_LAMP_RF, countlamprf);
    flagIncom.countlamprf = false;
  }

  if (flagIncom.countlamprf_fb)
  {
    // debug(MAIN_DEBUG, "\r\n%s:%d", "Phan hoi den rf thu", add_fb.id, add_fb.dim);
    inCom.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_RF_FB, (uint8_t *)&add_fb, sizeof(add_p_t));
    flagIncom.countlamprf_fb = false;
  }

  if (mtfc_flag_system.is_update_rtc)
  {
    mtlc_rtc_update(&mtfc_rtc_new_update);
  }

  if(flagIncom.is_check_status_sensor)
  {
    flagIncom.is_check_status_sensor = false;
    if(mtlc_sensor_allive.status)
    {
      mtfc_flag_system.is_sensor_active_cabinet = true;
    }
    else{
      mtfc_flag_system.is_sensor_active_cabinet = false;
      mtfc_flag_system.is_time_active_cabinet = true;
    }
  }
  
  if (flagIncom.setRTC)
  {
    flagIncom.setRTC = false;
    debug(MAIN_DEBUG, "Set RTC: %d, %d, %d, %d, %d, %ds\r\n", rtc_set_master.years, rtc_set_master.month,
          rtc_set_master.day, rtc_set_master.hour, rtc_set_master.min, rtc_set_master.sec);
    if (rtc_set_master.years != 2000 && rtc_set_master.month != 0 && rtc_set_master.day != 0)
    {
      debug(MAIN_DEBUG, "%s\r\n", "set RTC Success");
      rtc_cus.setDate({}, rtc_set_master.day, rtc_set_master.month, rtc_set_master.years);
      rtc_cus.setTime(rtc_set_master.hour, rtc_set_master.min, rtc_set_master.sec);
    }
    wait(1);
  }
  
  if (flagIncom.setScheduleDim)
  {
    flagIncom.setScheduleDim = false;
    SerialFlash.eraseBlock(ADDR_STORAGE_DIM_ALL_CONTROL);
    SerialFlash.write(ADDR_STORAGE_DIM_ALL_CONTROL, (char *)&dim_active_schedule_control, sizeof(dim_active_schedule_control));
    wait(0.01);
    SerialFlash.read(ADDR_STORAGE_DIM_ALL_CONTROL, (char *)&dim_active_schedule_control, sizeof(dim_active_schedule_control));
    debug(MAIN_DEBUG, "group: %d, dim1: %d, dim2: %d, dim3: %d, dim4: %d, dim5: %d, dim6: %d, dim7: %d, dim8: %d, dim9: %d\r\n",
          dim_active_schedule_control.group, dim_active_schedule_control.dim1, dim_active_schedule_control.dim2,
          dim_active_schedule_control.dim3, dim_active_schedule_control.dim4, dim_active_schedule_control.dim5,
          dim_active_schedule_control.dim6, dim_active_schedule_control.dim7, dim_active_schedule_control.dim8,
          dim_active_schedule_control.dim9);

    debug(MAIN_DEBUG, "hh1: %d, hh2: %d, hh3: %d, hh4: %d, hh5: %d, hh6: %d, hh7: %d, hh8: %d, hh9: %d\r\n",
          dim_active_schedule_control.hh_start_dim1, dim_active_schedule_control.hh_start_dim2, dim_active_schedule_control.hh_start_dim3,
          dim_active_schedule_control.hh_start_dim4, dim_active_schedule_control.hh_start_dim5, dim_active_schedule_control.hh_start_dim6,
          dim_active_schedule_control.hh_start_dim7, dim_active_schedule_control.hh_start_dim8, dim_active_schedule_control.hh_start_dim9);

    debug(MAIN_DEBUG, "mm1: %d, mm2: %d, mm3: %d, mm4: %d, mm5: %d, mm6: %d, mm7: %d, mm8: %d, mm9: %d\r\n",
          dim_active_schedule_control.mm_start_dim1, dim_active_schedule_control.mm_start_dim2, dim_active_schedule_control.mm_start_dim3,
          dim_active_schedule_control.mm_start_dim4, dim_active_schedule_control.mm_start_dim5, dim_active_schedule_control.mm_start_dim6,
          dim_active_schedule_control.mm_start_dim7, dim_active_schedule_control.mm_start_dim8, dim_active_schedule_control.mm_start_dim9);
  }
  
  if (flagIncom.setdimIdPort1)
  {
    flagIncom.setdimIdPort1 = false;
    for (int i = 0; i < total_lamp_now; i++)
    {
      if ((fb_dim_port1.dim_fb[i] != fb_dim_update_port1.dim_fb[i] || fb_dim_port1.group[i] != fb_dim_update_port1.group[i] ||
           fb_dim_port1.id[i] != fb_dim_update_port1.id[i]) &&
          fb_dim_update_port1.group[i] != 0 && fb_dim_update_port1.id[i] != 0)
      {
        fb_dim_port1.dim_fb[i] = fb_dim_update_port1.dim_fb[i];
        fb_dim_port1.group[i] = fb_dim_update_port1.group[i];
        fb_dim_port1.id[i] = fb_dim_update_port1.id[i];
      }
    }
    SerialFlash.eraseBlock(ADDR_STORAGE_DIM_PORT_1);
    SerialFlash.write(ADDR_STORAGE_DIM_PORT_1, (char *)&fb_dim_port1, sizeof(fb_dim_id_t));
    // for (int i = 0; i < total_lamp_now; i++)
    // {
    //   debug(MAIN_DEBUG, "dim fb: %d, lamp port: %d, group: %d, id: %d, port: %d\r\n", fb_dim_update_port1.dim_fb[i],
    //         fb_dim_update_port1.lampport, fb_dim_update_port1.group[i], fb_dim_update_port1.id[i], fb_dim_update_port1.port);
    // }
  }
  
  if (flagIncom.setdimIdPort2)
  {
    flagIncom.setdimIdPort2 = false;
    for (int i = 0; i < total_lamp_now; i++)
    {
      if ((fb_dim_port2.dim_fb[i] != fb_dim_update_port2.dim_fb[i] || fb_dim_port2.group[i] != fb_dim_update_port2.group[i] ||
           fb_dim_port2.id[i] != fb_dim_update_port2.id[i]) &&
          fb_dim_update_port2.group[i] != 0 && fb_dim_update_port2.id[i] != 0)
      {
        fb_dim_port2.dim_fb[i] = fb_dim_update_port2.dim_fb[i];
        fb_dim_port2.group[i] = fb_dim_update_port2.group[i];
        fb_dim_port2.id[i] = fb_dim_update_port2.id[i];
      }
    }
    SerialFlash.eraseBlock(ADDR_STORAGE_DIM_PORT_2);
    SerialFlash.write(ADDR_STORAGE_DIM_PORT_2, (char *)&fb_dim_port2, sizeof(fb_dim_id_t));
    // for (int i = 0; i < total_lamp_now; i++)
    // {
    //   debug(MAIN_DEBUG, "dim fb: %d, lamp port: %d, group: %d, id: %d, port: %d\r\n", fb_dim_port2.dim_fb[i],
    //         fb_dim_port2.lampport, fb_dim_port2.group[i], fb_dim_port2.id[i], fb_dim_port2.port);
    // }
  }
  
  if (flagIncom.setdimIdGroupPort1)
  {
    flagIncom.setdimIdGroupPort1 = false;
    insert_lamp_to_group.rear = lamp_recv_cpu_port1.rear;
    for (int i = 0; i <= lamp_recv_cpu_port1.rear; i++)
    {
      if (lamp_recv_cpu_port1.inp_id[i] != 0)
      {
        debug(MAIN_DEBUG, "id port 1: %d\r\n", lamp_recv_cpu_port1.inp_id[i]);
        insert_lamp_to_group.inp_id[i] = lamp_recv_cpu_port1.inp_id[i];
      }
      else
      {
        break;
      }
    }

    if (lamp_recv_cpu_port1.group == 1)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1, (char *)&lamp_group1_p1, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1, (char *)&out_dim_lamp_gr1_p1, sizeof(add_lamp_dim_group_t));
    }
    else if (lamp_recv_cpu_port1.group == 2)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1, (char *)&lamp_group2_p1, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1, (char *)&out_dim_lamp_gr2_p1, sizeof(add_lamp_dim_group_t));
    }
    else if (lamp_recv_cpu_port1.group == 3)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1, (char *)&lamp_group3_p1, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1, (char *)&out_dim_lamp_gr3_p1, sizeof(add_lamp_dim_group_t));
    }
    initizaline_all_lamp_in_branch();
    compare_lamp_group_with_group_all(lamp_group1_p1, lamp_group_all_p1, 1);
    compare_lamp_group_with_group_all(lamp_group2_p1, lamp_group_all_p1, 1);
    compare_lamp_group_with_group_all(lamp_group3_p1, lamp_group_all_p1, 1);

    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
    // for(int i = 0 ; i <= lamp_total_default ; i++)
    // {
    //   debug(MAIN_DEBUG,"got lamp group all port 1: %d\r\n", out_dim_lamp_p1.id_dim[i]);
    // }
    debug(MAIN_DEBUG, "%s\r\n", "BRANCH 1");

    memset(&insert_lamp_to_group, 0x00, sizeof(insert_lamp_to_group));
  }

  if (flagIncom.setdimIdGroupPort2)
  {
    flagIncom.setdimIdGroupPort2 = false;
    insert_lamp_to_group.rear = lamp_recv_cpu_port2.rear;
    for (int i = 0; i <= lamp_recv_cpu_port2.rear; i++)
    {
      if (lamp_recv_cpu_port2.inp_id[i] != 0)
      {
        debug(MAIN_DEBUG, "id port 2: %d\r\n", lamp_recv_cpu_port2.inp_id[i]);
        insert_lamp_to_group.inp_id[i] = lamp_recv_cpu_port2.inp_id[i];
      }
      else
      {
        break;
      }
    }

    if (lamp_recv_cpu_port2.group == 1)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2, (char *)&lamp_group1_p2, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2, (char *)&out_dim_lamp_gr1_p2, sizeof(add_lamp_dim_group_t));
    }
    else if (lamp_recv_cpu_port2.group == 2)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2, (char *)&lamp_group2_p2, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2, (char *)&out_dim_lamp_gr2_p2, sizeof(add_lamp_dim_group_t));
    }
    else if (lamp_recv_cpu_port2.group == 3)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2);
      SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2, (char *)&insert_lamp_to_group, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2, (char *)&lamp_group3_p2, sizeof(change_lamp_to_group_t));
      SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2, (char *)&out_dim_lamp_gr3_p2, sizeof(add_lamp_dim_group_t));
    }
    initizaline_all_lamp_in_branch();
    compare_lamp_group_with_group_all(lamp_group1_p2, lamp_group_all_p2, 2);
    compare_lamp_group_with_group_all(lamp_group2_p2, lamp_group_all_p2, 2);
    compare_lamp_group_with_group_all(lamp_group3_p2, lamp_group_all_p2, 2);

    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
    debug(MAIN_DEBUG, "%s\r\n", "BRANCH 2");
    memset(&insert_lamp_to_group, 0x00, sizeof(insert_lamp_to_group));
  }

  if (flagIncom.setScheduleGroup)
  {
    flagIncom.setScheduleGroup = false;
    if (add_dim_schedule_group.group == ENABLE_GROUP1)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP1);
      SerialFlash.write(ADDR_STORAGE_SET_SCHEDULE_GROUP1, (char *)&add_dim_schedule_group, sizeof(dim_active_schedule_t));
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP1, (char *)&dim_schedule_group1, sizeof(dim_active_schedule_t));
    }
    else if (add_dim_schedule_group.group == ENABLE_GROUP2)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP2);
      SerialFlash.write(ADDR_STORAGE_SET_SCHEDULE_GROUP2, (char *)&add_dim_schedule_group, sizeof(dim_active_schedule_t));
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP2, (char *)&dim_schedule_group2, sizeof(dim_active_schedule_t));
    }
    else if (add_dim_schedule_group.group == ENABLE_GROUP3)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP3);
      SerialFlash.write(ADDR_STORAGE_SET_SCHEDULE_GROUP3, (char *)&add_dim_schedule_group, sizeof(dim_active_schedule_t));
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP3, (char *)&dim_schedule_group3, sizeof(dim_active_schedule_t));
    }
    /*-------------------------------------------------------------------------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------*/
    /*-------------------------------------------------------------------------------------------------------------------------*/
    else if (add_dim_schedule_group.group == DISABLE_GROUP1)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP1);
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP1, (char *)&dim_schedule_group1, sizeof(dim_active_schedule_t));
    }
    else if (add_dim_schedule_group.group == DISABLE_GROUP2)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP2);
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP2, (char *)&dim_schedule_group2, sizeof(dim_active_schedule_t));
    }
    else if (add_dim_schedule_group.group == DISABLE_GROUP3)
    {
      SerialFlash.eraseBlock(ADDR_STORAGE_SET_SCHEDULE_GROUP3);
      wait(0.01);
      SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP3, (char *)&dim_schedule_group3, sizeof(dim_active_schedule_t));
    }
    debug(MAIN_DEBUG, "group: %d, dim1: %d, dim2: %d, dim3: %d, dim4: %d, dim5: %d, dim6: %d, dim7: %d, dim8: %d, dim9: %d\r\n",
          dim_schedule_group1.group, dim_schedule_group1.dim1, dim_schedule_group1.dim2,
          dim_schedule_group1.dim3, dim_schedule_group1.dim4, dim_schedule_group1.dim5,
          dim_schedule_group1.dim6, dim_schedule_group1.dim7, dim_schedule_group1.dim8,
          dim_schedule_group1.dim9);

    debug(MAIN_DEBUG, "hh1: %d, hh2: %d, hh3: %d, hh4: %d, hh5: %d, hh6: %d, hh7: %d, hh8: %d, hh9: %d\r\n",
          dim_schedule_group1.hh_start_dim1, dim_schedule_group1.hh_start_dim2, dim_schedule_group1.hh_start_dim3,
          dim_schedule_group1.hh_start_dim4, dim_schedule_group1.hh_start_dim5, dim_schedule_group1.hh_start_dim6,
          dim_schedule_group1.hh_start_dim7, dim_schedule_group1.hh_start_dim8, dim_schedule_group1.hh_start_dim9);

    debug(MAIN_DEBUG, "mm1: %d, mm2: %d, mm3: %d, mm4: %d, mm5: %d, mm6: %d, mm7: %d, mm8: %d, mm9: %d\r\n",
          dim_schedule_group1.mm_start_dim1, dim_schedule_group1.mm_start_dim2, dim_schedule_group1.mm_start_dim3,
          dim_schedule_group1.mm_start_dim4, dim_schedule_group1.mm_start_dim5, dim_schedule_group1.mm_start_dim6,
          dim_schedule_group1.mm_start_dim7, dim_schedule_group1.mm_start_dim8, dim_schedule_group1.mm_start_dim9);

    debug(MAIN_DEBUG, "group: %d, dim1: %d, dim2: %d, dim3: %d, dim4: %d, dim5: %d, dim6: %d, dim7: %d, dim8: %d, dim9: %d\r\n",
          dim_schedule_group2.group, dim_schedule_group2.dim1, dim_schedule_group2.dim2,
          dim_schedule_group2.dim3, dim_schedule_group2.dim4, dim_schedule_group2.dim5,
          dim_schedule_group2.dim6, dim_schedule_group2.dim7, dim_schedule_group2.dim8,
          dim_schedule_group2.dim9);

    debug(MAIN_DEBUG, "hh1: %d, hh2: %d, hh3: %d, hh4: %d, hh5: %d, hh6: %d, hh7: %d, hh8: %d, hh9: %d\r\n",
          dim_schedule_group2.hh_start_dim1, dim_schedule_group2.hh_start_dim2, dim_schedule_group2.hh_start_dim3,
          dim_schedule_group2.hh_start_dim4, dim_schedule_group2.hh_start_dim5, dim_schedule_group2.hh_start_dim6,
          dim_schedule_group2.hh_start_dim7, dim_schedule_group2.hh_start_dim8, dim_schedule_group2.hh_start_dim9);

    debug(MAIN_DEBUG, "mm1: %d, mm2: %d, mm3: %d, mm4: %d, mm5: %d, mm6: %d, mm7: %d, mm8: %d, mm9: %d\r\n",
          dim_schedule_group2.mm_start_dim1, dim_schedule_group2.mm_start_dim2, dim_schedule_group2.mm_start_dim3,
          dim_schedule_group2.mm_start_dim4, dim_schedule_group2.mm_start_dim5, dim_schedule_group2.mm_start_dim6,
          dim_schedule_group2.mm_start_dim7, dim_schedule_group2.mm_start_dim8, dim_schedule_group2.mm_start_dim9);

    debug(MAIN_DEBUG, "group: %d, dim1: %d, dim2: %d, dim3: %d, dim4: %d, dim5: %d, dim6: %d, dim7: %d, dim8: %d, dim9: %d\r\n",
          dim_schedule_group3.group, dim_schedule_group3.dim1, dim_schedule_group3.dim2,
          dim_schedule_group3.dim3, dim_schedule_group3.dim4, dim_schedule_group3.dim5,
          dim_schedule_group3.dim6, dim_schedule_group3.dim7, dim_schedule_group3.dim8,
          dim_schedule_group3.dim9);

    debug(MAIN_DEBUG, "hh1: %d, hh2: %d, hh3: %d, hh4: %d, hh5: %d, hh6: %d, hh7: %d, hh8: %d, hh9: %d\r\n",
          dim_schedule_group3.hh_start_dim1, dim_schedule_group3.hh_start_dim2, dim_schedule_group3.hh_start_dim3,
          dim_schedule_group3.hh_start_dim4, dim_schedule_group3.hh_start_dim5, dim_schedule_group3.hh_start_dim6,
          dim_schedule_group3.hh_start_dim7, dim_schedule_group3.hh_start_dim8, dim_schedule_group3.hh_start_dim9);

    debug(MAIN_DEBUG, "mm1: %d, mm2: %d, mm3: %d, mm4: %d, mm5: %d, mm6: %d, mm7: %d, mm8: %d, mm9: %d\r\n",
          dim_schedule_group3.mm_start_dim1, dim_schedule_group3.mm_start_dim2, dim_schedule_group3.mm_start_dim3,
          dim_schedule_group3.mm_start_dim4, dim_schedule_group3.mm_start_dim5, dim_schedule_group3.mm_start_dim6,
          dim_schedule_group3.mm_start_dim7, dim_schedule_group3.mm_start_dim8, dim_schedule_group3.mm_start_dim9);
  }
  
  if (flagIncom.setChangeIdGroupPort1)
  {
    flagIncom.setChangeIdGroupPort1 = false;
  }

  if(mtfc_flag_system.is_switch_control)
  {
    mtfc_flag_system.is_switch_control = false;
    if(mtlc_working_current_base.is_read_signal_switch_cpu == 1)
    {
      mtlc_working_current_base.mtlc_working_state = mtlc_state_auto;
    }
    else if (mtlc_working_current_base.is_read_signal_switch_cpu == 2)
    {
      mtlc_working_current_base.mtlc_working_state = mtlc_state_manual;
    }
    else if(mtlc_working_current_base.is_read_signal_switch_cpu == 0)
    {
      mtlc_working_current_base.mtlc_working_state = mtlc_state_off;
    }
    else {
      mtlc_working_current_base.mtlc_working_state = mtlc_state_auto;
    }
  }
}
#pragma endregion

#pragma region[OUTCOM PROCESSING-----------------------------------------------------------------------------]
void mtlc_output_mtlc_parameters(void)
{
  // port 2
  if (flagIncom.forwardCounterfb)
  {
    outCom1.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    outCom5.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_1, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    debug(MAIN_DEBUG, "\r\nforward Counter fb = %d", counters_package.val1);
    flagIncom.forwardCounterfb = false;
  }

  // port 1
  if (flagIncom.forwardCounterfb2)
  {
    outCom1.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    outCom5.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    debug(MAIN_DEBUG, "\r\nforward Counter port 1 fb = %d", counters_package.val1);
    flagIncom.forwardCounterfb2 = false;
    flagIncom.flatcounterport1 = 1;
  }
  if (flagIncom.forwardCounterfb3)
  {
    outCom1.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_RF, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    outCom5.send_struct(CMD_MASTER_TO_CPU_COUNT_LAMP_PORT_2, (uint8_t *)&counters_package, sizeof(type_configPackage_t));
    debug(MAIN_DEBUG, "\r\nforward Counter RF fb = %d", counters_package.val1);
    flagIncom.forwardCounterfb3 = false;
  }
  if (flagIncom.fbdimtocpu)
  {
    outCom1.send_struct(CMD_MASTER_FB_DIM_TO_CPU, (uint8_t *)&add_p, sizeof(add_p));
    debug(MAIN_DEBUG, "\r\nforward dim to CPU: %d", add_p.dim);
    flagIncom.fbdimtocpu = false;
  }
}
#pragma endregion

#pragma region[TIMEOUT PROCESSING-----------------------------------------------------------------------------]
void timeOutProcessing(void)
{
  if (flagIncom.Istimeout)
  {
    control_relay(0);
    relay2 = 0;
    relay3 = 0;
    flagIncom.Istimeout = false;
  }
}
#pragma endregion

#pragma region[CAC HAM KHAC----------------------------------------------------------------------------------]
void mtlc_memory_startup_config(void)
{
  memset((uint8_t *)&mtfc_flag_system, 0, sizeof(type_flag_system_t));
  memset((uint8_t *)&mtfc_rtc_val, 0, sizeof(type_date_time_t));
  memset((uint8_t*)&change_lamp_power, 0, sizeof(change_power_lamp_t));
  memset((uint8_t *)&mtlc_force_cabinet, 0, sizeof(type_mtlc_force_on_t));
  memset((uint8_t *)&mtlc_sensor_allive, 0, sizeof(type_mtlc_force_on_t));
  memset((uint8_t *)&mtlc_working_message, 0, sizeof(type_mtlc_working_message_t));
  memset((uint8_t *)&mtlc_working_current_base, 0, sizeof(type_mtlc_working_pakage_t));
}

void mtlc_initialize_parameters(type_mtlc_working_pakage_t *obj)
{
  obj->mtlc_main_state = mtlc_normal_state;
  obj->mtlc_working_state = mtlc_state_auto;
}

void ScanI2C(void)
{
  
  int count1;
  for (int address = 1; address < 127; address++)
  {
    if (!i2c.write(address << 1, NULL, 0))
    {
      debug(MAIN_DEBUG, "I2C device found at address %3d (0x%3x in 8-bit)\n", address, address << 1);
      count1++;
    }
    wait_ms(20);
  }
  if (count1)
  {
    debug(MAIN_DEBUG, "count: %d\r\n", count1);
  }
  else
  {
    debug(MAIN_DEBUG, "\r\n%s", "No");
  }
  debug(MAIN_DEBUG, "\r\n%s", "device found\r\n");
}

void getCurrentRTC(void)
{
  int reg = rtc_cus.readRegister(DS3231_Aging_Offset);
  if (reg > 127)
  {
    reg = reg - 256;
  }
  rtc_cus.readDate(&add_rtc.day, &add_rtc.month, &add_rtc.years);
  rtc_cus.readTime(&add_rtc.hour, &add_rtc.min, &add_rtc.sec);
  wait(0.5);

#ifdef DEBUG_RTC
  debug(MAIN_DEBUG, "time : %02i-%02i-%02i\r\n", add_rtc.hour, add_rtc.min, add_rtc.sec);
  debug(MAIN_DEBUG, "day : %02i-%02i-%02i\r\n", add_rtc.day, add_rtc.month, add_rtc.years);
  wait(0.2);
#endif
}

void get_all_settings_storage(void)
{
  SerialFlash.read(ADDR_STORAGE_DIM_ALL_CONTROL, (char *)&dim_active_schedule_control, sizeof(dim_active_schedule_control));
  SerialFlash.read(ADDR_STORAGE_SCHEDULE_DATA, (char *)&meter_schedule, sizeof(meter_schedule));
  SerialFlash.read(ADDR_STORAGE_CHANGE_ID_DIM, (char *)&change_id, sizeof(change_id_t));
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[0]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[1]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[2]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[3]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[4]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[5]);
  // debug(MAIN_DEBUG, "Change id: %d\r\n", change_id.inptchr[6]);

#ifdef DEBUG_SETTINGS_STORAGE
  debug(MAIN_DEBUG, "size struct meter schedule: %d\r\n", sizeof(dim_active_schedule_control));
  debug(MAIN_DEBUG, "Size array schedule: %d\r\n", meter_schedule.size_array_schedule);
#endif
  /*------------------------------------------------------------------------------------------------------------*/
  /*------------------------------------------------------------------------------------------------------------*/
  /*------------------------------------------------------------------------------------------------------------*/

  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1, (char *)&lamp_group1_p1, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT1, (char *)&out_dim_lamp_gr1_p1, sizeof(add_lamp_dim_group_t));
  debug(MAIN_DEBUG, "rear lamp group 1 port 1 : %d\r\n", lamp_group1_p1.rear);
  compare_lamp_group_with_group_all(lamp_group1_p1, lamp_group_all_p1, 1);

  for (int i = 0; i <= lamp_group1_p1.rear; i++)
  {
    if (lamp_group1_p1.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 1 id lamp port 1: %d\r\n", lamp_group1_p1.inp_id[i]);
    }
    else
    {
      break;
    }
  }
  debug(MAIN_DEBUG, "--------------------------------------%s\r\n", "GROUP1");
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2, (char *)&lamp_group1_p2, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP1_PORT2, (char *)&out_dim_lamp_gr1_p2, sizeof(add_lamp_dim_group_t));
  compare_lamp_group_with_group_all(lamp_group1_p2, lamp_group_all_p2, 2);

  for (int i = 0; i <= lamp_group1_p2.rear; i++)
  {
    if (lamp_group1_p2.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 1 id lamp port 2: %d\r\n", lamp_group1_p2.inp_id[i]);
    }
    else
    {
      break;
    }
  }

  debug(MAIN_DEBUG, "--------------------------------------%s\r\n", "");

  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1, (char *)&lamp_group2_p1, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT1, (char *)&out_dim_lamp_gr2_p1, sizeof(add_lamp_dim_group_t));
  compare_lamp_group_with_group_all(lamp_group2_p1, lamp_group_all_p1, 1);

  for (int i = 0; i <= lamp_group2_p1.rear; i++)
  {
    if (lamp_group2_p1.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 2 id lamp port 1: %d\r\n", lamp_group2_p1.inp_id[i]);
    }
    else
    {
      break;
    }
  }

  debug(MAIN_DEBUG, "--------------------------------------%s\r\n", "GROUP2");
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2, (char *)&lamp_group2_p2, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP2_PORT2, (char *)&out_dim_lamp_gr2_p2, sizeof(add_lamp_dim_group_t));
  compare_lamp_group_with_group_all(lamp_group2_p2, lamp_group_all_p2, 2);

  for (int i = 0; i <= lamp_group2_p2.rear; i++)
  {
    if (lamp_group2_p2.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 2 id lamp port 2: %d\r\n", lamp_group2_p2.inp_id[i]);
    }
    else
    {
      break;
    }
  }

  debug(MAIN_DEBUG, "--------------------------------------%s\r\n", "");
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1, (char *)&lamp_group3_p1, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT1, (char *)&out_dim_lamp_gr3_p1, sizeof(add_lamp_dim_group_t));
  compare_lamp_group_with_group_all(lamp_group3_p1, lamp_group_all_p1, 1);

  for (int i = 0; i <= lamp_group3_p1.rear; i++)
  {
    if (lamp_group3_p1.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 3 id lamp port 1: %d\r\n", lamp_group3_p1.inp_id[i]);
    }
    else
    {
      break;
    }
  }

  debug(MAIN_DEBUG, "--------------------------------------%s\r\n", "GROUP3");
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2, (char *)&lamp_group3_p2, sizeof(change_lamp_to_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP3_PORT2, (char *)&out_dim_lamp_gr3_p2, sizeof(add_lamp_dim_group_t));
  compare_lamp_group_with_group_all(lamp_group3_p2, lamp_group_all_p2, 2);

  for (int i = 0; i <= lamp_group3_p2.rear; i++)
  {
    if (lamp_group3_p2.inp_id[i] != 0)
    {
      debug(MAIN_DEBUG, "GROUP 3 id lamp port 2: %d\r\n", lamp_group3_p2.inp_id[i]);
    }
    else
    {
      break;
    }
  }

  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
  SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP1, (char *)&dim_schedule_group1, sizeof(dim_active_schedule_t));
  SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP2, (char *)&dim_schedule_group2, sizeof(dim_active_schedule_t));
  SerialFlash.read(ADDR_STORAGE_SET_SCHEDULE_GROUP3, (char *)&dim_schedule_group3, sizeof(dim_active_schedule_t));
}

void control_relay(int relay_out)
{
  relay1 = relay_out;
  relay2 = relay_out;
  relay3 = relay_out;
}

void mtlc_one_seconds_ticker_event_handler(void)
{
  mtfc_flag_system.is_ticker_one_seconds_main = (mtfc_flag_system.is_ticker_one_seconds_main == false) ? true : mtfc_flag_system.is_ticker_one_seconds_main;
  mtfc_flag_system.is_ticker_one_seconds_keepallive_cpu = (mtfc_flag_system.is_ticker_one_seconds_keepallive_cpu == false) ? true : mtfc_flag_system.is_ticker_one_seconds_keepallive_cpu;
}

void mtlc_force_on_cabinet(type_mtlc_force_on_t *force_cabinet, type_flag_system_t *flag_system, type__mtfc_hm_time_t *time_current)
{
  int check_time_current = time_current->hour * 60 + time_current->minute;
  if(flag_system->is_progess_force_cabinet)
  {
    flag_system->is_progess_force_cabinet = false;
    flag_system->is_time_active_cabinet = false;
    flag_system->is_ticker_force_cabinet = true;
    if((check_time_current > ((force_cabinet->hh_start * 60) + force_cabinet->mm_start)) && (check_time_current < ((force_cabinet->hh_end * 60) + force_cabinet->mm_end)))
    {
      // debug(MAIN_DEBUG, "force cabinet time: %d ---- %d --- *** %d --- %d ---- force cabinet: %d\r\n", force_cabinet->hh_start, force_cabinet->mm_start, force_cabinet->hh_end, force_cabinet->mm_end, force_cabinet->force_cabinet);
      if(force_cabinet->force_cabinet == 1)
      {
        control_relay(1);
        mtlc_system_response.force_cabinet_status = 1;
        if(flag_system->is_response_force_cabinet)
        {
          flag_system->is_response_force_cabinet = false;
          inCom.send_struct(CMD_MASTER_TO_CPU_STATUS_FORCE_CABINET, (uint8_t*)&mtlc_system_response, sizeof(type_mtlc_data_response_t));
        }
      }
      else if(force_cabinet->force_cabinet == 0) {
        control_relay(0);
        mtlc_system_response.force_cabinet_status = 1;
        if(flag_system->is_response_force_cabinet)
        {
          flag_system->is_response_force_cabinet = false;
          inCom.send_struct(CMD_MASTER_TO_CPU_STATUS_FORCE_CABINET, (uint8_t*)&mtlc_system_response, sizeof(type_mtlc_data_response_t));
        }
      }
    }
  }
  // het thoi gian hoat dong cam bien, yeu cau tro ve lai thoi gian bat tat tu
  // if(flag_system->is_ticker_force_cabinet)
  // {
    if(check_time_current > ((force_cabinet->hh_end * 60) + force_cabinet->mm_end)){
      // debug(MAIN_DEBUG, "%s\r\n", "timeout active sensor bye :)");
      flag_system->is_time_active_cabinet = true;
    }
  // }
}

void mtlc_rtc_update(type_date_time_t *t)
{
  static unsigned long mtfc_mill_update_time = millis();
  if (mtfc_flag_system.is_update_rtc)
  {
    type_date_time_t temp = *t;
    if (mtfc_rtc.rtc_set_time(temp))
    {
      debug(MAIN_DEBUG, "%s\r\n", "Rtc update failed!!!");
      system_reset();
    }
    else
    {
      debug(MAIN_DEBUG, "%s\r\n", "Rtc update successful!!!");
    }
    debug(MAIN_DEBUG, "\r\nSET RTC: %02d %02d %02d - %d %02d/%02d/%04d",
          mtfc_rtc_new_update.hour,
          mtfc_rtc_new_update.minute,
          mtfc_rtc_new_update.seconds,
          mtfc_rtc_new_update.day,
          mtfc_rtc_new_update.date,
          mtfc_rtc_new_update.month,
          mtfc_rtc_new_update.year);
    mtfc_flag_system.is_require_update_rtc_ex = mtfc_flag_system.is_require_update_rtc_ex ? false : mtfc_flag_system.is_require_update_rtc_ex;
    mtfc_flag_system.is_update_rtc = false;
  }

  if (mtfc_rtc_new_update.year < 2025)
  {
    // debug(MAIN_DEBUG, "%s\r\n", "Error rtc time check hardware?");
    mtfc_flag_system.is_require_update_rtc_ex = true;
  }

  if (mtfc_flag_system.is_require_update_rtc_ex)
  {
    if (millis() - mtfc_mill_update_time >= 3000)
    {
      // debug(MAIN_DEBUG, "%s\r\n", "gui goi tin cho cpu"); // yeu cau CPU cap phat lai thoi gian
      outCom1.send_byte(CMD_MASTER_TO_CPU_SYN_RTC_TIME, 2);
      mtfc_mill_update_time = millis();
    }
  }
}

void mtlc_main_ticker_one_seconds(void)
{
  static unsigned long mtlc_send_error_time = millis();
  if (mtfc_flag_system.is_ticker_one_seconds_main)
  {
    mtfc_rtc_val = mtfc_rtc.rtc_get_time();
    mtfc_hm_present.hour = mtfc_rtc_val.hour;
    mtfc_hm_present.minute = mtfc_rtc_val.minute;

    debug(MAIN_DEBUG, "\r\n%s:%d:%d:%d", "Time", mtfc_hm_present.hour, mtfc_hm_present.minute, mtfc_rtc_val.seconds);
    if ((mtfc_rtc_val.hour == 1) && (mtfc_rtc_val.minute == 1) && (mtfc_rtc_val.seconds == 1))
    {
      mtfc_rtc.rtc_load_ex_rtc_to_internal();
    }
    mtfc_flag_system.is_ticker_one_seconds_main = false;
    mtfc_flag_system.is_enable_check_time_power = true;
  }

  if (mtfc_rtc_val.year < 2025 || mtfc_rtc_val.hour > 24)
  {
    if(millis() - mtlc_send_error_time >= 5000)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Error rtc_cus time check hardware ticker one second?");
      mtfc_flag_system.is_require_update_rtc_ex = true;
      mtlc_send_error_time = millis();
      system_reset();
    }
  }

  if (mtfc_flag_system.is_require_update_rtc_ex)
  {
    debug(MAIN_DEBUG, "%s\r\n", "gui goi tin cho cpu"); // yeu cau CPU cap phat lai thoi gian
    outCom1.send_byte(CMD_MASTER_TO_CPU_SYN_RTC_TIME, 2);
    mtlc_send_error_time = millis();
    mtfc_flag_system.is_require_update_rtc_ex = false;
  }
}

void mtlc_keepallive_cpu_ticker_one_seconds(void)
{
  if(mtfc_flag_system.is_ticker_one_seconds_keepallive_cpu)
  {
    keepalliveCPU = keepalliveCPU + 1;
    // debug(MAIN_DEBUG, "got value keep allive CPU : %d\r\n", keepalliveCPU);

    /**
     * mat ket noi tu cpu, off chuc nang cam bien anh sang
     */
    if(keepalliveCPU > 30)
    {
      mtfc_flag_system.is_time_active_cabinet = true;
      if(keepalliveCPU > 35)
      {
        keepalliveCPU = 0; 
      }
    }
    mtfc_flag_system.is_ticker_one_seconds_keepallive_cpu = false;
  }
}

void mtlc_normal_master_processing(void)
{
  static uint32_t mtlc_ticker_normal_master = millis();
  if(mtlc_working_current_base.mtlc_working_state == mtlc_state_off)
  {
    control_relay(0);
  }
  else if(mtlc_working_current_base.mtlc_working_state == mtlc_state_manual)
  {
    control_relay(1);
  }
  else if(mtlc_working_current_base.mtlc_working_state == mtlc_state_auto)
  {
    if(millis() - mtlc_ticker_normal_master >= 1000)
    {
      send_time_to_cpu_monitoring(&mtfc_hm_present, &mtlc_time_master_processing);
      if(mtfc_flag_system.is_sensor_active_cabinet)
      {
        mtlc_force_on_cabinet(&mtlc_force_cabinet, &mtfc_flag_system, &mtfc_hm_present);
      }
      if(mtfc_flag_system.is_enable_check_time_power)
      {
        if(mtfc_flag_system.is_time_active_cabinet)
        {
          control_power_slave_dim(&mtfc_hm_present);
        }
      }
      if (relay1.read())
      {
        control_dim_lamp_all();             // dim all control
        if (dim_schedule_group1.group == 1) // dim group
        {
          control_dim_group(&dim_schedule_group1, &out_dim_lamp_gr1_p1, &out_dim_lamp_gr1_p2, CMD_MASTER_TO_SLAVE_DIM_GROUP);
        }
        if (dim_schedule_group2.group == 2)
        {
          control_dim_group(&dim_schedule_group2, &out_dim_lamp_gr2_p1, &out_dim_lamp_gr2_p2, CMD_MASTER_TO_SLAVE_DIM_GROUP);
        }
        if (dim_schedule_group3.group == 3)
        {
          control_dim_group(&dim_schedule_group3, &out_dim_lamp_gr3_p1, &out_dim_lamp_gr3_p2, CMD_MASTER_TO_SLAVE_DIM_GROUP);
        }
      }
      mtlc_ticker_normal_master = millis();
    }
  }
}

void control_power_slave_dim(type__mtfc_hm_time_t *mtlc_time_present)
{
  int check_time = mtlc_time_present->hour * 60 + mtlc_time_present->minute;
  int start_time_1 = meter_schedule.hh_start[0] * 60 + meter_schedule.mm_start[0];
  int end_time_1 = meter_schedule.hh_end[0] * 60 + meter_schedule.mm_end[0];
  int start_time_2 = meter_schedule.hh_start[1] * 60 + meter_schedule.mm_start[1];
  int end_time_2 = meter_schedule.hh_end[1] * 60 + meter_schedule.mm_end[1];
  // debug(MAIN_DEBUG, "status relay %d\r\n", relay1.read());
  // debug(MAIN_DEBUG, "size_array_schedule %d\r\n", meter_schedule.size_array_schedule);
  // debug(MAIN_DEBUG, "check_time [%03d] start 1 [%03d] end 1 [%03d], start 2 [%03d] end 2 [%03d]\r\n", check_time, start_time_1, end_time_1, start_time_2, end_time_2);
  if (meter_schedule.size_array_schedule == 1)
  {
    if (start_time_1 <= end_time_1)
    {
      if (check_time >= start_time_1 && check_time <= end_time_1)
      {
        control_relay(1);
      }
      else
      {
        control_relay(0);
      }
    }
    else
    {
      if (check_time >= start_time_1 || check_time <= end_time_1)
      {
        control_relay(1);
      }
      else
      {
        control_relay(0);
      }
    }
  }
  else
  {
    if ((start_time_1 <= end_time_1) && (start_time_2 <= end_time_2))
    {
      if ((check_time >= start_time_1 && check_time <= end_time_1) || (check_time >= start_time_2 && check_time <= end_time_2))
      {
        control_relay(1);
      }
      else
      {
        control_relay(0);
      }
    }
    else if ((start_time_1 > end_time_1) && (start_time_2 <= end_time_2))
    {
      if ((((check_time >= start_time_1) || (check_time <= end_time_1)) || ((check_time >= start_time_2) && (check_time <= end_time_2))) && (start_time_1 > start_time_2))
      {
        control_relay(1);
      }
      else
      {
        control_relay(0);
      }
    }
    else if ((start_time_2 > end_time_2) && (start_time_1 <= end_time_1))
    {
      if ((((check_time >= start_time_2) || (check_time <= end_time_2)) || ((check_time >= start_time_1) && (check_time <= end_time_1))) && start_time_2 > start_time_1)
      {
        control_relay(1);
      }
      else
      {
        control_relay(0);
      }
    }
    else
    {
      control_relay(0);
    }
  }
}

void control_dim_lamp_all(void)
{
  static unsigned long mtlc_working_dim_lamp = millis();
  int tmp_current_time_dimAll = -1;
  int check_time = mtfc_hm_present.hour * 60 + mtfc_hm_present.minute;
  int check_time_dim1 = dim_active_schedule_control.hh_start_dim1 * 60 + dim_active_schedule_control.mm_start_dim1;
  int check_time_dim2 = dim_active_schedule_control.hh_start_dim2 * 60 + dim_active_schedule_control.mm_start_dim2;
  int check_time_dim3 = dim_active_schedule_control.hh_start_dim3 * 60 + dim_active_schedule_control.mm_start_dim3;
  int check_time_dim4 = dim_active_schedule_control.hh_start_dim4 * 60 + dim_active_schedule_control.mm_start_dim4;
  int check_time_dim5 = dim_active_schedule_control.hh_start_dim5 * 60 + dim_active_schedule_control.mm_start_dim5;
  int check_time_dim6 = dim_active_schedule_control.hh_start_dim6 * 60 + dim_active_schedule_control.mm_start_dim6;
  int check_time_dim7 = dim_active_schedule_control.hh_start_dim7 * 60 + dim_active_schedule_control.mm_start_dim7;
  int check_time_dim8 = dim_active_schedule_control.hh_start_dim8 * 60 + dim_active_schedule_control.mm_start_dim8;
  int check_time_dim9 = dim_active_schedule_control.hh_start_dim9 * 60 + dim_active_schedule_control.mm_start_dim9;
  if (tmp_current_time_dimAll != mtfc_hm_present.minute)
  {
    if (check_time == check_time_dim1)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim1;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim1;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      // goi tin 3s 1 lan truyen cho bo dim tai den va nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim1, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim2)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim2;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim2;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim2, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim3)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim3;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim3;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim3, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();        
      }
    }
    else if (check_time == check_time_dim4)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim4;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim4;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%d] group [%d]\r\n", check_time_dim4, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim5)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim5;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim5;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim5, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim6)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim6;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim6;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim6, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim7)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim7;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim7;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim7, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim8)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim8;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim8;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis() - mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim8, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
    else if (check_time == check_time_dim9)
    {
      tmp_current_time_dimAll = mtfc_hm_present.minute;
      out_dim_lamp_p1.dim_value = dim_active_schedule_control.dim9;
      out_dim_lamp_p2.dim_value = dim_active_schedule_control.dim9;
      out_dim_lamp_p1.cabinetid = change_id.cabinetid; // gan cabinet ID cho struct de gui di cho nema
      if(millis()- mtlc_working_dim_lamp >= 3000)
      {
        outCom2.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        outCom3.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p2, sizeof(add_lamp_dim_group_t));
        outCom5.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM, (uint8_t *)&out_dim_lamp_p1, sizeof(add_lamp_dim_group_t));
        debug(DIM_DEBUG, "check time dim [%03d] group [%d]\r\n", check_time_dim9, dim_active_schedule_control.group);
        mtlc_working_dim_lamp = millis();
      }
    }
  }
}

void implement_control_dim_group(dim_active_schedule_t *dim_params, add_lamp_dim_group_t *group_port1, add_lamp_dim_group_t *group_port2, uint8_t cmd_dim_group)
{
  outCom2.send_struct(cmd_dim_group, (uint8_t *)group_port1, sizeof(add_lamp_dim_group_t));
  outCom5.send_struct(cmd_dim_group, (uint8_t *)group_port1, sizeof(add_lamp_dim_group_t));
  outCom3.send_struct(cmd_dim_group, (uint8_t *)group_port2, sizeof(add_lamp_dim_group_t));
}

void control_dim_group(dim_active_schedule_t *dim_schedules, add_lamp_dim_group_t *add_id_group_dim_port1, add_lamp_dim_group_t *add_id_group_dim_port2, uint8_t cmd_dim_group)
{
  static unsigned long mtlc_working_dim_group = millis();
  int tmp_current_time_dim_group = -1;
  int check_time = mtfc_hm_present.hour * 60 + mtfc_hm_present.minute;
  int check_time_dim1 = dim_schedules->hh_start_dim1 * 60 + dim_schedules->mm_start_dim1;
  int check_time_dim2 = dim_schedules->hh_start_dim2 * 60 + dim_schedules->mm_start_dim2;
  int check_time_dim3 = dim_schedules->hh_start_dim3 * 60 + dim_schedules->mm_start_dim3;
  int check_time_dim4 = dim_schedules->hh_start_dim4 * 60 + dim_schedules->mm_start_dim4;
  int check_time_dim5 = dim_schedules->hh_start_dim5 * 60 + dim_schedules->mm_start_dim5;
  int check_time_dim6 = dim_schedules->hh_start_dim6 * 60 + dim_schedules->mm_start_dim6;
  int check_time_dim7 = dim_schedules->hh_start_dim7 * 60 + dim_schedules->mm_start_dim7;
  int check_time_dim8 = dim_schedules->hh_start_dim8 * 60 + dim_schedules->mm_start_dim8;
  int check_time_dim9 = dim_schedules->hh_start_dim9 * 60 + dim_schedules->mm_start_dim9;
  
  if (tmp_current_time_dim_group != mtfc_hm_present.minute)
  {
    if (check_time == check_time_dim1)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim1;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim1;
      if(millis() -  mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);    
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim2)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim2;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim2;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim3)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim3;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim3;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);  
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim4)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim4;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim4;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim5)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim5;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim5;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim6)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim6;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim6;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim7)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim7;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim7;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim8)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim8;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim8;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
    else if (check_time == check_time_dim9)
    {
      tmp_current_time_dim_group = mtfc_hm_present.minute;
      add_id_group_dim_port1->dim_value = dim_schedules->dim9;
      add_id_group_dim_port1->cabinetid = change_id.cabinetid;
      add_id_group_dim_port2->dim_value = dim_schedules->dim9;
      if(millis() - mtlc_working_dim_group >= 3000)
      {
        implement_control_dim_group(dim_schedules, add_id_group_dim_port1, add_id_group_dim_port2, cmd_dim_group);
        mtlc_working_dim_group = millis();
      }
    }
  }
}

void mtlc_led_status_processing(void)
{
  static uint32_t dutyCycleFlashLed = 0;
  if ((millis() - dutyCycleFlashLed) > 200)
  {
    ledAct = !ledAct;
    dutyCycleFlashLed = millis();
  }
}

void timeoutSendEventHandler(void)
{
  if (stateTimeOut == TURN_OFF_MASTER)
  {
    flagIncom.Istimeout = true;
  }
  stateTimeOut = PORT_IDLE;
}

void timeOutEnabled(float t, uint8_t stateSet)
{
  timeout.attach(&timeoutSendEventHandler, t);
  stateTimeOut = stateSet;
  // debug(MAIN_DEBUG, "\r\nEnable timeout feedback, state = %d", stateTimeOut);
}

void timeOutDisable(void)
{
  timeout.detach();
  // debug(MAIN_DEBUG, "\r\n%s","Detach timeOut");
}

add_p_t ReadMemory(void)
{
  add_p_t t;
  memory.read((uint32_t)ADD_SAVE_SETTING, (uint8_t *)&t, sizeof(add_p_t));
  return t;
}

void show_queue_lamp_port(int port)
{
  if (port == 1)
  {
    if (queue_lamps_port1.front == -1)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Empty queue_lamps_port1");
    }
    else
    {
      debug(MAIN_DEBUG, "%s\n", "queue lamps port1: ");
      for (int i = queue_lamps_port1.front; i <= queue_lamps_port1.rear; i++)
      {
        debug(MAIN_DEBUG, "%d ", queue_lamps_port1.inp_arr[i]);
      }
    }
  }
  else if (port == 2)
  {
    if (queue_lamps_port2.front == -1)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Empty queue lamps port2");
    }
    else
    {
      debug(MAIN_DEBUG, "%s\n", "queue_lamps_port2: ");
      for (int i = queue_lamps_port2.front; i <= queue_lamps_port2.rear; i++)
      {
        debug(MAIN_DEBUG, "%d ", queue_lamps_port2.inp_arr[i]);
      }
    }
  }
}

void insert_queue_lamp_port(uint8_t insert_item, int port)
{
  bool enable_save_queue = true;
  int i;
  if (port == 1)
  {
    if (queue_lamps_port1.rear == SIZE_QUEUE_ARRAY - 1)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Queue overflow");
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
        debug(MAIN_DEBUG, "%s\r\n", "Element to be inserted lamp to port 1 RS485 in the queue");
        queue_lamps_port1.rear = queue_lamps_port1.rear + 1;
        queue_lamps_port1.inp_arr[queue_lamps_port1.rear] = insert_item;
      }
      else
      {
        debug(MAIN_DEBUG, "%s\r\n", "Exist element in the queue");
      }
    }
  }
  else if (port == 2)
  {
    if (queue_lamps_port2.rear == SIZE_QUEUE_ARRAY - 1)
    {
      debug(MAIN_DEBUG, "%s\r\n", "Queue overflow");
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
        debug(MAIN_DEBUG, "%s\r\n", "Element to be inserted lamp to port 2 RS485 in the queue");
        queue_lamps_port2.rear = queue_lamps_port2.rear + 1;
        queue_lamps_port2.inp_arr[queue_lamps_port2.rear] = insert_item;
      }
      else
      {
        debug(MAIN_DEBUG, "%s\r\n", "Exist element in the queue");
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
      debug(MAIN_DEBUG, "%s\r\n", "Underflow");
      queue_lamps_port1.front = -1;
      queue_lamps_port1.rear = -1;
    }
    else
    {
      debug(MAIN_DEBUG, "Element deleted from the Queue lamp port 1: %d\n", queue.inp_arr[queue_lamps_port1.front]);
      queue_lamps_port1.front = queue_lamps_port1.front + 1;
      debug(MAIN_DEBUG, "gia tri queue front lamp port 1: %d\r\n", queue_lamps_port1.front);
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
      debug(MAIN_DEBUG, "%s\r\n", "Underflow");
      queue_lamps_port2.front = -1;
      queue_lamps_port2.rear = -1;
    }
    else
    {
      debug(MAIN_DEBUG, "Element deleted from the Queue lamp port 2: %d\n", queue.inp_arr[queue_lamps_port2.front]);
      queue_lamps_port2.front = queue_lamps_port2.front + 1;
      debug(MAIN_DEBUG, "gia tri queue front lamp port 2: %d\r\n", queue_lamps_port2.front);
      if (queue_lamps_port2.front > queue_lamps_port2.rear)
      {
        queue_lamps_port2.front = -1;
        queue_lamps_port2.rear = -1;
      }
    }
  }
}

void send_time_to_cpu_monitoring(type__mtfc_hm_time_t *mtfc_hm_present, type_RTC_monitoring_t *mtlc_time_master_processing)
{
  if (mtlc_time_master_processing->hour != mtfc_hm_present->hour || mtlc_time_master_processing->min != mtfc_hm_present->minute)
  {
    mtlc_time_master_processing->hour = mtfc_hm_present->hour;
    mtlc_time_master_processing->min = mtfc_hm_present->minute;
    outCom1.send_struct(CMD_MASTER_CPU_TIME_SET_CURRENT,
                        (uint8_t *)mtlc_time_master_processing, sizeof(type_RTC_monitoring_t));
    debug(MAIN_DEBUG, "hour : %d , minute : %d , hour_rtc : %d, min_rtc: %d\r\n",
          mtlc_time_master_processing->hour, mtlc_time_master_processing->min, mtfc_hm_present->hour, mtfc_hm_present->minute);
  }
}

void reset_lamp_group_all_in_branch1(void)
{
  for (int i = 1; i <= lamp_total_default; i++)
  {
    lamp_group_all_p1.inp_id[i] = i;
  }
}

void reset_lamp_group_all_in_branch2(void)
{
  for (int i = 1; i <= lamp_total_default; i++)
  {
    lamp_group_all_p2.inp_id[i] = i;
  }
}

void initizaline_all_lamp_in_branch(void)
{
  reset_lamp_group_all_in_branch1();
  SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1);
  SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&lamp_group_all_p1, sizeof(change_lamp_to_group_t));
  reset_lamp_group_all_in_branch2();
  SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2);
  SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&lamp_group_all_p2, sizeof(change_lamp_to_group_t));

  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&lamp_group_all_p2, sizeof(change_lamp_to_group_t));
  // for (int i = 1; i <= lamp_total_default; i++)
  // {
  //   debug(MAIN_DEBUG, "lamp group all p21: %d\r\n", lamp_group_all_p2.inp_id[i]);
  // }
  SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&lamp_group_all_p1, sizeof(change_lamp_to_group_t));
}

void compare_lamp_group_with_group_all(change_lamp_to_group_t number_group_port, change_lamp_to_group_t group_all_port, int port)
{
  if (port == 1)
  {
    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&group_all_port, sizeof(change_lamp_to_group_t));

    for (int i = 0; i <= number_group_port.rear; i++)
    {
      for (int j = 1; j <= lamp_total_default; j++)
      {
        if (group_all_port.inp_id[j] == number_group_port.inp_id[i])
        {
          group_all_port.inp_id[j] = 0;
        }
      }
    }

    SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1);
    SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&group_all_port, sizeof(change_lamp_to_group_t));

    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT1, (char *)&group_all_port, sizeof(change_lamp_to_group_t));
    // for (int i = 0; i <= lamp_total_default; i++)
    // {
    //   debug(MAIN_DEBUG, "id lamp group all port 1: %d\r\n", group_all_port.inp_id[i]);
    // }
  }
  else if (port == 2)
  {
    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&group_all_port, sizeof(change_lamp_to_group_t));

    for (int i = 0; i <= number_group_port.rear; i++)
    {
      for (int j = 1; j <= lamp_total_default; j++)
      {
        if (group_all_port.inp_id[j] == number_group_port.inp_id[i])
        {
          group_all_port.inp_id[j] = 0;
        }
      }
    }
    SerialFlash.eraseBlock(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2);
    SerialFlash.write(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&group_all_port, sizeof(change_lamp_to_group_t));

    SerialFlash.read(ADDR_STORAGE_SET_LAMP_GROUP_ALL_PORT2, (char *)&group_all_port, sizeof(change_lamp_to_group_t));
    // for (int i = 0; i <= lamp_total_default; i++)
    // {
    //   debug(MAIN_DEBUG, "id lamp group all port 2: %d\r\n", group_all_port.inp_id[i]);
    // }
  }
}
#pragma endregion