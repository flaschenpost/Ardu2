#include <ADCTouch.h>

/*
   Using a photo diode with a simple 4-diigit 7-segment output
 */

#include "SevSeg.h"
SevSeg sevseg; //Instantiate a seven segment controller object

void setup() {
  byte numDigits = 4;
  byte digitPins[] = {5,4,3,2};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(30);
}

void loop() {
  static unsigned long timer = millis();
  static int deciSeconds = 0;
  
  if (millis() >= timer) {
    timer += 200; 
    //sevseg.setNumber(ADCTouch.read(A0,10), 4);
    sevseg.setNumber(analogRead(A0),4);
  }
  //delay(50);
  sevseg.refreshDisplay(); // Must run repeatedly

}

/// END ///
