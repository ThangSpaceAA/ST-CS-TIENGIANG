#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <bsp_config.h>
#include <mspSerial.h>
#include <IWatchdog.h>

#include "flash.h"
#include "main.h"

#define setPin(n) digitalWrite(n, HIGH)
#define clearPin(n) digitalWrite(n, LOW)

/**
 * @brief Khai bao OOP
 *
 */
HardwareSerial debugComPort(DEBUG_RX_PIN, DEBUG_TX_PIN);
HardwareSerial cardComPort(CARD_RX_PIN, CARD_TX_PIN);
mspSerial cardCom(&cardComPort, 115200);

#define debug(condition, fmt, ...)           \
  do                                         \
  {                                          \
    if (condition)                           \
      debugComPort.printf(fmt, __VA_ARGS__); \
  } while (0)

//------------------------------------------------------------------------------------

#pragma region KHAI BAO HAM MACRO
/**
 * @brief Khai bao macro ham
 *
 */

/**
 * @brief Ham ghi bo nho
 *
 * @param pos vi tri
 * @param dat du lieu
 * @param siz kich thuoc
 */
void mtfc_read_eeprom_api(uint8_t pos, uint8_t *dat, uint16_t siz);

/**
 * @brief Ham doc bo nho
 *
 * @param pos vi tri
 * @param dat du lieu
 * @param siz kich thuoc
 */
void mtfc_write_eeprom_api(uint8_t pos, uint8_t *dat, uint16_t siz);

/**
 * @brief Ham doc  cai dat cua thiet bi
 *
 */
void mtfc_memory_startup_load(void);

/**
 * @brief Ham callback sau khi du lieu cong giao tiep nhan du farme truyen
 *
 */
void mtfc_card_com_event_handler(void);

/**
 * @brief Ham xu lieu tu cpu gui den
 *
 */
void mtfc_card_data_processing(void);

/**
 * @brief Doc cam bien ngo ra va tim loi
 *
 */

/**
 * @brief Delete all data saved flash(w25q32)
 */
void delete_all_settings(void);

/**
 * @brief function read from flash(w25q32)
 *
 * @param addr block read data
 * @param buf  data out
 * @param len  size buffer
 */
void mtfc_read_flash_api(uint32_t addr, void *buf, uint32_t len);

/**
 * @brief function write to flash (w25q32)
 *
 * @param addr block save data
 * @param buf  data in
 * @param len  size buffer
 */
void mtfc_write_flash_api(uint32_t addr, const void *buf, uint32_t len);

/**
 * @brief controll pin dim off and pin dim relay
 */
void mtfc_control_pin_dim(int value_brightness);

dim_active_t dim_active;
add_t add;
add_p_t add_p;
type_lightAddConfig_t ttrb;
add_p_t add_defaut;
group_t group;
type_configPackage_t counters_package;
type_flagIncom_t flagIncom;
type_lightAddConfig_t type_lightAddConfig;
change_id_t change_id;
uint8_t countlamprf = 0;
type_lightAddConfig_t lightAddConfigBuff;
volatile uint8_t brightness = 0;
uint8_t tempAddress = 0;

add_lamp_dim_group_t get_info_dim;

// thay doi cong suat den
change_power_lamp_t change_lamp_power;

#pragma endregion

