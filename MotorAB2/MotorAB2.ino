#include <IRremote.h>
#include <IRremoteInt.h>
#include <Servo.h> 


signed long speed = 0;
signed short int dir   = 1;

const unsigned int RECV_PIN = 12; //define input pin on Arduino
const unsigned int pinLeftDigital =  8; 
const unsigned int pinRightDigital =  7; 
const unsigned int pinLeftAnalog  =  6;
const unsigned int pinRightAnalog  = 5;

const unsigned int pinServo = 10;

const signed int STEP = 0x20;
const unsigned int breakStep = 0x20;

const unsigned int MIN  = 0x2F;
const unsigned int MAX  = 0xFF;

const unsigned long UP = 0xff629d;
const unsigned long DOWN = 0xffa857;
const unsigned long OK = 0xff02fd;
const unsigned long LEFT = 0xff22dd;
const unsigned long RIGHT = 0xffc23d;
const unsigned long KEEP = 0xffffffff;
const unsigned long KEY1 = 0xff6897;
const unsigned long KEY2 = 0xff9867;
const unsigned long KEY3 = 0xffb04f;
const unsigned long KEY4 = 0xff30cf;
const unsigned long KEY5 = 0xff18e7;
const unsigned long KEY6 = 0xff7a85;
const unsigned long KEY7 = 0xff10ef;
const unsigned long KEY9 = 0xff5aa5;



unsigned int breakRight = 0;
unsigned int breakLeft = 0;

signed int wheelExtra = 0;

int oldDelta=0;
unsigned long oldKey = 0;

IRrecv irrecv(RECV_PIN);
decode_results results;

Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

const unsigned int straightServo = 89;
int posServo = straightServo;    // variable to store the servo position 

unsigned long servoStarted = 5;
int oldServo = 0;
 
void setup() 
{ 
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(pinLeftDigital, OUTPUT);
  pinMode(pinLeftAnalog, OUTPUT);
  analogWrite(pinLeftDigital, 0);
  analogWrite(pinLeftAnalog, 0);
  pinMode (pinRightDigital, OUTPUT);
  pinMode (pinRightAnalog, OUTPUT);
  myservo.attach(pinServo);  // attaches the servo on pinServo to the servo object 
  servoStarted=millis()+1;
  myservo.write(posServo);
  // Serial.print("Setup Servo Started:");Serial.println(servoStarted);
}
  
void evalKey(const unsigned long key){
  short int delta = 0;
  short int keep = 0;
  oldServo=posServo;
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
        breakLeft=0;
        breakRight=0;
        posServo=straightServo;
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
        //Serial.print("LEFT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
        
        break;

      case RIGHT:
        keep=1;
        delta=0;
        if(breakLeft == 0){
          //Serial.print("breakRight = ");Serial.print(breakRight,HEX);Serial.print("breakStep = ");Serial.println(breakStep,HEX);
          if(breakRight < speed && breakRight < MAX-breakStep){
            breakRight += breakStep;
            //Serial.println("increased breakRight");
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
        //Serial.print("RIGHT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
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
      case KEY6:
        wheelExtra = -2;
        keep=1;
        break;
      case KEY4:
        wheelExtra = +2;
        keep=1;
        break;
      case KEY5:
        wheelExtra=0;
        keep=1;
        break;
      default:
        Serial.print("?");Serial.println(key,HEX);
       return;
    }
    if(key != KEEP){
      oldKey = key;
    }
    //Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.print(speed, HEX); Serial.print(" keep = ");Serial.print(keep);Serial.print(" delta=");Serial.print(delta);
    //Serial.println();
    if(delta != 0){
      oldDelta = delta;
    }
    float alpha=0;

    if(wheelExtra < -35){
      wheelExtra = -35;
    }
    if(wheelExtra > 35){
      wheelExtra = 35;
    }

    Serial.print("wheelExtra=");Serial.print(wheelExtra);
    if(breakRight > 0){
      alpha = -((float)breakRight*3 + (float) speed)*9.5 / (10+speed);
      if(breakRight==speed ){
        alpha = -35;
      }
    }
    if(breakLeft > 0 ){
      alpha = ((float)breakLeft*3 + (float) speed)*9.5 / (10+speed);
      if(breakLeft==speed){
        alpha=35;
      }
    }
    if(breakLeft==0 && breakRight==0){
      alpha = 0;
    }
    posServo = straightServo + alpha + wheelExtra;
    if(posServo>130){
      posServo = 130;
    }
    if(posServo < 50){
      posServo = 50;
    }

    if(posServo != oldServo && posServo > 0 && posServo < 180){
      //Serial.print("posServo = ");Serial.print(posServo);
      //Serial.print(" breakLeft=");Serial.print(breakLeft,HEX);
      //Serial.print(" breakRight=");Serial.println(breakRight,HEX);
      myservo.attach(pinServo);
      myservo.write(posServo);
      servoStarted = millis();
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
    //Serial.print("speed = ");Serial.println(speed);
    //Serial.print("speedLeft = ");Serial.print(speedLeft);Serial.print(" breakLeft=");Serial.println(breakLeft);
    //Serial.print("speedRight = ");Serial.print(speedRight);Serial.print(" breakRight=");Serial.println(breakRight);
      
    if(dir > 0){
      analogWrite(pinLeftAnalog, speedLeft);
      analogWrite(pinRightAnalog, speedRight);
      digitalWrite(pinLeftDigital, LOW);
      digitalWrite(pinRightDigital, LOW);
      //Serial.println("low");
    }
    else{
      analogWrite(pinLeftAnalog, 0xff-speedLeft);
      analogWrite(pinRightAnalog, 0xff-speedRight);
      digitalWrite(pinLeftDigital, HIGH);
      digitalWrite(pinRightDigital, HIGH);
      //Serial.println("high");
    }
}

void loop() {

  if(servoStarted > 0 && millis()> 1500 && millis() - servoStarted > 1500){
    myservo.detach();
    servoStarted = 0;
  }
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
   
   
