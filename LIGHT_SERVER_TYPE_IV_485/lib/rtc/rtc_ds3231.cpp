/******************************************************************/ /**
* @file ds3231.cpp
*
* @author Justin Jordan
*
* @version 1.0
*
* Started: 11NOV14
*
* Updated: 
*
* @brief Source file for DS3231 class
*
***********************************************************************
*
* @copyright 
* Copyright (C) 2015 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
**********************************************************************/

#include "rtc_ds3231.h"
#include "debug.h"


/**********************************************************/ /**
* Constructor for Ds3231 Class
*
* On Entry:
*     @param[in] sda - sda pin of I2C bus
*     @param[in] scl - scl pin of I2C bus
*
* On Exit:
*    @return none
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
*
* @endcode
**************************************************************/
Ds3231::Ds3231(PinName sda, PinName scl) : I2C(sda, scl)
{
    w_adrs = ((DS3231_I2C_ADRS << 1) | I2C_WRITE);
    r_adrs = ((DS3231_I2C_ADRS << 1) | I2C_READ);
}

/**********************************************************/ /**
* Sets the time on DS3231
* Struct data is in integrer format, not BCD.  Fx will convert
* to BCD for you.
*
* On Entry:
*     @param[in] time - struct cotaining time data; 
*
* On Exit:
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //time = 12:00:00 AM 12hr mode
* ds3231_time_t time = {12, 0, 0, 0, 1}
* uint16_t rtn_val;
*
* rtn_val = rtc.set_time(time);
*
* @endcode
**************************************************************/
uint16_t Ds3231::set_time_ds(ds3231_time_t time)
{
    uint8_t data[] = {0, 0, 0, 0};
    uint8_t data_length = 0;
    uint8_t max_hour = 24;

    data[data_length++] = SECONDS;
    data[data_length++] = uchar_2_bcd(time.seconds);
    data[data_length++] = uchar_2_bcd(time.minutes);

    //format Hours register
    data[data_length] = uchar_2_bcd(time.hours);
    if (time.mode)
    {
        max_hour = max_hour / 2;

        data[data_length] |= MODE;
        if (time.am_pm)
        {
            data[data_length] |= AM_PM;
        }
    }
    else
    {
        max_hour = max_hour - 1;
    }
    data_length++;

    //Make sure data is within range.
    if ((time.seconds > 59) || (time.minutes > 59) || (time.hours > max_hour))
    {
        debug(SETTIME_DEBUG,"\r\n%s","[ERROR] Time out range");
        return (1);
    }
    else
    {
        return (write(w_adrs, (const char *)data, data_length));
    }
}

/**********************************************************/ /**
* Sets the calendar on DS3231
*
* On Entry:
*     @param[in] calendar - struct cotaining calendar data 
*
* On Exit:
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //see datasheet for calendar format
* ds3231_calendar_t calendar = {1, 1, 1, 0}; 
* uint16_t rtn_val;
*
* rtn_val = rtc.set_calendar(calendar);
*
* @endcode
**************************************************************/
uint16_t Ds3231::set_calendar(ds3231_calendar_t calendar)
{
    uint8_t data[] = {0, 0, 0, 0, 0};
    uint8_t data_length = 0;

    data[data_length++] = DAY;
    data[data_length++] = uchar_2_bcd(calendar.day);
    data[data_length++] = uchar_2_bcd(calendar.date);
    data[data_length++] = uchar_2_bcd(calendar.month);
    data[data_length++] = uchar_2_bcd(calendar.year);

    //Make sure data is within range.
    if (((calendar.day < 1) || (calendar.day > 7)) ||
        ((calendar.date < 1) || (calendar.date > 31)) ||
        ((calendar.month < 1) || (calendar.month > 12)) ||
        (calendar.year > 99))
    {
        debug(SETTIME_DEBUG,"\r\n%s","[ERROR] Calendar out range");
        return (1);
    }
    else
    {
        return (write(w_adrs, (const char *)data, data_length));
    }
}

