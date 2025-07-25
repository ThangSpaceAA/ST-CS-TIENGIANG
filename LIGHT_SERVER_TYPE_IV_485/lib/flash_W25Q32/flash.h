
#ifndef SerialFlash_h_
#define SerialFlash_h_

#include "mbed.h"
class SerialFlashFile;

class SerialFlashChip
{
public:
    static bool begin(uint8_t pin = 6);
    static uint32_t capacity(const uint8_t *id);
    static uint32_t blockSize();
    static void sleep();
    static void wakeup();
    static void readID(uint8_t *buf);
    static void readSerialNumber(uint8_t *buf);
    static void read(uint32_t addr, void *buf, uint32_t len);
    static bool ready();
    static void wait();
    static void write(uint32_t addr, const void *buf, uint32_t len);
    static void eraseAll();
    static void eraseBlock(uint32_t addr);

    static SerialFlashFile open(const char *filename);
    static bool create(const char *filename, uint32_t length, uint32_t align = 0);
    static bool createErasable(const char *filename, uint32_t length) {
        return create(filename, length, blockSize());
    }
    static bool exists(const char *filename);
    static bool remove(const char *filename);
    static bool remove(SerialFlashFile &file);
    static void opendir() { dirindex = 0; }
    static bool readdir(char *filename, uint32_t strsize, uint32_t &filesize);
private:
    static uint16_t dirindex; // current position for readdir()
    static uint8_t flags;   // chip features
    static uint8_t busy;    // 0 = ready
                // 1 = suspendable program operation
                // 2 = suspendable erase operation
                // 3 = busy for realz!!

  //  SPI spi(D11,D12,D13); // mosi, miso, sclk
};

extern SerialFlashChip SerialFlash;


class SerialFlashFile
{
public:
    SerialFlashFile() : address(0) {
    }
    operator bool() {
        if (address > 0) return true;
        return false;
    }
    uint32_t read(void *buf, uint32_t rdlen) {
        if (offset + rdlen > length) {
            if (offset >= length) return 0;
            rdlen = length - offset;
        }
        SerialFlash.read(address + offset, buf, rdlen);
        offset += rdlen;
        return rdlen;
    }
    uint32_t write(const void *buf, uint32_t wrlen) {
        if (offset + wrlen > length) {
            if (offset >= length) return 0;
            wrlen = length - offset;
        }
        SerialFlash.write(address + offset, buf, wrlen);
        offset += wrlen;
        return wrlen;
    }
    void seek(uint32_t n) {
        offset = n;
    }
    uint32_t position() {
        return offset;
    }
    uint32_t size() {
        return length;
    }
    uint32_t available() {
        if (offset >= length) return 0;
        return length - offset;
    }
    void erase();
    void flush() {
    }
    void close() {
    }
    uint32_t getFlashAddress() {
        return address;
    }
protected:
    friend class SerialFlashChip;
    uint32_t address;  // where this file's data begins in the Flash, or zero
    uint32_t length;   // total length of the data in the Flash chip
    uint32_t offset; // current read/write offset in the file
    uint16_t dirindex;
};
#endif
