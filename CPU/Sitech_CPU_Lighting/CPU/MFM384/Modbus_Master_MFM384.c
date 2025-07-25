#include "Modbus_Master_MFM384.h"


float Random(float n)
{
   return n*rand()/RAND_MAX;
}

float convert_array_uint16_to_float(uint16_t *data, int byte_low, int byte_high)
{
    int byte_low_384 = data[byte_low];
    int byte_high_384 = data[byte_high];
    byte_high_384 <<= 16;
    int value = byte_low_384 + byte_high_384;
    float value_float = *(float *)&value;
    return value_float;
}