#pragma region CAC HAM CHINH
void mtfc_startup_config(void)
{
  pinMode(DIM_PIN, OUTPUT);
  pinMode(DIM_OFF_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ACT_DIM_PIN, OUTPUT);
  digitalWrite(ACT_DIM_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  SerialFlash.begin(FLASH_CHIP_SELECT);

  debugComPort.begin(115200);
  cardComPort.begin(115200);
  delay(0.1);
  debugComPort.printf("\r\nCom1");
  cardComPort.printf("\r\nCom2");
}

void setup()
{
  mtfc_startup_config();
  mtfc_memory_startup_load();
  cardCom.attachInterupt(mtfc_card_com_event_handler);
  IWatchdog.begin(10000000);
}

/**
 * @brief Chan led trang thai hoat dong
 * [soild]: cmd mode
 * [fast flash 200ms]: no config
 * [slow flash 1s]: active mode
 */
uint32_t mill_cnt = millis();
uint8_t cnt = 0;
void loop()
{
  cardCom.isr_event();
  mtfc_card_data_processing();
  IWatchdog.reload();
}

#pragma endregion

#pragma region XU LY DU LIEU NHAN DUOC

void mtfc_card_data_processing(void)
{
  // Processing Dim
  if (flagIncom.setdimvalue)
  {
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    if(add_defaut.cabinetid == add_p.cabinetid)
    {
      debug(MAIN_DEBUG, "\r\n%s", "Dim all suscess");
      brightness = add_p.dim;
      mtfc_control_pin_dim(brightness);
      analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
      add_defaut.dim = add_p.dim;
    }
    flagIncom.setdimvalue = false;
  }
  // Processing dim follow ID Lamp
  if (flagIncom.setdimid)
  {
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    if (add_defaut.id == add_p.id && add_defaut.cabinetid == add_p.cabinetid)
    {
      add_defaut.port = add_p.port;
      add_defaut.group = add_p.group;
      add_defaut.dim = add_p.dim;
      debug(MAIN_DEBUG, "\r\n%s", "Dim id success");
      brightness = add_p.dim;
      mtfc_control_pin_dim(brightness);
      analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
      cardCom.send_byte(CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB, 2);
    }
    flagIncom.setdimid = false;
  }

  if (flagIncom.setdimgroup)
  {
    for (int i = 0; i <= lamp_total_default; i++)
    {
      if (add_defaut.id == get_info_dim.id_dim[i])
      {
        brightness = get_info_dim.dim_value;
        mtfc_control_pin_dim(brightness);
        analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
        add_defaut.dim = get_info_dim.dim_value;
      }
    }
  }
  // Dem so luong den dang ket noi
  if (flagIncom.countlamp)
  {
    tempAddress = 0;
    tempAddress = counters_package.val1;
    if (counters_package.val1 < 240)
      counters_package.val1++;
    flagIncom.countlamp = false;
  }
  // Dim theo dia chi
  if (flagIncom.setdimaddress)
  {
    // memory.read((uint32_t)ADD_SAVE_SETTING, (uint8_t*)&lightAddConfigBuff, sizeof(type_lightAddConfig_t));
    if (add.add == lightAddConfigBuff.id)
    {
      // lampCom.send_struct(153, (uint8_t *)&add, sizeof(add_t));
      lightAddConfigBuff.id = add.add;
      lightAddConfigBuff.group = 200;
      // memory.write((uint32_t)ADD_SAVE_SETTING, (uint8_t*)&lightAddConfigBuff, sizeof(type_lightAddConfig_t));
    }
    flagIncom.setdimaddress = false;
  }

  // Xử lý giải thuật cài đặt địa chỉ đèn
  if (flagIncom.isCounterAddLamp) // Giải thuật đếm địa chỉ của đèn
  {
    tempAddress = 0;
    tempAddress = counters_package.val1;
    if (counters_package.val1 < 240)
      counters_package.val1++;
    // cardCom.send_struct(CMD_MASTER_TO_SLAVE_COUNT_LAMP,(uint8_t*)&counters_package, sizeof(type_configPackage_t));
    // inCom.send_byte(LSV_NODE_CONNECT_FB,1);
    // timeOutEnabled(5.0,PORT_COUNTER);
    // debug(MAIN_DEBUG,"\r\nLight add: [%03d]---Group: [%02d]\n", type_lightAddConfig.id, type_lightAddConfig.group);
    flagIncom.isCounterAddLamp = false;
  }

  // 
  if (flagIncom.changeidrf) // change ID cho nema
  {
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    if ((change_id.oldid == add_defaut.id))
    {
      add_defaut.id = change_id.newid;
      add_defaut.cabinetid = change_id.cabinetid;
      delay(1000);
      mtfc_write_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
      cardCom.send_byte(CMD_SLAVE_TO_MASTER_CHANGE_ID_RF_FB, 1);
    }
    cardCom.send_struct(CMD_MASTER_TO_SLAVE_CHANGE_ID_RF, (uint8_t *)&change_id, sizeof(change_id_t));
    flagIncom.changeidrf = false;
  }

  if (flagIncom.countlamprf) // Giải thuật đếm địa chỉ của đèn
  {
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    debug(MAIN_DEBUG, "\r\nDim value: [%03d]", add_defaut.dim);
    debug(MAIN_DEBUG, "\r\nid value: [%03d]", add_defaut.id);
    debug(MAIN_DEBUG, "\r\nport value: [%03d]", add_defaut.port);
    debug(MAIN_DEBUG, "\r\ngroup value: [%03d]", add_defaut.group);
    debug(MAIN_DEBUG, "\r\ninCom.read8(): [%03d]", countlamprf);
    if ((countlamprf == add_defaut.id)) //&& (add_p.port == add_defaut.port))
    {
      cardCom.send_struct(CMD_SLAVE_TO_MASTER_COUNT_LAMP_RF_FB, (uint8_t *)&add_defaut, sizeof(add_p_t));
      debug(MAIN_DEBUG, "\r\nDa send gia tri den thu: [%03d] ve master", add_defaut.id);
    }
    // outCom3.send_struct(CMD_MASTER_TO_SLAVE_CHANGE_ID_RF, (uint8_t *)&change_id, sizeof(change_id_t));
    flagIncom.countlamprf = false;
  }
}

#pragma endregion

#pragma region INTERRUP READ STEAM DATA CART_RX
void mtfc_card_com_event_handler(void)
{
  debug(MAIN_DEBUG, "\r\ncardCom.getMSP():%d\n", cardCom.getMSP());
  switch (cardCom.getMSP())
  {
  case CMD_MASTER_TO_SLAVE_COUNT_LAMP:
    cardCom.readstruct((uint8_t *)&counters_package, sizeof(type_configPackage_t));
    flagIncom.isCounterAddLamp = true;
    break;
  case CMD_MASTER_TO_SLAVE_DIM_ALL:
    cardCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    // cardCom.send_struct(CMD_MASTER_TO_SLAVE_DIM_ALL, (uint8_t *)&add_p, sizeof(add_p_t));
    debug(MAIN_DEBUG, "\r\nDim all value: [%03d]", add_p.dim);
    debug(MAIN_DEBUG, "\r\nid value: [%03d]", add_p.id);
    debug(MAIN_DEBUG, "\r\nport value: [%03d]", add_p.port);
    debug(MAIN_DEBUG, "\r\ngroup value: [%03d]", add_p.group);
    flagIncom.setdimvalue = true;
    break;
  case CMD_MASTER_TO_SLAVE_DIM_ID:
    cardCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    // cardCom.send_struct(CMD_MASTER_TO_SLAVE_DIM_ID, (uint8_t *)&add_p, sizeof(add_p_t));
    debug(MAIN_DEBUG, "\r\nDim id value: [%03d]", add_p.dim);
    debug(MAIN_DEBUG, "\r\nid value: [%03d]", add_p.id);
    debug(MAIN_DEBUG, "\r\nport value: [%03d]", add_p.port);
    debug(MAIN_DEBUG, "\r\ngroup value: [%03d]", add_p.group);
    flagIncom.setdimid = true;
    break;
  case CMD_MASTER_TO_SLAVE_DIM_GROUP:
  {
    cardCom.readstruct((uint8_t *)&get_info_dim, sizeof(add_lamp_dim_group_t));
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));

    for (int i = 0; i <= lamp_total_default; i++)
    {
      if (add_defaut.id == get_info_dim.id_dim[i] && (add_defaut.cabinetid == get_info_dim.cabinetid))
      {
        brightness = get_info_dim.dim_value;
        mtfc_control_pin_dim(brightness);
        analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
        add_defaut.dim = get_info_dim.dim_value;
      }
    }
    debug(MAIN_DEBUG, "\r\nDim group value: [%03d]", brightness);
    debug(MAIN_DEBUG, "\r\nid value: [%03d]", add_p.id);
    debug(MAIN_DEBUG, "\r\nport value: [%03d]", add_p.port);
    debug(MAIN_DEBUG, "\r\ngroup value: [%03d]", add_p.group);
    // flagIncom.setdimgroup = true;
  }
  break;
  case CMD_MASTER_TO_SLAVE_CHANGE_POWER_LAMP:
  {
    cardCom.readstruct((uint8_t*)&change_lamp_power, sizeof(change_power_lamp_t));
    mtfc_write_flash_api(ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP, (uint8_t*)&change_lamp_power, sizeof(change_power_lamp_t));
    delay(100);
    mtfc_read_flash_api(ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP, (uint8_t *)&change_lamp_power, sizeof(change_power_lamp_t));
    
  }
  break;
  case CMD_MASTER_TO_SLAVE_SETUP_INFO:
    cardCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    debug(MAIN_DEBUG, "\r\nInfor Dim value: [%03d]", add_p.dim);
    debug(MAIN_DEBUG, "\r\nInfor value: [%03d]", add_p.id);
    debug(MAIN_DEBUG, "\r\nInfor port value: [%03d]", add_p.port);
    debug(MAIN_DEBUG, "\r\nnInfor group value: [%03d]", add_p.group);
    flagIncom.setupinfor = true;
    break;
  case CMD_MASTER_TO_SLAVE_ADD_ID_TO_GROUP:
    cardCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    debug(MAIN_DEBUG, "\r\nSet group Dim value: [%03d]", add_p.dim);
    debug(MAIN_DEBUG, "\r\nSet group value: [%03d]", add_p.id);
    debug(MAIN_DEBUG, "\r\nSet group port value: [%03d]", add_p.port);
    debug(MAIN_DEBUG, "\r\nSet group group value: [%03d]", add_p.group);
    flagIncom.addidtogorup = true;
    break;
  case CMD_MASTER_TO_SLAVE_DIM_ALL_CUSTOM:
  {
    cardCom.readstruct((uint8_t*)&get_info_dim, sizeof(add_lamp_dim_group_t));
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t*)&add_defaut, sizeof(add_p_t));

    for(int i = 0 ; i <= lamp_total_default; i++)
    {
      if(add_defaut.id == get_info_dim.id_dim[i] && (add_defaut.cabinetid == get_info_dim.cabinetid))
      {
        brightness = get_info_dim.dim_value;
        mtfc_control_pin_dim(brightness);
        analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
        add_defaut.dim = get_info_dim.dim_value;
      }
    }
  }
  break;  
  case 151:
    cardCom.readstruct((uint8_t *)&type_lightAddConfig, sizeof(type_lightAddConfig_t));
    flagIncom.setaddress = true;
    break;
  case 152:
    cardCom.readstruct((uint8_t *)&add, sizeof(add_t));
    flagIncom.setdimaddress = true;
    break;
  case 154:
    cardCom.readstruct((uint8_t *)&counters_package, sizeof(counters_package));
    flagIncom.countlamp = true;
    break;
  case 155:
    cardCom.readstruct((uint8_t *)&add_p, sizeof(add_p_t));
    flagIncom.setgroup = true;
    break;
  case 156:
    cardCom.readstruct((uint8_t *)&group, sizeof(group_t));
    flagIncom.dimgroup = true;
    break;

  case CMD_MASTER_TO_SLAVE_CHANGE_ID_RF:
    cardCom.readstruct((uint8_t *)&change_id, sizeof(change_id_t));
    flagIncom.changeidrf = true;
    break;

  case CMD_MASTER_TO_SLAVE_COUNT_LAMP_RF:
    countlamprf = cardCom.read8();
    flagIncom.countlamprf = true;
    break;
  default:
    break;
  }
}
#pragma endregion