/**********************************************************/ /**
* Set either Alarm1 or Alarm2 of DS3231
*
* On Entry:
*     @param[in] alarm - struct cotaining alarm data 
*                        
*     @param[in] one_r_two - TRUE for Alarm1 and FALSE for 
*                            Alarm2
*
* On Exit:
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //see ds3231.h for .members and datasheet for alarm format
* ds3231_alrm_t alarm; 
* uint16_t rtn_val;
*
* rtn_val = rtc.set_alarm(alarm, FALSE);
*
* @endcode
**************************************************************/
uint16_t Ds3231::set_alarm(ds3231_alrm_t alarm, bool one_r_two)
{
    uint8_t data[] = {0, 0, 0, 0, 0};
    uint8_t data_length = 0;
    uint8_t max_hour = 24;
    uint8_t mask_var = 0;

    //setting alarm 1 or 2?
    if (one_r_two)
    {
        data[data_length++] = ALRM1_SECONDS;

        //config seconds register
        if (alarm.am1)
        {
            mask_var |= ALRM_MASK;
        }
        data[data_length++] = (mask_var | uchar_2_bcd(alarm.seconds));
        mask_var = 0;

        //config minutes register
        if (alarm.am2)
        {
            mask_var |= ALRM_MASK;
        }
        data[data_length++] = (mask_var | uchar_2_bcd(alarm.minutes));
        mask_var = 0;

        //config hours register
        if (alarm.am3)
        {
            mask_var |= ALRM_MASK;
        }
        if (alarm.mode)
        {
            max_hour = max_hour / 2;
            mask_var |= MODE;
            if (alarm.am_pm)
            {
                mask_var |= AM_PM;
            }
        }
        else
        {
            max_hour = max_hour - 1;
        }
        data[data_length++] = (mask_var | uchar_2_bcd(alarm.hours));
        mask_var = 0;

        //config day/date register
        if (alarm.am4)
        {
            mask_var |= ALRM_MASK;
        }
        if (alarm.dy_dt)
        {
            mask_var |= DY_DT;
            data[data_length++] = (mask_var | uchar_2_bcd(alarm.day));
        }
        else
        {
            data[data_length++] = (mask_var | uchar_2_bcd(alarm.date));
        }
        mask_var = 0;
    }
    else
    {
        data[data_length++] = ALRM2_MINUTES;

        //config minutes register
        if (alarm.am2)
        {
            mask_var |= ALRM_MASK;
        }
        data[data_length++] = (mask_var | uchar_2_bcd(alarm.minutes));
        mask_var = 0;

        //config hours register
        if (alarm.am3)
        {
            mask_var |= ALRM_MASK;
        }
        if (alarm.mode)
        {
            max_hour = max_hour / 2;
            mask_var |= MODE;
            if (alarm.am_pm)
            {
                mask_var |= AM_PM;
            }
        }
        else
        {
            max_hour = max_hour - 1;
        }
        data[data_length++] = (mask_var | uchar_2_bcd(alarm.hours));
        mask_var = 0;

        //config day/date register
        if (alarm.am4)
        {
            mask_var |= ALRM_MASK;
        }
        if (alarm.dy_dt)
        {
            mask_var |= DY_DT;
            data[data_length++] = (mask_var | uchar_2_bcd(alarm.day));
        }
        else
        {
            data[data_length++] = (mask_var | uchar_2_bcd(alarm.date));
        }
        mask_var = 0;
    }

    //Make sure data is within range.
    if ((alarm.seconds > 59) || (alarm.minutes > 59) || (alarm.hours > max_hour) ||
        ((alarm.day < 1) || (alarm.day > 7)) ||
        ((alarm.date < 1) || (alarm.date > 31)))
    {
        return (1);
    }
    else
    {
        return (write(w_adrs, (const char *)data, data_length));
    }
}

