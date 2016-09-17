#include <IRremote.h>
#include <IRremoteInt.h>
#include <Servo.h> 


#define LED_LEFT 0
#define LED_RIGHT 1
#define LED_RED 0
#define LED_GREEN 1

// Richtung, Farbe: Rechts rot/gruen, links rot/gruen
const unsigned int LEDs[2][2]={{A0,A1},{4,3}};

// Eingang Fernbedienung
#define IR_INPUT 12 

// Pins fuer die Motoren
#define PIN_LEFT_DIGITAL 8 
#define PIN_RIGHT_DIGITAL 7 
#define PIN_LEFT_ANALOG 6
#define PIN_RIGHT_ANALOG 5

// Pin für den Lenkmotor (Servo)
#define PIN_SERVO 10

// Schrittweite (step) schneller/langsamer
const signed int STEP = 0x20;
// Schrittweite Lenkung (Grad)
const unsigned int stepServo = 5; // Grad

// Servo gerade? Damit kann man notfalls kleine Ausgleiche machen 
const unsigned int straightServo = 90;

// bei 63 Grad dreht es um ein stehendes Hinterrad. 90-60 = 30
const unsigned int minServo = straightServo-60;
// 90+60 = 150
const unsigned int maxServo = straightServo+60;

const unsigned int SPEED_MIN  = 0x2F;
const unsigned int SPEED_MAX  = 0xFF;

// Verschiedene Modus (Arten) der LED
#define MODE_OFF 0
#define MODE_GREEN 1
#define MODE_RED 2
#define MODE_BLINK 3
#define MODE_BOTH 4

// die LED können in einem der Modus sein, und in welchem wird hier gespeichert.
unsigned int leftLedMode=MODE_OFF;
unsigned int rightLedMode=MODE_OFF;
// wann wurde das letzte mal umgeschaltet?
unsigned long lastBlinkSwitch=0;
unsigned long blinkMillis = 350;

// Fernbedienung Codes
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


// manche Tastendrücke liefern bei gedrückt halten nur 0xffffffff = KEEP, dann brauchen wir die alten Werte
int oldDeltaSpeed=0;
signed int oldDeltaServo = 0;
unsigned long oldKey = 0;

// Wie schnell ist das Auto?
signed long speed = 0;

// Welche Richtung (direction) hat es? 1 = vor, -1 = zurueck
signed short int dir   = 1;

// Fernbedienungssensor aktivieren
IRrecv irrecv(IR_INPUT);

// Fernbedienungsergebnis heisst jetzt "results"
decode_results results;

Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

int posServo = straightServo;    // variable to store the servo position 

unsigned long servoStarted = 5;
int oldServo = 0;

int currentBlink = LED_RED;

// Das hier wird einmal nach jedem Start oder Reset aufgerufen
void setup() 
{ 
  Serial.begin(9600); // nur fuer die Textausgabe am Computer
  irrecv.enableIRIn(); // Start the receiver
  // Ausgabemodus setzen
  pinMode(PIN_LEFT_DIGITAL, OUTPUT);
  pinMode(PIN_LEFT_ANALOG, OUTPUT);
  analogWrite(PIN_LEFT_DIGITAL, 0);
  analogWrite(PIN_LEFT_ANALOG, 0);
  pinMode(PIN_RIGHT_DIGITAL, OUTPUT);
  pinMode(PIN_RIGHT_ANALOG, OUTPUT);
  analogWrite(PIN_RIGHT_DIGITAL, 0);
  analogWrite(PIN_RIGHT_ANALOG, 0);


  // Alle LEDs
  for (int i=0;i<2;i++){
    for(int j=0;j<2;j++){
      pinMode(LEDs[i][j], OUTPUT);
      digitalWrite(LEDs[i][j], LOW);
    }
  }
  // Servo erst mal starten
  myservo.attach(PIN_SERVO);  // attaches the servo on PIN_SERVO to the servo object 
  // uns merken, seit wann er läuft
  servoStarted=millis()+1;
  // und auf "Mitte" setzen.
  myservo.write(posServo);
  // Serial.print("Setup Servo Started:");Serial.println(servoStarted);

  speed = 0;
  dir = 1;
  oldKey=0;
  leftLedMode=MODE_OFF;
  rightLedMode=MODE_OFF;
  posServo=straightServo;
  checkLEDs();
}
  
