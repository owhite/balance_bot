#define SERIAL_BAUD   57600
#define PACKET_LEN    6 
#define HC12 Serial1

// byte positions of packets that come in
// all returned packets terminate with '>'
#define CMD            1 // incoming command from master
#define RESPONSE       2 // response from bridge in returning packet
#define JOY1_HIGH      3
#define JOY1_LOW       4
#define JOY2_HIGH      5
#define JOY2_LOW       6
#define BUTTON1        7
#define BUTTON2        8
#define BUTTON3        9
#define BUTTON4        10
#define CLOSE          11

#define OPEN_DELIM  0x3C
#define CLOSE_DELIM 0x3E

// commands between master and slave
#define HELLO             0x31 // wake up 
#define ALREADY_CONNECTED 0x32 // check if awake
#define REPORT            0x33 // slave should send back info
#define ERROR             0x34 // report back 'never heard of last command'

uint16_t joy1PinVal; uint16_t joy2PinVal;

char printString[20];

// used for serial communcations
const uint8_t numChars = 24;
uint8_t  serialBuf[numChars]; 
uint8_t  packetLen;
uint32_t lastReceive   = 0;
boolean  collecting = false;
boolean  is_connected = false;
boolean  packetComplete = false;
uint8_t  order_received = 0;
uint8_t  idx = 0;

uint32_t serialDelta = 0;
uint32_t serialInterval = 60; 
uint32_t serialNow;

uint32_t serial2Now; // used for testing
uint32_t serial3Now;