/**********************************************************/ /**
* Ham cai dat alarm 1
**************************************************************/
void Ds3231::alarm_one_set(uint8_t hour, uint8_t minute, uint8_t sec)
{
    //Bien de luu thanh ghi trang thai
    ds3231_cntl_stat_t rtc_control_status;
    //luu thoi gian cai dat cho alarm
    ds3231_alrm_t alarm1_config;

    alarm1_config.seconds = sec;
    alarm1_config.minutes = minute;
    alarm1_config.hours = hour;
    alarm1_config.day = 1;
    alarm1_config.date = 1;
    alarm1_config.am4 = true;
    alarm1_config.am3 = false;
    alarm1_config.am2 = false;
    alarm1_config.am1 = false;
    alarm1_config.am_pm = false;
    alarm1_config.mode = false;
    alarm1_config.dy_dt = false;
    //Doc gia tri hien tai cua thanh ghi control
    get_cntl_stat_reg(&rtc_control_status);
    //Xoa thanh ghi status
    rtc_control_status.status = 0;
    //Bat co gat cua alarm 1
    rtc_control_status.control = rtc_control_status.control | 0x05;
    //Luu gia tri cai dat alarm 1
    set_alarm(alarm1_config, true);
    //Luu gia tri cua thanh ghi control
    set_cntl_stat_reg(rtc_control_status);
}

/**********************************************************/ /**
* Ham cai dat alarm 2
**************************************************************/
void Ds3231::alarm_two_set(uint8_t hour, uint8_t minute, uint8_t sec)
{
    //Bien de luu thanh ghi trang thai
    ds3231_cntl_stat_t rtc_control_status;
    //luu thoi gian cai dat cho alarm
    ds3231_alrm_t alarm2_config;

    alarm2_config.seconds = sec;
    alarm2_config.minutes = minute;
    alarm2_config.hours = hour;
    alarm2_config.day = 1;
    alarm2_config.date = 1;
    alarm2_config.am4 = true;
    alarm2_config.am3 = false;
    alarm2_config.am2 = false;
    alarm2_config.am1 = false;
    alarm2_config.am_pm = false;
    alarm2_config.mode = false;
    alarm2_config.dy_dt = false;
    //Doc gia tri hien tai cua thanh ghi control
    get_cntl_stat_reg(&rtc_control_status);
    //Xoa thanh ghi status
    rtc_control_status.status = 0;
    //Bat co gat cua alarm 2
    rtc_control_status.control = rtc_control_status.control | 0x06;
    //Luu gia tri cai dat alarm 2
    set_alarm(alarm2_config, false);
    //Luu gia tri cua thanh ghi control
    set_cntl_stat_reg(rtc_control_status);
}

/**********************************************************/ /**
* Set control and status registers of DS3231
*
* On Entry:
*     @param[in] data - Struct containing control and status 
*                       register data
*
* On Exit:
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15);  
* 
* //do not use 0xAA, see datasheet for appropriate data 
* ds3231_cntl_stat_t data = {0xAA, 0xAA}; 
*
* rtn_val = rtc.set_cntl_stat_reg(data);
*
* @endcode
**************************************************************/
uint16_t Ds3231::set_cntl_stat_reg(ds3231_cntl_stat_t data)
{
    uint8_t local_data[] = {0, 0, 0};
    uint8_t data_length = 0;

    local_data[data_length++] = CONTROL;
    local_data[data_length++] = data.control;
    local_data[data_length++] = data.status;

    //users responsibility to make sure data is logical
    return (write(w_adrs, (const char *)local_data, data_length));
}