// Hilfsfunktion zum Auswerten der Fernbedienung
void evalKey(const unsigned long key){
  // Geschwindigkeit aendern?
  short int deltaSpeed = 0;
  // Richtung aendern?
  short signed int deltaServo = 0;
  oldServo=posServo;
    switch(key){
      
      case KEEP:
        //Serial.print("KEEP");Serial.println(oldDeltaSpeed, DEC);
        if(oldKey != 0){
          evalKey(oldKey);
        }
        return;
        break;
      case OK:
        setup();
        return;
        break;
        
      case LEFT:
        deltaSpeed=0;
        deltaServo = stepServo;        
        break;

      case RIGHT:
        deltaSpeed=0;
        deltaServo = -stepServo;
        //Serial.print("RIGHT:");Serial.print(breakLeft, HEX);Serial.print(breakRight, HEX);
        break;

      case UP:
        deltaSpeed = 1;
        oldDeltaSpeed=deltaSpeed;
        deltaServo=0;
        break;
      case DOWN:
        deltaSpeed = -1;
        oldDeltaSpeed=deltaSpeed;
        deltaServo=0;
        break;
      case KEY6:
        setLED(LED_LEFT, MODE_BLINK);
        setLED(LED_RIGHT, MODE_OFF);
        return;
        break;
      case KEY4:
        setLED(LED_LEFT, MODE_OFF);
        setLED(LED_RIGHT, MODE_BLINK);
        return;
        break;
      case KEY5:
        setLED(LED_LEFT, MODE_OFF);
        setLED(LED_RIGHT, MODE_OFF);
        return; 
        break;
      default:
        Serial.print("?");Serial.println(key,HEX);
        oldKey = 0;
       return;
    }
    if(key == UP || key == DOWN){
      oldKey = key;
    }

    // Lenkung: deltaServo auf posServo addieren
    posServo += deltaServo;
    // Grenzen beachten
    if(posServo < minServo){
      posServo = minServo;
    }
    if(posServo > maxServo){
      posServo = maxServo;
    }

    // das langsamer/schneller (Änderung der Geschwindigkeit) speichern
    if(deltaSpeed != 0){
      oldDeltaSpeed = deltaSpeed;
    }
    // Lenkungsänderung speichern
    if(deltaServo != 0){
      oldDeltaServo = deltaServo;
    }

    // Auf die Geschwindigkeit kommt Richtung * Änderung drauf
    // wenn Fahrt bereits rückwärts und Änderung auch nach rückwärts, dann wird das Fahrzeug schneller 
    speed = (signed long)speed + (signed long)deltaSpeed*(signed long)STEP*(signed long)dir;

    // Grenzen der Geschwindigkeit
    if(speed > SPEED_MAX){
      speed = SPEED_MAX;
    }
    // wenn durch bremsen oder Beschleunigen die null durchschritten wurde, ist das ein Richtungswechsel
    if(speed < 0){
      speed = 0;
      dir = -dir;
    }
    if(0 < speed && speed < SPEED_MIN && deltaSpeed != 0){
      dir = deltaSpeed;
      speed = SPEED_MIN;
    }

    if(0 < speed && speed < SPEED_MIN){
      speed = SPEED_MIN;
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
    if(posServo <= straightServo - stepServo){
      leftLedMode = MODE_BLINK;
      rightLedMode=MODE_GREEN;
      lastBlinkSwitch=millis();
      currentBlink = LED_RED;
    }
    else{
      if(posServo >= straightServo + stepServo){
        leftLedMode = MODE_GREEN;
        rightLedMode=MODE_BLINK;
        lastBlinkSwitch=millis();
        currentBlink = LED_RED;
      }
      else{
        leftLedMode = MODE_GREEN;
        rightLedMode = MODE_GREEN;
        lastBlinkSwitch=0;
      }
    }

    Serial.print("posServo=");Serial.print(posServo);Serial.print("speedLeft=");Serial.print(speedLeft, HEX);Serial.print("speedRight=");Serial.println(speedRight, HEX);
    if(posServo != oldServo && posServo > 0 && posServo < 180){
      //Serial.print("posServo = ");Serial.print(posServo);
      //Serial.print(" breakLeft=");Serial.print(breakLeft,HEX);
      //Serial.print(" breakRight=");Serial.println(breakRight,HEX);
      myservo.attach(PIN_SERVO);
      myservo.write(posServo);
      servoStarted = millis();
    }

    //Serial.print(" new speed = ");Serial.print(speed, HEX);
    //Serial.println();
    
    //Serial.print(" dir = ");Serial.print(dir, DEC);Serial.print(" speed = ");Serial.println(speed, HEX);
      
    if(dir > 0){
      analogWrite(PIN_LEFT_ANALOG, speedLeft);
      analogWrite(PIN_RIGHT_ANALOG, speedRight);
      digitalWrite(PIN_LEFT_DIGITAL, LOW);
      digitalWrite(PIN_RIGHT_DIGITAL, LOW);
      //Serial.println("low");
    }
    else{
      analogWrite(PIN_LEFT_ANALOG, 0xff-speedLeft);
      analogWrite(PIN_RIGHT_ANALOG, 0xff-speedRight);
      digitalWrite(PIN_LEFT_DIGITAL, HIGH);
      digitalWrite(PIN_RIGHT_DIGITAL, HIGH);
      //Serial.println("high");
    }
    if(speed == 0){
      leftLedMode=MODE_OFF;
      rightLedMode=MODE_OFF;
      lastBlinkSwitch = 0;
    }
    checkLEDs();
}

void checkLEDs(){
  setLED(LED_LEFT, leftLedMode);
  setLED(LED_RIGHT, rightLedMode);
}

void setLED(int side, int mode){
  Serial.print("setLED: ");Serial.print(side);Serial.print(" ");Serial.println(mode);
  Serial.print("LEDS=");Serial.println(LEDs[side][LED_GREEN]);
  switch(mode){
    case MODE_OFF:
      Serial.println(LEDs[side][LED_GREEN]);
      digitalWrite(LEDs[side][LED_GREEN], 1);
      digitalWrite(LEDs[side][LED_RED], 1);
      break;
    case MODE_GREEN:
      digitalWrite(LEDs[side][LED_GREEN], 1);
      digitalWrite(LEDs[side][LED_RED], 0);
      break;
    case MODE_RED:
      digitalWrite(LEDs[side][LED_GREEN], 0);
      digitalWrite(LEDs[side][LED_RED], 1);
      break;
    case MODE_BOTH:
      digitalWrite(LEDs[side][LED_GREEN], 1);
      digitalWrite(LEDs[side][LED_RED], 1);
      break;
    case MODE_BLINK:
      if(currentBlink == LED_RED){
        digitalWrite(LEDs[side][LED_GREEN], LOW);
        digitalWrite(LEDs[side][LED_RED], HIGH);
      }
      else{
        digitalWrite(LEDs[side][LED_GREEN], HIGH);
        digitalWrite(LEDs[side][LED_RED], LOW);
      }
      break;
  } 
}

void loop() {

  if(lastBlinkSwitch > 0 && millis() - lastBlinkSwitch > blinkMillis){
    lastBlinkSwitch = millis();
    if(currentBlink == LED_RED){
      currentBlink = LED_GREEN;
    }
    else{
      currentBlink = LED_RED;
    }
    checkLEDs();
  }
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
   
   
