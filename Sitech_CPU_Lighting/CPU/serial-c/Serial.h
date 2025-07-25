#ifndef SERIAL_H_
#define SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>     /* for close() */
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <wiringPi.h>
#include <wiringSerial.h>



typedef enum _serial_state
{
    IDLE,
    HEADER_START,
    HEADER_M,
    HEADER_ARROW,
    HEADER_SIZE,
    HEADER_CMD,
}_serial_state;

extern uint8_t checksum;
extern uint8_t indRX;
extern uint8_t inBuf[255];
extern uint8_t offset;
extern uint8_t dataSize;
extern uint8_t cmdMSP;


extern int fd;
extern _serial_state c_state;


void serialize8(uint8_t a);     //Truyen 1 byte
void serialize16(int16_t a);    //Truyen 2 byte
void serialize32(uint32_t a);   //Truyen 4 byte
void headSerialResponse(uint8_t err, uint8_t s);
void headSerialReply(uint8_t s);
void headSerialError(uint8_t s);
void tailSerialReply(void);
void serializeNames(const char *s);
uint8_t read8(void);
uint16_t read16(void);
int16_t readint16(void);
uint32_t read32(void);
void send_struct(uint8_t cmd, uint8_t *cb, uint8_t siz);
void send_byte(uint8_t cmd, uint8_t data);
void readstruct(uint8_t *pt, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif