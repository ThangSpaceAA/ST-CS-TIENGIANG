#ifndef MODBUS_MASTER_MFM384_H
#define MODBUS_MASTER_MFM384_H


#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/time.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "modbus/modbus.h"
#include <wiringPi.h>
#include <wiringSerial.h>


#define TEMPLATE_POWER "CPU-{\"CMD\": %d,\
                             \"V1\": %.02f,\
                             \"V2\": %.02f,\
                             \"V3\": %.02f,\
                             \"I1\": %.02f,\
                             \"I2\": %.02f,\
                             \"I3\": %.02f,\
                             \"P1\": %.02f,\
                             \"P2\": %.02f,\
                             \"P3\": %.02f,\
                             \"P_Total\": %.02f,\
                             \"PF1\": %.02f,\
                             \"PF2\": %.02f,\
                             \"PF3\": %.02f,\
                             \"F1\": %.02f,\
                             \"F2\": %.02f,\
                             \"F3\": %.02f,\
                             \"PKWH\": %0.2f }"
                                            



float convert_array_uint16_to_float(uint16_t *data, int byte_low, int byte_high);

typedef struct __attribute__((__packed__))
{
    float V1_MFM384;
    float V2_MFM384;
    float V3_MFM384;

    float I1_MFM384; 
    float I2_MFM384;
    float I3_MFM384;

    float P1_MFM384;
    float P2_MFM384;
    float P3_MFM384;
    float P_Total_MFM384;

    float PF2_MFM384;
    float PF1_MFM384;
    float PF3_MFM384;

    float F1_MFM384;
    float F2_MFM384;
    float F3_MFM384;
    float P_Consumtion_kWH;
} parameter_MFM384_t;

float Random(float n);
#endif