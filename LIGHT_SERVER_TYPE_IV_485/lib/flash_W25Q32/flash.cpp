
#include "flash.h"
#include "debug.h"

static SPI spi(PA_7, PA_6, PA_5);
DigitalOut cspin(PA_4);

#define CSASSERT()  cspin = 0
#define CSRELEASE() cspin = 1

uint16_t SerialFlashChip::dirindex = 0;
uint8_t SerialFlashChip::flags = 0;
uint8_t SerialFlashChip::busy = 0;

#define FLAG_32BIT_ADDR     0x01    // larger than 16 MByte address
#define FLAG_STATUS_CMD70   0x02    // requires special busy flag check
#define FLAG_DIFF_SUSPEND   0x04    // uses 2 different suspend commands
#define FLAG_MULTI_DIE      0x08    // multiple die, don't read cross 32M barrier
#define FLAG_256K_BLOCKS    0x10    // has 256K erase blocks
#define FLAG_DIE_MASK       0xC0    // top 2 bits count during multi-die erase


void SerialFlashChip::wait(void)
{
    uint32_t status;
    //Serial.print("wait-");
    while (1) {
        CSASSERT();
        if (flags & FLAG_STATUS_CMD70) {
            // some Micron chips require this different
            // command to detect program and erase completion
            spi.write(0x70);
            status = spi.write(0);
            CSRELEASE();
            //Serial.printf("b=%02x.", status & 0xFF);
            if ((status & 0x80)) break;
        } else {
            // all others work by simply reading the status reg
            spi.write(0x05);
            status = spi.write(0);
            CSRELEASE();
            //Serial.printf("b=%02x.", status & 0xFF);
            if (!(status & 1)) break;
        }
    }
    busy = 0;
    //Serial.println();
}

void SerialFlashChip::read(uint32_t addr, void *buf, uint32_t len)
{
    uint8_t *p = (uint8_t *)buf;
    uint8_t b, f, status, cmd;

    memset(p, 0, len);
    f = flags;
    b = busy;
    if (b) {
        // read status register ... chip may no longer be busy
        CSASSERT();
        if (flags & FLAG_STATUS_CMD70) {
            spi.write(0x70);
            status = spi.write(0);
            if ((status & 0x80)) b = 0;
        } else {
            spi.write(0x05);
            status = spi.write(0);
            if (!(status & 1)) b = 0;
        }
        CSRELEASE();
        if (b == 0) {
            // chip is no longer busy :-)
            busy = 0;
        } else if (b < 3) {
            // TODO: this may not work on Spansion chips
            // which apparently have 2 different suspend
            // commands, for program vs erase
            CSASSERT();
            spi.write(0x06); // write enable (Micron req'd)
            CSRELEASE();
            wait_us(1);
            cmd = 0x75; //Suspend program/erase for almost all chips
            // but Spansion just has to be different for program suspend!
            if ((f & FLAG_DIFF_SUSPEND) && (b == 1)) cmd = 0x85;
            CSASSERT();
            spi.write(cmd); // Suspend command
            CSRELEASE();
            if (f & FLAG_STATUS_CMD70) {
                // Micron chips don't actually suspend until flags read
                CSASSERT();
                spi.write(0x70);
                do {
                    status = spi.write(0);
                } while (!(status & 0x80));
                CSRELEASE();
            } else {
                CSASSERT();
                spi.write(0x05);
                do {
                    status = spi.write(0);
                } while ((status & 0x01));
                CSRELEASE();
            }
        } else {
            // chip is busy with an operation that can not suspend
            wait();         // should we wait without ending
            b = 0;          // the transaction??
        }
    }
    do {
        uint32_t rdlen = len;
        if (f & FLAG_MULTI_DIE) {
            if ((addr & 0xFE000000) != ((addr + len - 1) & 0xFE000000)) {
                rdlen = 0x2000000 - (addr & 0x1FFFFFF);
            }
        }
        CSASSERT();
        // TODO: FIFO optimize....
        if (f & FLAG_32BIT_ADDR) {
            spi.write(0x03);
            spi.write(addr >> 24);
            spi.write(addr >> 16);
            spi.write(addr >> 8);
            spi.write(addr);
        } else {
            spi.write(3);
            spi.write(addr >> 16);
            spi.write(addr >> 8);
            spi.write(addr);
        }
        uint32_t i = rdlen;  // need block transfer
        while(i>0){
            *p++ = spi.write(0);
            i--;
        }
        CSRELEASE();
        addr += rdlen;
        len -= rdlen;
    } while (len > 0);
    if (b) {
        CSASSERT();
        spi.write(0x06); // write enable (Micron req'd)
        CSRELEASE();
        wait_us(1);
        cmd = 0x7A;
        if ((f & FLAG_DIFF_SUSPEND) && (b == 1)) cmd = 0x8A;
        CSASSERT();
        spi.write(cmd); // Resume program/erase
        CSRELEASE();
    }
}

