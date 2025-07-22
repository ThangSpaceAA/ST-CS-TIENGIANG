#ifndef W25Q32_H
#define W25Q32_H

#include "mbed.h"

#define SPI_FREQ 1000000
#define SPI_MODE 0
#define SPI_NBIT 8

#define WRITE_ENABLE 0x06
#define WRITE_DISABLE 0x04
#define READ_DATA 0x03
#define WRITE_DATA 0x02
#define CHIP_ERASE 0x60
#define DEV_ID 0x90

#define DUMMY_ADDR 0x00
#define WAIT_TIME 1000

#define ADDR_BMASK2 0x00ff0000
#define ADDR_BMASK1 0x0000ff00
#define ADDR_BMASK0 0x000000ff

#define ADDR_BSHIFT2 16
#define ADDR_BSHIFT1 8
#define ADDR_BSHIFT0 0

class W25Q32
{
public:
    W25Q32(PinName mosi, PinName miso, PinName sclk, PinName cs);

    int readByte(int addr);                          // takes a 24-bit (3 bytes) address and returns the data (1 byte) at that location
    int readByte(int a2, int a1, int a0);            // takes the address in 3 separate bytes A[23,16], A[15,8], A[7,0]
    void readStream(int addr, char *buf, int count); // takes a 24-bit address, reads count bytes, and stores results in buf

    void writeByte(int addr, int data);               // takes a 24-bit (3 bytes) address and a byte of data to write at that location
    void writeByte(int a2, int a1, int a0, int data); // takes the address in 3 separate bytes A[23,16], A[15,8], A[7,0]
    void writeStream(int addr, char *buf, int count); // write count bytes of data from buf to memory, starting at addr

    void chipErase(); // erase all data on chip
    int getMID();     // get the manufacturer ID

private:
    void writeEnable();     // write enable
    void writeDisable();    // write disable
    void chipEnable();      // chip enable
    void chipDisable();     // chip disable
    int writeSPI(int data); // software SPI write

    // SPI inputs/outputs
    DigitalOut _cs;
    DigitalOut _mosi;
    DigitalIn _miso;
    DigitalOut _sclk;
};

#endif