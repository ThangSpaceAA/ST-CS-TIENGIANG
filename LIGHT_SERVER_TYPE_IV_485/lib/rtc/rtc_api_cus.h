#ifndef USER_RTC_H
#define USER_RTC_H
#include <mbed.h>
#include "rtc_ds3231.h"



typedef struct __attribute__((packed)){
    uint8_t     day;//0-6
    uint8_t     hour;//0-23
    uint8_t     minute;
    uint8_t     seconds;
    uint8_t     date;//1-31
    uint8_t     month;//1-12
    uint16_t    year;//20xx
}type_date_time_t;

//<----------------------------------------Bien lu kieu gio phut
typedef struct __attribute__((packed)){
    uint8_t hour;
    uint8_t minute;
}type_hm_time_t;


class rtc:public Ds3231
{
    public:

        /**
         * @brief Construct a new rtc object
         * 
         * @param sda_pin 
         * @param scl_pin 
         * @param rst_ic_pin 
         */
        rtc(PinName sda_pin, PinName scl_pin, PinName rst_ic_pin);

        /**
         * @brief Cai dat module rtc
         * 
         * @return int8_t: -1 failed
         */
        int8_t rtc_ic_init(void);

        /**
         * @brief load thoi gian thuc tu ic ngoai vao rtc noi stm32
         * 
         */
        void rtc_load_ex_rtc_to_internal(void);

        /**
         * @brief Ham cai dat thoi gian
         * 
         * @param settime_data cau truc thoi gian
         * @return uint8_t #0 error
         */
        uint8_t rtc_set_time(type_date_time_t settime_data);

        /**
         * @brief Ham doc thoi gian thuc
         * 
         * @return type_date_time_t thoi gian thuc
         */
        type_date_time_t rtc_get_time(void);

        /**
         * @brief Ham doc rtc unix time
         * 
         * @return time_t 
         */
        time_t epoch(void);



    private:
        DigitalOut          rst_pin;
    protected:

};
#endif