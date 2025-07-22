#ifndef MSPSERIAL_H
#define MSPSERIAL_H
#include <Arduino.h>
#include <HardwareSerial.h>

enum _serial_state
{
	IDLE,
	HEADER_START,
	HEADER_M,
	HEADER_ARROW,
	HEADER_SIZE,
	HEADER_CMD,
};

class mspSerial
{
private:
	_serial_state c_state;
	typedef std::function<void(void)> callback_function_t;

public:

	mspSerial(HardwareSerial* SerIn, uint32_t baud);
	void attachInterupt(callback_function_t callback);
	void send_struct(uint8_t cmd, uint8_t* cb, uint8_t siz);
	void readstruct(uint8_t* pt, uint8_t size);
	void send_byte(uint8_t cmd, uint8_t data);
	uint8_t read8(void);
	uint16_t read16(void);
	int16_t readint16(void);
	uint32_t read32(void);
	uint8_t getMSP(void);
	void isr_event(void);

private:
	HardwareSerial* _Serial;
	callback_function_t _fuc_callback;
	void serialize8(uint8_t a);
	void serialize16(int16_t a);
	void serialize32(uint32_t a);
	void headSerialResponse(uint8_t err, uint8_t s);
	void headSerialReply(uint8_t s);
	void headSerialError(uint8_t s);
	void tailSerialReply(void);
	uint8_t cmdMSP;
	uint8_t checksum;
	uint8_t	indRX;
	uint8_t	inBuf[255];
	uint8_t offset;
	uint8_t dataSize;
	uint32_t _baud;
};

#endif