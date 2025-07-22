/* Copyright (c) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#ifndef DEBUG_H
#define DEBUG_H
#include <mbed.h>

#define USING_WATCHDOG_TIMER        1
#define MAIN_DEBUG                  1 //<----------print debug ham main 
#define DIM_DEBUG                   1
#define W25Q32_DEBUG                0 //<----------print debug ham main 
#define DEBUG_SCHEDULE
#define RTC_DEBUG                   1                        //<----------print debug luc cau hinh RTC
#define SETTIME_DEBUG               0                    //<----------print debug module  DS3231 set new time   >
// #define DEBUG_RTC
// #define DEBUG_SETTINGS_STORAGE            

extern Serial com4;

#define debug(condition,fmt, ...) \
                                do { \
                                      if(condition)\
                                        com4.printf(fmt, __VA_ARGS__);\
                                  } while (0)



#endif