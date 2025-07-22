#include <mspSerial.h>



mspSerial::mspSerial(HardwareSerial* SerIn, uint32_t baud)
{
    _Serial = SerIn;
    _baud = baud;
}

void mspSerial::attachInterupt(callback_function_t callback) {

    _fuc_callback = callback;
    _Serial->begin(_baud);
}

//
void mspSerial::serialize8(uint8_t a)//Truyen 1 byte
{
    _Serial->write(a);
    checksum ^= a;
}

//
void mspSerial::serialize16(int16_t a)//Truyen 2 byte
{
    static uint8_t t;
    t = a;
    _Serial->write(t);
    checksum ^= t;
    t = (a >> 8) & 0xff;
    _Serial->write(t);
    checksum ^= t;
}

//
void mspSerial::serialize32(uint32_t a)//Truyen 4 byte
{
    static uint8_t t;
    t = a;
    _Serial->write(t);
    checksum ^= t;
    t = a >> 8;
    _Serial->write(t);
    checksum ^= t;
    t = a >> 16;
    _Serial->write(t);
    checksum ^= t;
    t = a >> 24;
    _Serial->write(t);
    checksum ^= t;
}

//
void mspSerial::headSerialResponse(uint8_t err, uint8_t s)//Ham dinh dang truyen chuan Multiwii
{
    serialize8('$');
    serialize8('M');
    serialize8(err ? '!' : '>');
    checksum = 0;               // start calculating a new checksum
    serialize8(s);
    serialize8(cmdMSP);
}

//
void mspSerial::headSerialReply(uint8_t s)//Ham bao truyen bao nhieu byte
{
    headSerialResponse(0, s);
}

//
void mspSerial::headSerialError(uint8_t s)//Ham bao loi truyen 
{
    headSerialResponse(1, s);
}

//
void mspSerial::tailSerialReply(void)//Truyen ma checksum
{
    serialize8(checksum);
}



//
uint8_t mspSerial::read8(void)//Read 1 byte
{
    return inBuf[indRX++] & 0xff;
}
//
uint16_t mspSerial::read16(void)//Read 2 byte
{
    uint16_t t = read8();
    t += (uint16_t)read8() << 8;
    return t;
}
// 
int16_t mspSerial::readint16(void)//
{
    int16_t temp = (inBuf[indRX++]);
    temp = temp + ((inBuf[indRX++]) << 8);
    return temp;
}
//
uint32_t mspSerial::read32(void)//Read 4 byte
{
    uint32_t t = read16();
    t += (uint32_t)read16() << 16;
    return t;
}

//
void  mspSerial::send_struct(uint8_t cmd, uint8_t* cb, uint8_t siz) {
    cmdMSP = cmd;
    headSerialReply(siz);
    while (siz--)
        serialize8(*cb++);
    tailSerialReply();
}

//
void mspSerial::send_byte(uint8_t cmd, uint8_t data) {
    cmdMSP = cmd;
    headSerialReply(1);
    serialize8(data);
    tailSerialReply();
}

//
void mspSerial::readstruct(uint8_t* pt, uint8_t size) {
    uint16_t i = 0;
    for (i = 0; i < size; i++) {
        *pt = inBuf[indRX++];
        pt++;
    }
}



void mspSerial::isr_event(void)
{
    uint8_t c = 0;
    while (_Serial->available()) {
        c = _Serial->read();
        if (c_state == IDLE) {
            c_state = (c == '$') ? HEADER_START : IDLE;
            if (c_state == IDLE) {
            }
        }
        else if (c_state == HEADER_START) {
            c_state = (c == 'M') ? HEADER_M : IDLE;
        }
        else if (c_state == HEADER_M) {
            c_state = (c == '>') ? HEADER_ARROW : IDLE;
        }
        else if (c_state == HEADER_ARROW) {
            if (c > 255) {
                c_state = IDLE;
            }
            else {
                dataSize = c;
                offset = 0;
                checksum = 0;
                indRX = 0;
                checksum ^= c;
                c_state = HEADER_SIZE;
            }
        }
        else if (c_state == HEADER_SIZE) {
            cmdMSP = c;
            checksum ^= c;
            c_state = HEADER_CMD;
        }
        else if (c_state == HEADER_CMD && offset < dataSize) {
            checksum ^= c;
            inBuf[offset++] = c;
        }
        else if (c_state == HEADER_CMD && offset >= dataSize) {
            if (checksum == c) {
                _fuc_callback();
            }
            c_state = IDLE;
        }
    }
}

uint8_t mspSerial::getMSP(void){
    return cmdMSP;
}