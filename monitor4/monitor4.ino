#include <Bounce2.h>

/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

//code by Dror Rapaport
//email:drorra@post.bgu.ac.il
#include <EEPROM.h>
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio


//Beginning of Auto generated function prototypes by Atmel Studio
void enterErrorMode();
void exitErrorMode();
//End of Auto generated function prototypes by Atmel Studio


bool isError=false; //1 if error mode, 0 if operational
int opState=3; //HIGH if inputs need to be HIGH, LOW otherwise, 3 in init
int opInput=4; //operation input pin

//the inputs we monitor
int monitorIn1=5;
int monitorIn2=6;
unsigned long OpSwitchTime;  //time since operation change (not including delay)
int relayPin=10; //Pin for relay (LOW if in error mode, HIGH else) (NC mode, negate if NO mode)
int resetPin=7; //Reset button
int greenLED=9; //green led pin
int redLED=8;  //red led pin
int initWaitTimer=0;
unsigned long initWaitThershold=1.5*1000; //wait 1.5 secs from opState change to voltage check
unsigned long millisHeld=0;    // How long the button was held (milliseconds)
int ErrorCodeAddress = 0; //address for storing error code in EEPROM
int resetTime=3*1000; //time to reset in milliseconds
unsigned long debounceButtonThreshold=10; //time to debounce for button, tweak if needed
unsigned long debounceInputThreshold=10; //time to debounce for inputs, tweak if needed

//debouncers
Bounce  opBouncer  = Bounce(); 
Bounce  resetBouncer  = Bounce();
Bounce  in1Bouncer  = Bounce();
Bounce  in2Bouncer  = Bounce();

unsigned long toggleTime; //timestamp when button press started
//Write error to EEPROM, turn off relay and sets LEDs
void enterErrorMode(){
  EEPROM.write(ErrorCodeAddress,1);
  isError=true;
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);
  digitalWrite(relayPin, LOW);
  Serial.write("error mode \n");
}


//Delete error from EEPROM, turn off relay and sets LEDs
void exitErrorMode(){
  EEPROM.write(ErrorCodeAddress,0);
  isError=false;
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(relayPin, HIGH);
  Serial.write("operation mode. \n");
}
void setup(){
  Serial.begin(9600);
  Serial.write("Hello guys! \n");
  //initialize pins
  pinMode(opInput,INPUT);
  pinMode(monitorIn1,INPUT);
  pinMode(monitorIn2,INPUT);
  pinMode(resetPin,INPUT);
  pinMode(redLED,OUTPUT);
  pinMode(greenLED,OUTPUT);
  //attaching debouncers
  opBouncer.attach(opInput);
  opBouncer.interval(debounceInputThreshold);
  in1Bouncer.attach(monitorIn1);
  in1Bouncer.interval(debounceInputThreshold);
  in2Bouncer.attach(monitorIn2);
  in2Bouncer.interval(debounceInputThreshold);
  resetBouncer.attach(resetPin);
  resetBouncer.interval(debounceButtonThreshold);

  //Find if device was in error mode before shutting down
  int initError=(int)EEPROM.read(ErrorCodeAddress);
  if(initError){
    Serial.write("recorded error, went to error mode \n");
    enterErrorMode();
  }
  else{
    Serial.write("no recorded error \n");
    exitErrorMode();
  }
}

void loop() {
  if(isError){
    resetBouncer.update();
    //Count pressing time
    int currentButtonState=resetBouncer.read();
    if (currentButtonState==HIGH){
      if(toggleTime==0){
        Serial.write("started pressing \n");
        toggleTime=millis();
      }
      millisHeld=millis()-toggleTime;
      if (millisHeld>=resetTime){
        Serial.write("button pressed, exit error mode \n");
        exitErrorMode();
        millisHeld=0;
        toggleTime=0;
      }
    }
    else{
      toggleTime=0;
      millisHeld=0;
      }
  }
  else{
    opBouncer.update();
    int currentOp=opBouncer.read();
    if(currentOp!=opState){
      Serial.write("operation mode changed to ");
      if(currentOp==HIGH){
        Serial.write("HIGH \n");
      }
      else{
        Serial.write("LOW \n");
      }
      opState=currentOp;
      delay(initWaitThershold);
      isError=false;
      return;
    }
    in1Bouncer.update();
    in2Bouncer.update();
    int in1state=in1Bouncer.read();
    int in2state=in2Bouncer.read();
    Serial.write("input 1 state: ");
    Serial.write(in1state);
    Serial.write(" input 2 state: ");
    Serial.write(in2state);
    Serial.write("\n");
    if (in1state!=opState){
      Serial.write("error in input pin 1 \n");
      enterErrorMode();
    }
    else if (in2state!=opState){
      Serial.write("error in input pin 2 \n");
      enterErrorMode();
    }
  }
  delay(1);
}