void SerialFlashChip::write(uint32_t addr, const void *buf, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t max, pagelen;

     //Serial.printf("WR: addr %08X, len %d\n", addr, len);
    do {
        if (busy) wait();
        CSASSERT();
        // write enable command
        spi.write(0x06);
        CSRELEASE();
        max = 256 - (addr & 0xFF);
        pagelen = (len <= max) ? len : max;
         //Serial.printf("WR: addr %08X, pagelen %d\n", addr, pagelen);
        wait_us(1); // TODO: reduce this, but prefer safety first
        CSASSERT();
        if (flags & FLAG_32BIT_ADDR) {
            spi.write(0x02); // program page command
            spi.write(addr >> 24);
            spi.write(addr >> 16);
            spi.write(addr >> 8);
            spi.write(addr);
        } else {
            spi.write(2);
            spi.write(addr >> 16);
            spi.write(addr >> 8);
            spi.write(addr);
        }
        addr += pagelen;
        len -= pagelen;
        do {
            spi.write(*p++);
        } while (--pagelen > 0);
        CSRELEASE();
        busy = 4;
    } while (len > 0);
}

void SerialFlashChip::eraseAll()
{
    if (busy) wait();
    uint8_t id[5];
    readID(id);
    //Serial.printf("ID: %02X %02X %02X\n", id[0], id[1], id[2]);
    if (id[0] == 0x20 && id[2] >= 0x20 && id[2] <= 0x22) {
        // Micron's multi-die chips require special die erase commands
        //  N25Q512A    20 BA 20  2 dies  32 Mbyte/die   65 nm transitors
        //  N25Q00AA    20 BA 21  4 dies  32 Mbyte/die   65 nm transitors
        //  MT25QL02GC  20 BA 22  2 dies  128 Mbyte/die  45 nm transitors
        uint8_t die_count = 2;
        if (id[2] == 0x21) die_count = 4;
        uint8_t die_index = flags >> 6;
         //Serial.printf("Micron die erase %d\n", die_index);
        flags &= 0x3F;
        if (die_index >= die_count) return; // all dies erased :-)
        uint8_t die_size = 2;  // in 16 Mbyte units
        if (id[2] == 0x22) die_size = 8;
        CSASSERT();
        spi.write(0x06); // write enable command
        CSRELEASE();
         wait_us(1);
        CSASSERT();
        // die erase command
        spi.write(0xC4);
        spi.write((die_index * die_size) );
        spi.write(0);
        spi.write(0);
        spi.write(0);
        CSRELEASE();
         //Serial.printf("Micron erase begin\n");
        flags |= (die_index + 1) << 6;
    } else {
        // All other chips support the bulk erase command
        CSASSERT();
        // write enable command
        spi.write(0x06);
        CSRELEASE();
         wait_us(1);
        CSASSERT();
        // bulk erase command
        spi.write(0xC7);
        CSRELEASE();
    }
    busy = 3;
}

void SerialFlashChip::eraseBlock(uint32_t addr)
{
    uint8_t f = flags;
    if (busy) wait();
    CSASSERT();
    spi.write(0x06); // write enable command
    CSRELEASE();
     wait_us(1);
    CSASSERT();
    if (f & FLAG_32BIT_ADDR) {
        spi.write(0xD8);
        spi.write(addr >> 24);
        spi.write(addr >> 16);
        spi.write(addr >> 8);
        spi.write(addr);
    } else {
        spi.write(0xD8);
        spi.write(addr >> 16);
        spi.write(addr >> 8);
        spi.write(addr);
    }
    CSRELEASE();
    busy = 2;
}


bool SerialFlashChip::ready()
{
    uint32_t status;
    if (!busy) return true;
    CSASSERT();
    if (flags & FLAG_STATUS_CMD70) {
        // some Micron chips require this different
        // command to detect program and erase completion
        spi.write(0x70);
        status = spi.write(0);
        CSRELEASE();
        //Serial.printf("ready=%02x\n", status & 0xFF);
        if ((status & 0x80) == 0) return false;
    } else {
        // all others work by simply reading the status reg
        spi.write(0x05);
        status = spi.write(0);
        CSRELEASE();
        //Serial.printf("ready=%02x\n", status & 0xFF);
        if ((status & 1)) return false;
    }
    busy = 0;
    if (flags & 0xC0) {
        // continue a multi-die erase
        eraseAll();
        return false;
    }
    return true;
}


