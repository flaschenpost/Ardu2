#include <IRremote.h>
#include <IRremoteInt.h>

int RECV_PIN = 11; //define input pin on Arduino
IRrecv irrecv(RECV_PIN);
decode_results results;

signed long speedA = 0;
signed int dirA   = 1;

const unsigned int pinADigital = 9;
const unsigned int pinAAnalog = 10; 

const unsigned int STEP = 16;
const unsigned int MIN  = 0x5F;
const unsigned int MAX  = 0xFF;

const unsigned long UP = 0xff629d;
const unsigned long DOWN = 0xffa857;
const unsigned long OK = 0xff02fd;
const unsigned long KEEP = 0xffffffff;

int oldDelta=0;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(pinADigital, OUTPUT);
  pinMode(pinAAnalog, OUTPUT);
  analogWrite(pinADigital, 0);
  analogWrite(pinAAnalog, 0);
}

void loop() {
  int delta = 0;
  int keep = 1;
 
  if (irrecv.decode(&results)) {
    // ff629d = UP
    // ffa857 = down
    // ff22dd = left
    // ffc23d = right
    // ff02fd = OK    

    switch(results.value){
      case OK:
        delta = 0;
        //Serial.println("oldDelta=0");
        oldDelta=0;
        keep = 0;
        break;
        
      case UP:
        delta = 1;
        oldDelta=delta;
        keep = 1;
        break;
      
      case DOWN:
        delta = -1;
        oldDelta=delta;
        keep = 1;
        break;
      case KEEP:
        //Serial.print("KEEP");Serial.println(oldDelta, DEC);
        delta = oldDelta;
        break;
      default:
        Serial.println(results.value, HEX);
        delta = 0;
        keep = 1;
        
    }


    //Serial.print(" dirA = ");Serial.print(dirA, DEC);Serial.print(" speedA = ");Serial.print(speedA, HEX); Serial.print(" keep = ");Serial.print(keep);Serial.print(" delta=");Serial.print(delta);
    //Serial.println();
    if(delta != 0){
      oldDelta = delta;
    }

    speedA = (signed long)speedA*(signed long)keep + (signed long)delta*(signed long)STEP*(signed long)dirA;
    //Serial.print(" new speedA = ");Serial.print(speedA, HEX);
    //Serial.println();
    
    if(speedA > MAX){
      speedA = MAX;
    }
    if(speedA < MIN && delta != 0){
      dirA = delta;
      speedA = MIN;
    }
    if(speedA < 0){
      speedA = -speedA;
      dirA = -dirA;
    }
    if(0 < speedA && speedA < MIN){
      speedA = MIN;
    }
    //Serial.print(" dirA = ");Serial.print(dirA, DEC);Serial.print(" speedA = ");Serial.println(speedA, HEX);

    if(dirA > 0){
      analogWrite(pinAAnalog, speedA);
      digitalWrite(pinADigital, LOW);
    }
    else{
      analogWrite(pinAAnalog, 0xff-speedA);
      digitalWrite(pinADigital, HIGH);
    }
    
    irrecv.resume(); // Receive the next value
  }
}

