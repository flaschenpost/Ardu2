#include <ADCTouch.h>

/* SevSeg Counter Example
 
 Copyright 2017 Dean Reading
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 
 This example demonstrates a very simple use of the SevSeg library with a 4
 digit display. It displays a counter that counts up, showing deci-seconds.
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
