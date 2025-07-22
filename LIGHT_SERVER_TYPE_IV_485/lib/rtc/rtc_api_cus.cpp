#include "rtc_api_cus.h"
#include "debug.h"
#include <rtc_api.h>
#include "rtc_api_hal.h"

rtc::rtc(PinName sda_pin, PinName scl_pin, PinName rst_ic_pin)
    : Ds3231(sda_pin, scl_pin), rst_pin(rst_ic_pin)
{
    frequency(100000);
}

int8_t rtc::rtc_ic_init(void)
{
    rtc_init();
    // if (RCC->BDCR != 0x8103)
    // {
    //     debug(RTC_DEBUG, "\r\n%s", "The rtc internal init failed");
    //     return -1;
    // }
    debug(RTC_DEBUG, "\r\n%s", "Reset extern rtc");
    rst_pin = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
        wait(0.1);
        debug(RTC_DEBUG, "%s", ".");
    }
    rst_pin = 1;
    wait(0.2);
    rtc_load_ex_rtc_to_internal();
    return 0;
}

void rtc::rtc_load_ex_rtc_to_internal(void)
{
    time_t epoch_time = Ds3231::get_epoch();//unix_time code
    set_time(epoch_time);
    debug(RTC_DEBUG, "\r\nNew time loaded: %s", ctime(&epoch_time));
}

uint8_t rtc::rtc_set_time(type_date_time_t settime_data)
{
    uint16_t result = 0;
    //Part time to struct
    struct tm t;
    t.tm_hour = settime_data.hour;
    t.tm_min = settime_data.minute;
    t.tm_sec = settime_data.seconds;
    t.tm_mday = settime_data.date;
    t.tm_mon = (settime_data.month - 1);
    t.tm_year = (settime_data.year - 1900);
    //Luu vao STM32
    time_t time_make = mktime(&t);
    set_time(time_make);
    time_make = time(NULL);
    struct tm *time = gmtime(&time_make);
    //Luu vao DS323X
    result = set_all_time(
        (time->tm_wday + 1),
        time->tm_hour,
        time->tm_min,
        time->tm_sec,
        time->tm_mday,
        (time->tm_mon + 1),
        time->tm_year);
    if (result)
        debug(RTC_DEBUG, "\r\n%s", "Set date time to ex_rtc error!");
    debug(RTC_DEBUG, "\r\n%s", "Set new time successfull!");
    return (uint8_t)(result);
}

type_date_time_t rtc::rtc_get_time(void)
{
    ds3231_time_t time1;
    type_date_time_t t;
    time_t time_make = time(NULL);
    struct tm *time = gmtime(&time_make);
    get_time(&time1);
    // debug(MAIN_DEBUG, "\r\n%s:%d:%d:%d", "TimeNow", time1.hours, time1.minutes,time1.seconds);
    t.day = time->tm_wday;
    t.hour = time1.hours;
    t.minute = time1.minutes;
    t.seconds = time1.seconds;
    t.date = time->tm_mday;
    t.month = (time->tm_mon + 1);
    t.year = (time->tm_year + 1900);
    return t;
}
time_t rtc::epoch(void)
{
    return time(NULL);
}

