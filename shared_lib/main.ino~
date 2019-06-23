#include <i2c_t3.h>
#include "MLX90621.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include "RunningAverage.h"
#include "icon_data.h"
#include "infinity_data.h"

#define MEM_LEN 12

#define S_SHOW_ICON           0
#define S_SHOW_ANIMATION      1
#define S_BLOB_TRACK          2
#define S_PUPIL_TRACK         3
#define S_GESTURE_TRACK       4

#define LEFT  1
#define RIGHT 2
#define NONE  3

#define SERIAL_SPEED   115200
#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

uint8_t columnBuf[]    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int right;
int left;

uint8_t dataBuf[MEM_LEN]; 

uint8_t heat_map[16 * 4];

uint8_t LEDPin = 13;

int8_t animationCount = 0;

int8_t state = S_SHOW_ICON;
uint8_t maxState = 5;
boolean inIconZone = false;
uint8_t iconCount = 0;

boolean yBump = false;
boolean zBump = false;
boolean oldZBump = false;

elapsedMillis elapsedRequestTime;
uint16_t requestDelay = 1000;

elapsedMillis elapsedBumpTime;
uint16_t bumpDelay = 400;
boolean  bumpState = false;

elapsedMillis elapsedBtn1Time;
uint16_t btn1Delay = 200;
uint8_t btn1State = LOW;
boolean btn1Pressed = false;

elapsedMillis elapsedBtn2Time;
uint16_t btn2Delay = 200;
uint8_t btn2State = LOW;
boolean btn2Pressed = false;

uint8_t btnPin1 = 3;
uint8_t btnPin2 = 4;
uint8_t circleX = 0;
uint8_t circleY = 0;

boolean updateDisplay = true;

uint8_t iconSelectNum = 0;

elapsedMillis elapsedJoyTime;
boolean blinkOn = false;
uint32_t blinkDelta = 0;
uint32_t blinkInterval = 400; 
uint32_t blinkNow;
float blobCenterX;
float blobCenterY;
uint8_t blobMass;

// sampling parameters
int RAsamples = 6;        // number of samples in rolling buffer
RunningAverage RA_x(RAsamples);
RunningAverage RA_y(RAsamples);

String inputString = "";

MLX90621 sensor; // create an instance of the Sensor class

void setup()   {                
  inputString.reserve(200);

  Serial.begin(SERIAL_SPEED);

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }

  Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  display.display();

  pinMode(btnPin1, INPUT); // set pin mode after display setting. 
  pinMode(btnPin2, INPUT);
  pinMode(LEDPin, OUTPUT);

  sensor.initialise (16); // start the thermo cam with 8 frames per second

  RA_x.clear(); RA_y.clear();
}

void showBlob() {
  for(int y=0;y<4;y++){ //go through all the rows
    for(int x=0;x<16;x++){ // first 8 columns
      double tempAtXY= sensor.getTemperature(y+x*4); 
      if (tempAtXY > 23) {
	display.fillRect(x * 8, y * 16, 8, 16, 1);
      }
    }
  }
}

