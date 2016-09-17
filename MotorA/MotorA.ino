#include <IRremote.h>
#include <IRremoteInt.h>

int RECV_PIN = 11; //define input pin on Arduino
IRrecv irrecv(RECV_PIN);
decode_results results;

signed long speed = 0;
signed short int dir   = 1;

const unsigned int pinADigital = 7; 
const unsigned int pinBDigital = 8; 
const unsigned int pinAAnalog = 9;
const unsigned int pinBAnalog = 10; 

const signed int STEP = 0x10;
const unsigned int breakStep = 0x10;

const unsigned int MIN  = 0x4F;
const unsigned int MAX  = 0xFF;

const unsigned long UP = 0xff629d;
const unsigned long DOWN = 0xffa857;
const unsigned long OK = 0xff02fd;
const unsigned long LEFT = 0xff22dd;
const unsigned long RIGHT = 0xffc23d;
const unsigned long KEEP = 0xffffffff;

unsigned int breakRight = 0;
unsigned int breakLeft = 0;

int oldDelta=0;
unsigned long oldKey = 0;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(pinADigital, OUTPUT);
  pinMode(pinAAnalog, OUTPUT);
  analogWrite(pinADigital, 0);
  analogWrite(pinAAnalog, 0);
}

void evalKey(const unsigned long key){
  short int delta = 0;
  short int keep = 0;
    switch(key){
      
      case KEEP:
        //Serial.print("KEEP");Serial.println(oldDelta, DEC);
        evalKey(oldKey);
        return;
        break;
      case OK:
        delta = 0;
        //Serial.println("oldDelta=0");
        oldDelta=0;
        keep = 0;
        break;
        
      case LEFT:
        keep=1;
        delta=0;
        if(breakRight == 0){
          if(breakLeft < speed && breakLeft < MAX-breakStep){
            breakLeft += breakStep;
          }
        }
        else{
          if(breakRight > breakStep){
            breakRight -= breakStep;
          }
          else{
            breakRight = 0;
          }
        }
        Serial.print("LEFT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
        break;

      case RIGHT:
        keep=1;
        delta=0;
        if(breakLeft == 0){
          Serial.print("breakRight = ");Serial.print(breakRight,HEX);Serial.print("breakStep = ");Serial.println(breakStep,HEX);
          if(breakRight < speed && breakRight < MAX-breakStep){
            breakRight += breakStep;
            Serial.println("increased breakRight");
          }
        }
        else{
          if(breakLeft > breakStep){
            breakLeft -= breakStep;
          }
          else{
            breakLeft = 0;
          }
        }
        Serial.print("RIGHT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
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
      default:
       return;
    }
    if(key != KEEP){
      oldKey = key;
    }
    Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.print(speed, HEX); Serial.print(" keep = ");Serial.print(keep);Serial.print(" delta=");Serial.print(delta);
    Serial.println();
    if(delta != 0){
      oldDelta = delta;
    }

    speed = (signed long)speed*(signed long)keep + (signed long)delta*(signed long)STEP*(signed long)dir;
    //Serial.print(" new speed = ");Serial.print(speed, HEX);
    //Serial.println();
    
    if(speed > MAX){
      speed = MAX;
    }
    if(speed < MIN && delta != 0){
      dir = delta;
      speed = MIN;
    }
    if(speed < 0){
      speed = -speed;
      dir = -dir;
    }
    if(0 < speed && speed < MIN){
      speed = MIN;
    }
    //Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.println(speed, HEX);

    unsigned short speedLeft=speed;
    unsigned short speedRight=speed;
    
    if(breakLeft > 0){
      if(breakLeft > speedLeft){
        breakLeft = speedLeft;
      }
      if(speedLeft - breakLeft < MIN){
        speedLeft = 0;
      }
      else{
        speedLeft -= breakLeft;
      }
    }
    
    if(breakRight > 0){
      if(breakRight > speedLeft){
        breakRight = speedRight;
      }
      if(speedRight - breakRight < MIN){
        speedRight = 0;
      }
      else{
        speedRight -= breakRight;
      }
    }
    Serial.print("speed = ");Serial.println(speed);
    Serial.print("speedLeft = ");Serial.print(speedLeft);Serial.print(" breakLeft=");Serial.println(breakLeft);
    Serial.print("speedRight = ");Serial.print(speedRight);Serial.print(" breakRight=");Serial.println(breakRight);
      
    if(dir > 0){
      analogWrite(pinAAnalog, speedLeft);
      analogWrite(pinBAnalog, speedRight);
      digitalWrite(pinADigital, LOW);
      digitalWrite(pinBDigital, LOW);
    }
    else{
      analogWrite(pinAAnalog, 0xff-speedLeft);
      analogWrite(pinBAnalog, 0xff-speedRight);
      digitalWrite(pinADigital, HIGH);
      digitalWrite(pinBDigital, HIGH);
    }
}

void loop() {
 
  if (irrecv.decode(&results)) {
    // ff629d = UP
    // ffa857 = down
    // ff22dd = left
    // ffc23d = right
    // ff02fd = OK    
    evalKey(results.value);
    irrecv.resume(); // Receive the next value
  }
}