/**********************************************************/ /**
* Gets the time on DS3231
*
* On Entry:
*     @param[in] time - pointer to struct for storing time data
*
* On Exit:
*     @param[out] time - contains current integrer rtc time 
*                        data
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //time = 12:00:00 AM 12hr mode
* ds3231_time_t time = {12, 0, 0, 0, 1} 
* uint16_t rtn_val;
*
* rtn_val = rtc.get_time(&time);
*
* @endcode
**************************************************************/
uint16_t Ds3231::get_time(ds3231_time_t *time)
{
    uint16_t rtn_val = 1;
    uint8_t data[3];

    data[0] = SECONDS;
    rtn_val = write(w_adrs, (const char *)data, 1);

    if (!rtn_val)
    {
        rtn_val = read(r_adrs, (char *)data, 3);

        time->seconds = bcd_2_uchar(data[0]);
        time->minutes = bcd_2_uchar(data[1]);
        time->am_pm = (data[2] & AM_PM);
        time->mode = (data[2] & MODE);

        if (time->mode)
        {
            time->hours = bcd_2_uchar((data[2] & 0x1F));
        }
        else
        {
            time->hours = bcd_2_uchar((data[2] & 0x3F));
        }
    }

    return (rtn_val);
}

/**********************************************************/ /**
* Gets the calendar on DS3231
*
* On Entry:
*     @param[in] calendar - pointer to struct for storing 
*                           calendar data
*
* On Exit:
*     @param[out] calendar - contains current integer rtc 
*                            calendar data
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //see datasheet for calendar format
* ds3231_calendar_t calendar = {1, 1, 1, 0}; 
* uint16_t rtn_val;
*
* rtn_val = rtc.get_calendar(&calendar);
*
* @endcode
**************************************************************/
uint16_t Ds3231::get_calendar(ds3231_calendar_t *calendar)
{
    uint16_t rtn_val = 1;
    uint8_t data[4];

    data[0] = DAY;
    rtn_val = write(w_adrs, (const char *)data, 1);

    if (!rtn_val)
    {
        rtn_val = read(r_adrs, (char *)data, 4);

        calendar->day = bcd_2_uchar(data[0]);
        calendar->date = bcd_2_uchar(data[1]);
        calendar->month = bcd_2_uchar((data[2] & 0x1F));
        calendar->year = bcd_2_uchar(data[3]);
    }

    return (rtn_val);
}

/**********************************************************/ /**
* Get either Alarm1 or Alarm2 of DS3231
*
* On Entry:
*     @param[in] alarm - pointer to struct for storing alarm 
*                        data; 
*
*     @param[in] one_r_two - TRUE for Alarm1 and FALSE for 
*                            Alarm2
*
* On Exit:
*     @param[out] alarm - contains integer alarm data
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* //see ds3231.h for .members and datasheet for alarm format
* ds3231_alrm_t alarm; 
* uint16_t rtn_val;
*
* rtn_val = rtc.get_alarm(&alarm, FALSE);
*
* @endcode
**************************************************************/
uint16_t Ds3231::get_alarm(ds3231_alrm_t *alarm, bool one_r_two)
{
    uint16_t rtn_val = 1;
    uint8_t data[4];

    if (one_r_two)
    {
        data[0] = ALRM1_SECONDS;
        rtn_val = write(w_adrs, (const char *)data, 1);

        if (!rtn_val)
        {
            rtn_val = read(r_adrs, (char *)data, 4);

            alarm->seconds = bcd_2_uchar(data[0] & 0x7F);
            alarm->am1 = (data[0] & ALRM_MASK);
            alarm->minutes = bcd_2_uchar(data[1] & 0x7F);
            alarm->am2 = (data[1] & ALRM_MASK);
            alarm->am3 = (data[2] & ALRM_MASK);
            alarm->am_pm = (data[2] & AM_PM);
            alarm->mode = (data[2] & MODE);

            if (alarm->mode)
            {
                alarm->hours = bcd_2_uchar((data[2] & 0x1F));
            }
            else
            {
                alarm->hours = bcd_2_uchar((data[2] & 0x3F));
            }

            if (data[3] & DY_DT)
            {
                alarm->dy_dt = 1;
                alarm->day = bcd_2_uchar(data[3] & 0x0F);
            }
            else
            {
                alarm->date = bcd_2_uchar(data[3] & 0x3F);
            }
            alarm->am4 = (data[3] & ALRM_MASK);
        }
    }
    else
    {
        data[0] = ALRM2_MINUTES;
        rtn_val = write(w_adrs, (const char *)data, 1);

        if (!rtn_val)
        {
            rtn_val = read(r_adrs, (char *)data, 4);

            alarm->minutes = bcd_2_uchar(data[0] & 0x7F);
            alarm->am2 = (data[0] & ALRM_MASK);
            alarm->am3 = (data[1] & ALRM_MASK);
            alarm->am_pm = (data[1] & AM_PM);
            alarm->mode = (data[1] & MODE);

            if (alarm->mode)
            {
                alarm->hours = bcd_2_uchar((data[2] & 0x1F));
            }
            else
            {
                alarm->hours = bcd_2_uchar((data[2] & 0x3F));
            }

            if (data[2] & DY_DT)
            {
                alarm->dy_dt = 1;
                alarm->day = bcd_2_uchar(data[2] & 0x0F);
            }
            else
            {
                alarm->date = bcd_2_uchar(data[2] & 0x3F);
            }
            alarm->am4 = (data[2] & ALRM_MASK);
        }
    }

    return (rtn_val);
}

