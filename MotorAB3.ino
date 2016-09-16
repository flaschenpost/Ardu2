#include <IRremote.h>
#include <IRremoteInt.h>
#include <Servo.h> 


const unsigned int RECV_PIN = 12; //define input pin on Arduino
const unsigned int pinLeftDigital =  8; 
const unsigned int pinRightDigital =  7; 
const unsigned int pinLeftAnalog  =  6;
const unsigned int pinRightAnalog  = 5;

const unsigned int pinServo = 10;

const signed int STEP = 0x20;
const unsigned int stepServo = 5; // Grad

const unsigned int straightServo = 90;
// bei 63 Grad dreht es um ein stehendes Hinterrad
const unsigned int minServo = 30;
const unsigned int maxServo = 150;

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

int oldDelta=0;
signed int oldDeltaServo = 0;
unsigned long oldKey = 0;

signed long speed = 0;
signed short int dir   = 1;


IRrecv irrecv(RECV_PIN);
decode_results results;

Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

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
  short signed int deltaServo = 0;
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
        posServo=straightServo;
        deltaServo = 0;
        break;
        
      case LEFT:
        keep=1;
        delta=0;
        deltaServo = stepServo;        
        break;

      case RIGHT:
        keep=1;
        delta=0;
        deltaServo = -stepServo;
        //Serial.print("RIGHT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
        break;

      case UP:
        delta = 1;
        oldDelta=delta;
        keep = 1;
        deltaServo=0;
        break;
      case DOWN:
        delta = -1;
        oldDelta=delta;
        keep = 1;
        deltaServo=0;
        break;
/*      case KEY6:
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
        */
      default:
        Serial.print("?");Serial.println(key,HEX);
       return;
    }
    if(key != KEEP){
      oldKey = key;
    }
    posServo += deltaServo;
    if(posServo < minServo){
      posServo = minServo;
    }
    if(posServo > maxServo){
      posServo = maxServo;
    }

    //Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.print(speed, HEX); Serial.print(" keep = ");Serial.print(keep);Serial.print(" delta=");Serial.print(delta);
    //Serial.println();
    if(delta != 0){
      oldDelta = delta;
    }
    if(deltaServo != 0){
      oldDeltaServo = deltaServo;
    }
    speed = (signed long)speed*(signed long)keep + (signed long)delta*(signed long)STEP*(signed long)dir;

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

    int speedLeft = speed;
    int speedRight= speed;
    if(posServo < straightServo - stepServo){
      unsigned long alpha = straightServo - posServo;
      Serial.print("posServo = ");Serial.print(posServo);Serial.print(", alpha=");Serial.println(alpha);
      if(alpha >= 50){
        speedRight = 0;
      }
      else{
        speedRight = (long)speed * (95-alpha) / (95 + alpha);
      }
    }
    if(posServo > straightServo + stepServo){
      unsigned long alpha = posServo - straightServo;
      Serial.print("posServo = ");Serial.print(posServo);Serial.print(", alpha=");Serial.println(alpha);
      if(alpha >= 50){
        speedLeft = 0; 
      }
      speedLeft = (long)speed * (95-alpha)/(95+alpha);
    }
    Serial.print("posServo=");Serial.print(posServo);Serial.print("speedLeft=");Serial.print(speedLeft, HEX);Serial.print("speedRight=");Serial.println(speedRight, HEX);
    if(posServo != oldServo && posServo > 0 && posServo < 180){
      //Serial.print("posServo = ");Serial.print(posServo);
      //Serial.print(" breakLeft=");Serial.print(breakLeft,HEX);
      //Serial.print(" breakRight=");Serial.println(breakRight,HEX);
      myservo.attach(pinServo);
      myservo.write(posServo);
      servoStarted = millis();
    }

    //Serial.print(" new speed = ");Serial.print(speed, HEX);
    //Serial.println();
    
    //Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.println(speed, HEX);
      
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

  if(servoStarted > 0 && millis()> 800 && millis() - servoStarted > 800){
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
   
   
