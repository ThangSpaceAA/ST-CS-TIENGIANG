#include "W25Q32.h"

W25Q32::W25Q32(PinName mosi, PinName miso, PinName sclk, PinName cs) : _cs(cs), _mosi(mosi), _miso(miso), _sclk(sclk)
{
}

int W25Q32::getMID()
{
    int ID = 0;
    chipEnable();
    this->writeSPI(DEV_ID);
    this->writeSPI(DUMMY_ADDR);
    this->writeSPI(DUMMY_ADDR);
    this->writeSPI(DUMMY_ADDR);
    this->writeSPI(DUMMY_ADDR);
    ID = this->writeSPI(DUMMY_ADDR);
    chipDisable();
    return ID;
}

int W25Q32::readByte(int addr)
{
    chipEnable();
    this->writeSPI(READ_DATA);
    this->writeSPI((addr & ADDR_BMASK2) >> ADDR_BSHIFT2);
    this->writeSPI((addr & ADDR_BMASK1) >> ADDR_BSHIFT1);
    this->writeSPI((addr & ADDR_BMASK0) >> ADDR_BSHIFT0);
    int response = this->writeSPI(DUMMY_ADDR);
    chipDisable();
    return response;
}

int W25Q32::readByte(int a2, int a1, int a0)
{
    chipEnable();
    this->writeSPI(READ_DATA);
    this->writeSPI(a2);
    this->writeSPI(a1);
    this->writeSPI(a0);
    int response = this->writeSPI(DUMMY_ADDR);
    chipDisable();
    return response;
}
void W25Q32::readStream(int addr, char *buf, int count)
{
    if (count < 1)
    {
        return;
    }

    chipEnable();
    this->writeSPI(READ_DATA);
    this->writeSPI((addr & ADDR_BMASK2) >> ADDR_BSHIFT2);
    this->writeSPI((addr & ADDR_BMASK1) >> ADDR_BSHIFT1);
    this->writeSPI((addr & ADDR_BMASK0) >> ADDR_BSHIFT0);

    for (int i = 0; i < count; i++)
    {
        buf[i] = this->writeSPI(DUMMY_ADDR);
    }

    chipDisable();
}

void W25Q32::writeByte(int addr, int data)
{
    writeEnable();
    chipEnable();
    this->writeSPI(WRITE_DATA);
    this->writeSPI((addr & ADDR_BMASK2) >> ADDR_BSHIFT2);
    this->writeSPI((addr & ADDR_BMASK1) >> ADDR_BSHIFT1);
    this->writeSPI((addr & ADDR_BMASK0) >> ADDR_BSHIFT0);
    this->writeSPI(data);
    chipDisable();
    writeDisable();
    thread_sleep_for(WAIT_TIME);
}

void W25Q32::writeByte(int a2, int a1, int a0, int data)
{
    writeEnable();
    chipEnable();
    this->writeSPI(WRITE_DATA);
    this->writeSPI(a2);
    this->writeSPI(a1);
    this->writeSPI(a0);
    this->writeSPI(data);
    chipDisable();
    writeDisable();
    thread_sleep_for(WAIT_TIME);
}

void W25Q32::writeStream(int addr, char *buf, int count)
{
    if (count < 1)
    {
        return;
    }

    writeEnable();
    chipEnable();
    this->writeSPI(WRITE_DATA);
    this->writeSPI((addr & ADDR_BMASK2) >> ADDR_BSHIFT2);
    this->writeSPI((addr & ADDR_BMASK1) >> ADDR_BSHIFT1);
    this->writeSPI((addr & ADDR_BMASK0) >> ADDR_BSHIFT0);

    for (int i = 0; i < count; i++)
    {
        this->writeSPI(buf[i]);
    }

    chipDisable();
    writeDisable();
    thread_sleep_for(WAIT_TIME);
}

void W25Q32::chipErase()
{
    writeEnable();
    chipEnable();
    this->writeSPI(CHIP_ERASE);
    chipDisable();
    writeDisable();
    thread_sleep_for(WAIT_TIME);
}

void W25Q32::writeEnable()
{
    chipEnable();
    this->writeSPI(WRITE_ENABLE);
    chipDisable();
}

void W25Q32::writeDisable()
{
    chipEnable();
    this->writeSPI(WRITE_DISABLE);
    chipDisable();
}

void W25Q32::chipEnable()
{
    _cs = 0;
}

void W25Q32::chipDisable()
{
    _cs = 1;
}

// Sends and receives 1 byte of SPI data MSB endianness
int W25Q32::writeSPI(int data)
{
    int i;
    int aux = 0;
    int aux2 = 0;
    int read = 0;

    for (i = 0; i < 8; i++)
    {
        _sclk = 0;
        aux = data & 0x80;
        aux >>= 7;
        _mosi = aux;
        data <<= 1;

        // input
        _sclk = 1;

        read <<= 1;
        aux2 = _miso;
        read |= aux2;
    }

    _sclk = 0;
    return read;
}