/**********************************************************/ /**
* Get control and status registers of DS3231
*
* On Entry:
*     @param[in] data - pointer to struct for storing control 
*                       and status register data
*
* On Exit:
*     @param[out] data - contains control and status registers
*                        data
*     @return return value = 0 on success, non-0 on failure
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15);  
* 
* //do not use 0xAA, see datasheet for appropriate data 
* ds3231_cntl_stat_t data = {0xAA, 0xAA}; 
*
* rtn_val = rtc.get_cntl_stat_reg(&data);
*
* @endcode
**************************************************************/
uint16_t Ds3231::get_cntl_stat_reg(ds3231_cntl_stat_t *data)
{
    uint16_t rtn_val = 1;
    uint8_t local_data[2];
    local_data[0] = CONTROL;
    rtn_val = write(w_adrs, (const char *)local_data, 1);
    if (!rtn_val)
    {
        rtn_val = read(r_adrs, (char *)local_data, 2);

        data->control = local_data[0];
        data->status = local_data[1];
    }

    return (rtn_val);
}

/**********************************************************/ /**
* Get temperature data of DS3231
*
* On Entry:
*
* On Exit:
*     @return return value = raw temperature data
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* uint16_t temp; 
*
* temp = rtc.get_temperature();
*
* @endcode
**************************************************************/
uint16_t Ds3231::get_temperature(void)
{
    uint16_t rtn_val = 1;
    uint8_t data[2];

    data[0] = MSB_TEMP;
    rtn_val = write(w_adrs, (const char *)data, 1);

    if (!rtn_val)
    {
        read(r_adrs, (char *)data, 2);

        rtn_val = data[0] << 8;
        rtn_val |= data[1];
    }

    return (rtn_val);
}

/**********************************************************/ /**
* Get epoch time based on current RTC time and date.  
* DS3231 must be configured and running before this fx is 
* called
*
* On Entry:
*
* On Exit:
*     @return return value = epoch time
*
* Example:
* @code
* 
* //instantiate rtc object
* Ds3231 rtc(D14, D15); 
* 
* time_t epoch_time; 
*
* epoch_time = rtc.get_epoch();
*
* @endcode
**************************************************************/
time_t Ds3231::get_epoch(void)
{
    //system vars
    struct tm sys_time;

    //RTC vars
    ds3231_time_t rtc_time = {0, 0, 0, 0, 0};
    ds3231_calendar_t rtc_calendar = {0, 0, 0, 0};

    get_calendar(&rtc_calendar);
    get_time(&rtc_time);

    sys_time.tm_wday = rtc_calendar.day - 1;
    sys_time.tm_mday = rtc_calendar.date;
    sys_time.tm_mon = rtc_calendar.month - 1;
    sys_time.tm_year = rtc_calendar.year + 100;

    //check for 12hr or 24hr mode
    if (rtc_time.mode)
    {
        //check am/pm
        if (rtc_time.am_pm && (rtc_time.hours != 12))
        {
            sys_time.tm_hour = rtc_time.hours + 12;
        }
        else
        {
            sys_time.tm_hour = rtc_time.hours;
        }
    }
    else
    {
        //24hr mode
        sys_time.tm_hour = rtc_time.hours;
    }

    sys_time.tm_min = rtc_time.minutes;
    sys_time.tm_sec = rtc_time.seconds;

    //make epoch time
    return (mktime(&sys_time));
}