#pragma region HAM LIEN QUAN BO NHO
uint8_t tmp_power_lamp;
void mtfc_memory_startup_load()
{
  mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
  mtfc_read_flash_api(ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP, (uint8_t*)&change_lamp_power, sizeof(change_power_lamp_t));

  debug(MAIN_DEBUG, "\r\nDim value: [%03d]", add_defaut.dim);
  debug(MAIN_DEBUG, "\r\nid value: [%03d]", add_defaut.id);
  debug(MAIN_DEBUG, "\r\nport value: [%03d]", add_defaut.port);
  debug(MAIN_DEBUG, "\r\ngroup value: [%03d]", add_defaut.group);
  debug(MAIN_DEBUG, "\r\nID cabinet: [%03d]", add_defaut.cabinetid);
  // debug(MAIN_DEBUG, "\r\npower lamp value: [%.2f]", (float)tmp_power_lamp);

  if (add_defaut.id == 255 || add_defaut.id == 0)
  {
    add_defaut.id = ID_DEFAULT;
    add_defaut.port = PORT_DEFAULT;
    add_defaut.group = GROUP_DEFAULT;
    add_defaut.dim = VALUE_DIM_DEFAULT;
    change_lamp_power.value_power_lamp = POWER_LAMP_DEFAULT;
    mtfc_write_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    mtfc_write_flash_api(ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP, (uint8_t *)&change_lamp_power, sizeof(change_power_lamp_t));
    delay(200);
    mtfc_read_flash_api(ADDR_STORAGE_DIM_CONTROL, (uint8_t *)&add_defaut, sizeof(add_p_t));
    mtfc_read_flash_api(ADDR_STORAGE_ADDRESS_CHANGE_POWER_LAMP, (uint8_t*)&change_lamp_power, sizeof(change_power_lamp_t));
  }
  brightness = VALUE_DIM_DEFAULT;
  analogWrite(DIM_PIN, brightness * 255 / 100 * ((float)change_lamp_power.value_power_lamp / 100));
}

