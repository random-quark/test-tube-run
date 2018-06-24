
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

bool marbleRunStarted = false;
long marbleRunStartTime = 0;
bool servo1AlreadyMoved = false;
bool servo2AlreadyMoved = false;

bool reverseLeds = false;

int lowestLdrValue = 1012;
int displayedLed = STARTING_LED;

int lastLedUpdate = 0;
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
//
//  Serial.println("start run from setup");
//  startRun();
}

void updateLed() {    
    if (millis() - lastLedUpdate < LED_UPDATE_TIME) return;
    if (!marbleRunStarted) return;

    if (displayedLed < stopAtLed && !reverseLeds) {
      displayedLed++;
    }
    if (displayedLed > stopAtLed && reverseLeds) {
      displayedLed--;
    }

    FastLED.clear();
    leds[displayedLed] = CRGB::Red;
    FastLED.show();

    lastLedUpdate = millis();
}

void startLedSequence(int startingLed, int ledsInSequence, bool _reverseLeds) {
  if (_reverseLeds) reverseLeds = true;
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
    if (atStage(13000) && !ledsFired[2]) {
      startLedSequence(30, 30, true);
      ledsFired[2] = true;
    }  
}

void fireServoSequences() {
    if (atStage(5000)  && !servo1AlreadyMoved && marbleRunStarted)
    {
      servo1AlreadyMoved = true;
      for (int i = 5; i <= 120; i++) {
        rightServo.write(i);
        delay(20);
      }
      delay(2000);
      for (int i = 120; i >= 5; i--) {
        rightServo.write(i);
        delay(20);
      }
    }
    
     if (atStage(13000) && !servo2AlreadyMoved && marbleRunStarted)
      { 
       servo2AlreadyMoved = true;
        for (int i = 180; i >= 60; i--) {
          leftServo.write(i);
          delay(20);
        }
      delay(2000);
      for (int i = 60; i<= 180; i++) {
        leftServo.write(i);
        delay(20);
      }
    }
    
    if (atStage(TOTAL_RUN_TIME))
    { 
      Serial.println("END RUN");
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
