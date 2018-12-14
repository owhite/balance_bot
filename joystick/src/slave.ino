#include <Throb.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <variables.h>

#define LED_PIN    13

Adafruit_SSD1306 display(4);
Throb throb(LED_PIN);

uint8_t btnPin1 = 9;
uint8_t btnPin2 = 8;
uint8_t btnPin3 = 7;
uint8_t btnPin4 = 6;
uint8_t joy1Pin  = 17;
uint8_t joy2Pin  = 16;

byte hiByte;
byte loByte;

uint8_t btn1; uint8_t btn2; uint8_t btn3; uint8_t btn4; 

void setup(){
  Serial.begin(SERIAL_BAUD);
  HC12.begin(SERIAL_BAUD);

  pinMode(btnPin1, INPUT);
  pinMode(btnPin2, INPUT);
  pinMode(btnPin3, INPUT);
  pinMode(btnPin4, INPUT);
  pinMode(LED_PIN, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.setTextColor(WHITE);

  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW);

  serialBuf[1] = 0 + '0';
}

boolean packetReceived() {
  packetComplete = false;

  serialBuf[0] = ERROR;

  if (HC12.available() > 0) {
    uint8_t rc = HC12.read();
    if (rc == HELLO || rc == ALREADY_CONNECTED || rc == REPORT) {
      serialBuf[0] = rc;
      packetComplete = true;
    }
  }

  return(packetComplete);
}

void everyLoop() {
  joy1PinVal = analogRead(joy1Pin); joy2PinVal = analogRead(joy2Pin);
  btn1 = digitalRead(btnPin1);  btn2 = digitalRead(btnPin2);
  btn3 = digitalRead(btnPin3);  btn4 = digitalRead(btnPin4);

  display.clearDisplay();
  display.setTextSize(1); display.setCursor(0,5); display.print("BTN:");
  display.setCursor(50,5); display.print(btn1);
  display.setCursor(60,5); display.print(btn2);
  display.setCursor(70,5); display.print(btn3);
  display.setCursor(80,5); display.print(btn4);

  display.setCursor(0,20); display.print(joy1PinVal);
  display.setCursor(100,20); display.print(joy2PinVal);
  display.setCursor(0,30); display.print(printString);
  
  display.setCursor(60,50); 

  if ( throb.pulseOnTimer(lastReceive) ) {
    display.setTextSize(2); display.print('*');
  }
  else {
    display.setTextSize(2); display.print('.');
  }

  display.display();
  strcpy(printString, "...");

}

void loop(){
  everyLoop();

  if (packetReceived()) {
    lastReceive = millis();

    switch(serialBuf[0]) {
    case HELLO: 
      // dont proceed until master sends hello
      if(!is_connected) { 
	is_connected = true;
	serialBuf[0] = OPEN_DELIM;
	serialBuf[CMD] = HELLO;
	serialBuf[CMD + 1] = CLOSE_DELIM;
	packetLen = 3;
	strcpy(printString, "hello");
      }
      else {
	// no more hellos, tell master the slave is woke
	serialBuf[0] = OPEN_DELIM;
	serialBuf[CMD] = ALREADY_CONNECTED;
	serialBuf[CMD + 1] = CLOSE_DELIM;
	packetLen = 3;
	strcpy(printString, "connect1");
      }
      break;

    case ALREADY_CONNECTED: // we woke
      serialBuf[0] = OPEN_DELIM;
      serialBuf[CMD] = ALREADY_CONNECTED;
      serialBuf[CMD + 1] = CLOSE_DELIM;
      packetLen = 3;
      strcpy(printString, "connect2");
      break;

    case REPORT:
      strcpy(printString, "report");

      serialBuf[0] = OPEN_DELIM;
      serialBuf[CMD] = REPORT;
      serialBuf[JOY1_HIGH] = highByte(joy1PinVal);
      serialBuf[JOY1_LOW]  =  lowByte(joy1PinVal);
      serialBuf[JOY2_HIGH] = highByte(joy2PinVal);
      serialBuf[JOY2_LOW]  =  lowByte(joy2PinVal);

      serialBuf[BUTTON1] = btn1; serialBuf[BUTTON2] = btn2;
      serialBuf[BUTTON3] = btn3; serialBuf[BUTTON4] = btn4;
      serialBuf[CLOSE] = CLOSE_DELIM;
      packetLen = 12;
      break;
    default: // received an unknown command from master
      strcpy(printString, "error");
      serialBuf[0] = OPEN_DELIM;
      serialBuf[CMD] = ERROR; 
      serialBuf[CMD + 1] = CLOSE_DELIM;
      packetLen = 3;
      return;
    }

    //  avoid filling up the master's serial buffer
    //   by only writes back if a packet was received
    Serial.println(packetLen);
    HC12.write((uint8_t*) serialBuf, packetLen); 
  }
}

