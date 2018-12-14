// this is basically a test to see if
//  the bot can act as a master.
//  it sends commands every serialInterval milliseconds.
//  the joystick sends a result based on the request

#include <Throb.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <variables.h>
#include <Encoder.h>

#define LED_PIN    13
Throb throb(LED_PIN);

#define EN_A 4
#define IN1  5
#define IN2  6
#define IN3  7
#define IN4  8
#define EN_B 9

#define CHA_1 23
#define CHB_1 22
#define CHA_2 21
#define CHB_2 20
#define BATT_IN 17

#define P_IN 16
#define I_IN 15
#define D_IN 14
#define LED_PIN 13

uint16_t P_val;
uint16_t I_val;
uint16_t D_val;
uint16_t B_val;

int pwmOutput1;
int pwmOutput2;

uint16_t position1 = 0;
uint16_t position2 = 0;
uint16_t delta1 = 0;
uint16_t delta2 = 0;
uint16_t target1 = 1600;
uint16_t target2 = 1600;

Encoder Enc1(CHA_1, CHB_1);
Encoder Enc2(CHA_2, CHB_2);
boolean blinkOn = false;

// AT+C004
// AT+B19200
// AT+U8N2
// AT+P5

void setup() {
  HC12.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD); 
  packetComplete = false;

  pinMode(LED_PIN, OUTPUT);
  pinMode(EN_A, OUTPUT); pinMode(EN_B, OUTPUT);
  pinMode(IN1, OUTPUT);  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);  pinMode(IN4, OUTPUT);
}

boolean receiveCMD() {
  if (HC12.available() > 0) {
    packetComplete = false;
    uint8_t rc = HC12.read();
    serialNow = millis();

    if (idx > numChars - 1) { idx = 0; }
    if (rc == CLOSE_DELIM) {
      serialBuf[idx] = CLOSE_DELIM;
      packetLen = idx + 1;
      idx = 0; 
      if (serialBuf[0] == OPEN_DELIM) {
	// you could break here, but
	// that would not clear any other spurious
	// characters coming in from HC12
	packetComplete = true;
      }
      collecting = false;
    }
    if (rc == OPEN_DELIM) {
      idx = 0;
      serialBuf[idx] == rc;
      collecting = true;
    }
    if (collecting) { // collect if you're between OPEN and CLOSE
      serialBuf[idx] = rc; 
      idx++;
    }
  }

  return(packetComplete);
}

void loop() {
  serialNow = millis();
  while((serialNow - serialDelta) > serialInterval) {
    HC12.write(REPORT); 
    serialDelta = serialNow;
    blinkOn = !blinkOn;
    digitalWrite(LED_PIN, blinkOn);

    serial2Now = millis();
  }

  P_val = 1024 - analogRead(P_IN);
  I_val = 1024 - analogRead(I_IN);
  D_val = 1024 - analogRead(D_IN); 

  B_val = analogRead(BATT_IN);

  target1 = P_val * 10;
  target2 = I_val * 10;

  position1 = Enc1.read();
  position2 = Enc2.read();

  if (receiveCMD()) {
    packetComplete = false;

    serial3Now = millis();
    if (serialBuf[CMD] == REPORT) {

      joy1PinVal = serialBuf[JOY1_HIGH] * 256 + serialBuf[JOY1_LOW];
      joy2PinVal = serialBuf[JOY2_HIGH] * 256 + serialBuf[JOY2_LOW]; 
      Serial.printf("(%d :: %d) %d%d%d%d\n",
		    joy1PinVal, joy2PinVal - 512,
		    serialBuf[BUTTON1], serialBuf[BUTTON2],
		    serialBuf[BUTTON3], serialBuf[BUTTON4]);

      pwmOutput1 = map(joy2PinVal, 0, 1024, 0, 120);

      if (serialBuf[BUTTON1] == HIGH || serialBuf[BUTTON2] == HIGH) {
	if (serialBuf[BUTTON1] == HIGH) {
	  digitalWrite(IN1, HIGH);
	  digitalWrite(IN2, LOW);
	  analogWrite(EN_A, pwmOutput1);
	}
	if (serialBuf[BUTTON2] == HIGH) {
	  digitalWrite(IN1, LOW);
	  digitalWrite(IN2, HIGH);
	  analogWrite(EN_A, pwmOutput1);
	}
      }
      else {
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, LOW);
      }

      if (serialBuf[BUTTON3] == HIGH || serialBuf[BUTTON4] == HIGH) {
	if (serialBuf[BUTTON3] == HIGH) {
	  digitalWrite(IN3, LOW);
	  digitalWrite(IN4, HIGH);
	  analogWrite(EN_B, pwmOutput1);
	}
	if (serialBuf[BUTTON4] == HIGH) {
	  digitalWrite(IN3, HIGH);
	  digitalWrite(IN4, LOW);
	  analogWrite(EN_B, pwmOutput1);
	}
      }
      else {
	digitalWrite(IN3, LOW);
	digitalWrite(IN4, LOW);
      }


    }
    else {
      for (int i = 0; i < packetLen; i++) {
	Serial.printf("%d ", serialBuf[i]);
      }

      Serial.printf(" :: %d\n", serial3Now - serial2Now);
    }
  }

}