void findBlobCenter(float& X, float& Y, uint8_t& mass) {

  mass = 0;
  uint8_t xMassSum[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t yMassSum[] = {0, 0, 0, 0};

  for(int y=0;y<4;y++){ //go through all the rows
    for(int x=0;x<16;x++){ // first 8 columns
      double tempAtXY= sensor.getTemperature(y+x*4); 
      if (tempAtXY > 23) { xMassSum[x]++; yMassSum[y]++; mass++;}
    }
  }

  uint16_t xSum = 0; 
  uint16_t xMomentSum = 0;
  for(uint8_t x=0;x<16;x++){ xMomentSum += xMassSum[x] * (x + 1); xSum += xMassSum[x]; }
  if (xSum != 0) { X = (float) xMomentSum / (float) xSum;}
  else X = -1;

  uint16_t ySum = 0; 
  uint16_t yMomentSum = 0;
  for(uint8_t y=0;y<4;y++){ yMomentSum += yMassSum[y] * (y + 1); ySum += yMassSum[y]; }
  if (ySum != 0) { Y = (float) yMomentSum / (float) ySum;}
  else Y = -1;
}


void loop() {
  Serial.println("here");

  everyLoop();

  switch (state) {
  case S_SHOW_ICON:

    if (updateDisplay) {
      display.clearDisplay();
      if (blinkOn) {
	display.drawBitmap(0, 0, icon_bmp[iconCount], ICON_WIDTH, ICON_HEIGHT, 1);
	display.display();
      }
      else {
	display.display();
      }
    }

    break;

  case S_SHOW_ANIMATION:

    if (animationCount > INFINITY_MAX) { animationCount = 0; }
    display.clearDisplay();
    display.drawBitmap(34, 9, infinity_gif[animationCount], INFINITY_WIDTH, INFINITY_HEIGHT, 1);
    animationCount++;
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(state);

    display.display();

    break;

  case S_BLOB_TRACK: 
    display.clearDisplay();

    sensor.measure(); //get new readings from the sensor

    showBlob();

    findBlobCenter(blobCenterX, blobCenterY, blobMass);

    if (blobMass != 0) {
      RA_x.addValue((blobCenterX - 1) * (float) 8);
      RA_y.addValue((blobCenterY - 1) * (float) 16);
      display.fillCircle(RA_x.getAverage(), RA_y.getAverage(), 10, BLACK);
      display.drawCircle(RA_x.getAverage(), RA_y.getAverage(), 10, WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(state);

    display.display();

    break;

  case S_PUPIL_TRACK: 
    display.clearDisplay();

    sensor.measure(); //get new readings from the sensor
    findBlobCenter(blobCenterX, blobCenterY, blobMass);

    if (blobMass != 0) {
      RA_x.addValue((blobCenterX - 1) * (float) 8);
      RA_y.addValue((blobCenterY - 1) * (float) 16);
      display.fillCircle(RA_x.getAverage(), RA_y.getAverage(), 10, BLACK);
      display.drawCircle(RA_x.getAverage(), RA_y.getAverage(), 10, WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(state);

    display.display();

    break;

  default:
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(state);
    display.setCursor(20, 0);
    display.println("??");

    display.display();
    break;

  }   
  updateDisplay = false;
}

// housekeeping things
void everyLoop() {

  // BUMP CHECK BLOCK
  lis.read();      // get X Y and Z data at once
  sensors_event_t event; 
  lis.getEvent(&event);
  
  yBump = false;
  zBump = false;

  if (abs(event.acceleration.y) > 15) {
    Serial.println("boinked");

    yBump = true;
  }

  if (abs(event.acceleration.z) > 15) {
    if (elapsedBumpTime < bumpDelay) {
      elapsedBumpTime = 0;
      updateDisplay = true;
      zBump = true;
      blinkOn = !blinkOn;
      Serial.println("double tap");
    }
    else {
      delay(40);
      elapsedBumpTime = 0;
    }
  }

  if (yBump) {
    Serial.println("side");
    if (iconCount >= ICON_MAX) {
      iconCount = 0;
    }
    else {
      iconCount++;
    }
    updateDisplay = true;
  }

  // BUTTON CHECK BLOCK
  btn1State = digitalRead(btnPin1);
  btn2State = digitalRead(btnPin2);
  
  if (btn1Pressed && elapsedBtn1Time > btn1Delay) {
    if (iconCount >= ICON_MAX) {
      iconCount = 0;
    }
    else {
      iconCount++;
    }

    display.clearDisplay();
    btn1State == LOW;
    btn1Pressed = false;
    updateDisplay = true;
  }
  if (btn1State == HIGH) { elapsedBtn1Time = 0; btn1Pressed = true;}

  if (btn2Pressed && elapsedBtn2Time > btn2Delay) {
    if (iconCount <= 0) {
      iconCount = ICON_MAX;
    }
    else {
      iconCount--;
    }
    display.clearDisplay();
    btn2State == LOW;
    btn2Pressed = false;
    updateDisplay = true;
  }
  if (btn2State == HIGH) { elapsedBtn2Time = 0; btn2Pressed = true;}

}


