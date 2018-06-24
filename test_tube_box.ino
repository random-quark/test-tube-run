
/*
 * Controls LEDs and 2x servos based on LDR trigger
 * Author: Tom Chambers (tom.chambers@gmail.com)
 */

#include <Servo.h>
#include <FastLED.h>

Servo leftServo;
Servo rightServo;

#define LED_STRIP_PIN     6
#define LED_TYPE    WS2812B
#define NUM_LEDS    100
#define MARBLE_TIME_PER_LED 50
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
int ledsFired[3] = {false,false,false};

int TOTAL_RUN_TIME = 25000; // 15000

int LDR_PIN = A0;
int LEFT_SERVO_PIN = 10;
int RIGHT_SERVO_PIN = 9;
int LDR_THRESHOLD = 800;

int STARTING_LED = 4;
int LED_UPDATE_TIME = 150;
int LED_BRIGHTNESS = 30;
int LED_RANGE = 2; // do not put too high to avoid blowing leds

bool marbleRunStarted = false;
long marbleRunStartTime = 0;
bool servo1AlreadyMoved = false;
bool servo2AlreadyMoved = false;

bool reverseLeds = false;

int lowestLdrValue = 1012;
int displayedLed = STARTING_LED;

long lastLedUpdate = 0;
int stopAtLed = 0;

// right servo. zero = 5;

void setup() {
  Serial.begin(9600);
  leftServo.attach(LEFT_SERVO_PIN);
  rightServo.attach(RIGHT_SERVO_PIN);
  FastLED.addLeds<LED_TYPE, LED_STRIP_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.show();
  rightServo.write(5);
  leftServo.write(180);
  resetRun();

//  Serial.println("start run from setup");
//  startRun();
}

void setLedColors() {
  for (int i = displayedLed - LED_RANGE; i <= displayedLed + LED_RANGE; i++) {
    if (reverseLeds && i < stopAtLed) continue;
    if (!reverseLeds && i > stopAtLed) continue;
    leds[i] = CRGB::Red;
  }
}

void updateLed() {    
    if (millis() - lastLedUpdate < LED_UPDATE_TIME) return;
    if (!marbleRunStarted) return;

    Serial.println(millis() - lastLedUpdate);

    lastLedUpdate = millis();

    Serial.println("led change time");

    FastLED.clear();

    if (displayedLed >= stopAtLed && reverseLeds) {
      displayedLed--;
      setLedColors();
    }
    if (displayedLed <= stopAtLed && !reverseLeds) {
      displayedLed++;
      setLedColors();
    }

    FastLED.show();
}

void startLedSequence(int startingLed, int ledsInSequence, bool _reverseLeds) {
  if (_reverseLeds) {
    reverseLeds = true;
  } else {
    reverseLeds = false;
  }
  displayedLed = startingLed;
  if (reverseLeds) {
    stopAtLed = startingLed - ledsInSequence; 
  } else {
    stopAtLed = startingLed + ledsInSequence;  
  }
  displayedLed = startingLed;
}

void resetRun() {
  marbleRunStarted = false;
  marbleRunStartTime = 0;
  servo1AlreadyMoved = false;
  servo2AlreadyMoved = false;
  displayedLed = STARTING_LED;
  for (int i=0; i<3; i++) {
    ledsFired[i] = false;
  }
  FastLED.clear();
}

bool atStage(int time) {
  if (!marbleRunStarted) return false;
  if (millis() - marbleRunStartTime > time) return true;
  return false;
}

void checkLdr() {
  int ldrValue = analogRead(LDR_PIN);
  if (ldrValue < LDR_THRESHOLD)
  {
    startRun();
  }
}

void startRun() {
  Serial.println("Begin marble run");
  marbleRunStarted = true;
  marbleRunStartTime = millis();
}

void fireLedSequences() {
    if (atStage(0) && !ledsFired[0]) {
      startLedSequence(30, 30, false);
      ledsFired[0] = true;
    }
    if (atStage(5000) && !ledsFired[1]) {
      startLedSequence(60, 30, false);
      ledsFired[1] = true;
    }
    if (atStage(12200) && !ledsFired[2]) {
      startLedSequence(30, 30, true);
      ledsFired[2] = true;
    }  
}

void fireServoSequences() {
    if (atStage(5000)  && !servo1AlreadyMoved && marbleRunStarted)
    {
//      int servo1UpPos = 120;
      int servo1UpPos = 90;
      int servo1DownPos = 5;
      servo1AlreadyMoved = true;
      for (int i = servo1DownPos; i <= servo1UpPos; i++) {
        rightServo.write(i);
        delay(20);
      }
    }
    
     if (atStage(13000) && !servo2AlreadyMoved && marbleRunStarted)
      { 
//       int servo2UpPos = 60;
       int servo2DownPos = 180;

       int servo2UpPos = 80;
       servo2AlreadyMoved = true;
        for (int i = servo2DownPos; i >= servo2UpPos; i--) {
          leftServo.write(i);
          delay(20);
        }
    }
    
    if (atStage(TOTAL_RUN_TIME))
    { 
      Serial.println("END RUN");
      for (int i = 120; i >= 5; i--) {
        rightServo.write(i);
        delay(10);
      }
      for (int i = 60; i<= 180; i++) {
        leftServo.write(i);
        delay(10);
      }
      resetRun();
    }  
}

void loop() {
  if (marbleRunStarted) {
    updateLed();
    fireLedSequences();
    fireServoSequences();   
  } else {
    checkLdr();
  }
}