/**********************************************************/ /**
* Private mmber fx, converts unsigned char to BCD
*
* On Entry:
*     @param[in] data - 0-255
*
* On Exit:
*     @return bcd_result = BCD representation of data
*
**************************************************************/
uint16_t Ds3231::uchar_2_bcd(uint8_t data)
{
    uint16_t bcd_result = 0;

    //Get hundreds
    bcd_result |= ((data / 100) << 8);
    data = (data - (data / 100) * 100);

    //Get tens
    bcd_result |= ((data / 10) << 4);
    data = (data - (data / 10) * 10);

    //Get ones
    bcd_result |= data;

    return (bcd_result);
}

/**********************************************************/ /**
* Private mmber fx, converts BCD to a uint8_t
*
* On Entry:
*     @param[in] bcd - 0-99
*
* On Exit:
*     @return rtn_val = integer rep. of BCD
*
**************************************************************/
uint8_t Ds3231::bcd_2_uchar(uint8_t bcd)
{
    uint8_t rtn_val = 0;

    rtn_val += ((bcd & 0xf0) >> 4) * 10;
    rtn_val += (bcd & 0x000f);

    return rtn_val;
}

/**
* Ham kiem co ngat alarm
*/
uint8_t Ds3231::alarm_check_match(void)
{
    uint16_t res = 0, alarm_match = 0;
    ds3231_cntl_stat_t status_alarm;
    res = get_cntl_stat_reg(&status_alarm);
    if (!res)
    {
        if ((status_alarm.status & 0x01) != 0)
        {
            alarm_match = 1;
        }
        else if ((status_alarm.status & 0x02) != 0)
        {
            alarm_match = 2;
        }
        if (alarm_match)
        {
            status_alarm.status = 0;
            set_cntl_stat_reg(status_alarm);
        }
    }
    return (alarm_match);
}

/**
* Ham cai dat thoi gian
*/
uint16_t Ds3231::set_all_time(uint32_t wday, uint32_t hour, uint32_t minute, uint32_t seconds, uint32_t mday, uint32_t month, uint32_t year)
{
    uint16_t res_1 = 0, res_2 = 0;
    ds3231_time_t rtc_time;
    ds3231_calendar_t rtc_calendar;
    rtc_time.hours = hour;
    rtc_time.minutes = minute;
    rtc_time.seconds = seconds;
    rtc_time.mode = 0; // format 24 hour
    rtc_calendar.date = mday;
    rtc_calendar.day = wday;
    rtc_calendar.month = month;
    if (year >= 300)
    {
        rtc_calendar.year = year - 300;
    }
    else if (year >= 200)
    {
        rtc_calendar.year = year - 200;
    }
    else if (year >= 100)
    {
        rtc_calendar.year = year - 100;
    }
    else
    {
        rtc_calendar.year = year;
    }
    
    debug(SETTIME_DEBUG, "\r\nSet date time: %d %d:%d:%d %d/%d/%d", rtc_calendar.day, rtc_time.hours, rtc_time.minutes, rtc_time.seconds, rtc_calendar.date, rtc_calendar.month, rtc_calendar.year);
    res_1 = set_time_ds(rtc_time);
    res_2 = set_calendar(rtc_calendar);
    debug(SETTIME_DEBUG,"\r\nRes time:%d\r\nRes date:%d", res_1, res_2);
    return (res_1 | res_2);
}