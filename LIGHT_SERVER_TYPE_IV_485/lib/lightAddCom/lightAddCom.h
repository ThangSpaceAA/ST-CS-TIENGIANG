#ifndef LIGHT_ADD_COM
#define LIGHT_ADD_COM
#include "mbed.h"
#include "main.h"

typedef enum _serial_state_
{
  IDLE,
  HEADER_START,
  HEADER_M,
  HEADER_ARROW,
  HEADER_SIZE,
  HEADER_CMD,
}_serial_state_;




class lightAddCom
{
	protected:
	    _serial_state_  c_state;
	    Serial& _mainCom;
		Serial& _sideCom;
		Serial& _side2Com;
		Callback<void(uint8_t)>    _function;
	public:	
	    lightAddCom(Serial& mainCom, Serial& sideCom, Serial& side2Com, uint32_t baud);
		void 	attach(Callback<void(uint8_t)> func);
		void  	send_struct(uint8_t cmd,uint8_t *cb, uint8_t siz);
		void 	readstruct(uint8_t *pt, uint8_t size);
		void 	send_byte(uint8_t cmd,uint8_t data);
		uint8_t  read8(void);
		uint16_t read16(void);
		int16_t  readint16(void);
		uint32_t read32(void);
		uint8_t  get_cmd(void);      
		void set_baud(uint32_t baud);
		void isr_enable(void);
		void isr_disenable(void);
	private:				
		void serialize8(uint8_t a);
	    void serialize16(int16_t a);
		void serialize32(uint32_t a);
		void headSerialResponse(uint8_t err, uint8_t s);
		void headSerialReply(uint8_t s);
		void headSerialError(uint8_t s);
		void tailSerialReply(void);
		void serializeNames(const char *s);
		void rxInterrupt (void);
		uint8_t checksum;
		uint8_t	indRX;
		uint8_t	inBuf[255];				
		uint8_t offset;
		uint8_t dataSize;
		uint8_t cmdMSP;
};
#endif