void mtfc_control_pin_dim(int value_brightness)
{
  if (value_brightness == 0)
  {
    digitalWrite(DIM_OFF_PIN, HIGH);
    digitalWrite(RELAY_PIN, LOW); // Dong tai ngo ra
  }
  else
  {
    digitalWrite(DIM_OFF_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH); // Mo tai ngo ra
  }
}

void mtfc_write_eeprom_api(uint8_t pos, uint8_t *dat, uint16_t siz)
{
  uint8_t temp = 0;
  uint16_t address = pos;
  while (siz--)
  {
    temp = *dat;
    delay(100);
    EEPROM.write(address++, temp);
    dat++;
  }
}

void mtfc_read_eeprom_api(uint8_t pos, uint8_t *dat, uint16_t siz)
{
  uint16_t address = pos;
  for (uint16_t i = 0; i < siz; i++)
  {
    *dat = EEPROM.read(address++);
    dat++;
  }
}

void delete_all_setting(void)
{
  SerialFlash.eraseAll();
}

void mtfc_write_flash_api(uint32_t addr, const void *buf, uint32_t len)
{
  SerialFlash.eraseBlock(addr);
  delay(2);
  SerialFlash.write(addr, buf, len);
}

void mtfc_read_flash_api(uint32_t addr, void *buf, uint32_t len)
{
  SerialFlash.read(addr, buf, len);
}
#pragma endregion