#define ID0_WINBOND 0xEF
#define ID0_SPANSION    0x01
#define ID0_MICRON  0x20
#define ID0_MACRONIX    0xC2
#define ID0_SST     0xBF
#define ID0_ADESTO      0x1F

//#define FLAG_32BIT_ADDR   0x01    // larger than 16 MByte address
//#define FLAG_STATUS_CMD70 0x02    // requires special busy flag check
//#define FLAG_DIFF_SUSPEND 0x04    // uses 2 different suspend commands
//#define FLAG_256K_BLOCKS  0x10    // has 256K erase blocks

bool SerialFlashChip::begin(uint8_t pin)
{
    uint8_t id[5];
    uint8_t f;
    uint32_t size;

    spi.frequency(30000000);   // max
    CSRELEASE();
    readID(id);
    f = 0;
    size = capacity(id);
    if (size > 16777216) {
        // more than 16 Mbyte requires 32 bit addresses
        f |= FLAG_32BIT_ADDR;
        if (id[0] == ID0_SPANSION) {
            // spansion uses MSB of bank register
            CSASSERT();
            spi.write(0x17); // bank register write
            spi.write(0x80);
            CSRELEASE();
        } else {
            // micron & winbond & macronix use command
            CSASSERT();
            spi.write(0x06); // write enable
            CSRELEASE();
            wait_us(1);
            CSASSERT();
            spi.write(0xB7); // enter 4 byte addr mode
            CSRELEASE();
        }
        if (id[0] == ID0_MICRON) f |= FLAG_MULTI_DIE;
    }
    if (id[0] == ID0_SPANSION) {
        // Spansion has separate suspend commands
        f |= FLAG_DIFF_SUSPEND;
        if (!id[4]) {
            // Spansion chips with id[4] == 0 use 256K sectors
            f |= FLAG_256K_BLOCKS;
        }
    }
    if (id[0] == ID0_MICRON) {
        // Micron requires busy checks with a different command
        f |= FLAG_STATUS_CMD70; // TODO: all or just multi-die chips?
    }
    flags = f;
    readID(id);
    return true;
}

// chips tested: https://github.com/PaulStoffregen/SerialFlash/pull/12#issuecomment-169596992
//
void SerialFlashChip::sleep()
{
    if (busy) wait();
    CSASSERT();
    spi.write(0xB9); // Deep power down command
    CSRELEASE();
}

void SerialFlashChip::wakeup()
{
    CSASSERT();
    spi.write(0xAB); // Wake up from deep power down command
    CSRELEASE();
}

void SerialFlashChip::readID(uint8_t *buf)
{
    if (busy) wait();
    CSASSERT();
    spi.write(0x9F);
    buf[0] = spi.write(0); // manufacturer ID
    buf[1] = spi.write(0); // memory type
    buf[2] = spi.write(0); // capacity
    if (buf[0] == ID0_SPANSION) {
        buf[3] = spi.write(0); // ID-CFI
        buf[4] = spi.write(0); // sector size
    }
    CSRELEASE();
    debug(MAIN_DEBUG, "ID: %02X %02X %02X\n", buf[0], buf[1], buf[2]);
}

void SerialFlashChip::readSerialNumber(uint8_t *buf) //needs room for 8 bytes
{
    if (busy) wait();
    CSASSERT();
    spi.write(0x4B);         
    spi.write(0);  
    spi.write(0);
    spi.write(0);
    spi.write(0);
    for (int i=0; i<8; i++) {       
        buf[i] = spi.write(0);
    }
    CSRELEASE();
//  Serial.printf("Serial Number: %02X %02X %02X %02X %02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
}

uint32_t SerialFlashChip::capacity(const uint8_t *id)
{
    uint32_t n = 1048576; // unknown chips, default to 1 MByte

	if (id[0] == ID0_ADESTO && id[1] == 0x89) {
		n = 1048576*16; //16MB
	} else
	if (id[2] >= 16 && id[2] <= 31) {
		n = 1ul << id[2];
	} else
	if (id[2] >= 32 && id[2] <= 37) {
		n = 1ul << (id[2] - 6);
	} else
	if ((id[0]==0 && id[1]==0 && id[2]==0) || 
		(id[0]==255 && id[1]==255 && id[2]==255)) {
		n = 0;
	}
    //Serial.printf("capacity %lu\n", n);
    return n;
}

uint32_t SerialFlashChip::blockSize()
{
    // Spansion chips >= 512 mbit use 256K sectors
    if (flags & FLAG_256K_BLOCKS) return 262144;
    // everything else seems to have 64K sectors
    return 65536;
}

SerialFlashChip SerialFlash;