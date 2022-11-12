
// SimpleRx - the slave or the receiver

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Ewma.h>

//LRF pin number
#define CE_PIN 49
#define CSN_PIN 53

//tx-rx encryption code thing
const byte thisSlaveAddress[5] = {'T','E','R','A','X'};

RF24 radio(CE_PIN, CSN_PIN);

Ewma RotaFilter(.1);  //time delay-ish rotation
Ewma LiftFilter(.2);  //time delay-ish lift

uint8_t buf[12];
// char buf[12]; // this must match dataToSend in the TX
bool newData = false;

// Define Us signal output pins
#define Us2EXT    5 
#define Us3EXT    6
#define UsDIGR    7
#define UsWNCH    8
#define UsROTA    9
#define UsLIFT    10

// set initial Us value on startup
int UsMapInit = 0;

static unsigned long lastRecvTime = 0; // set failsafe counter 

// set output pins
const int pin2EXT = 22;
const int pin3EXT = 23;
const int pinDIGR = 24;
const int pinWNCH = 25;
const int pinROTA = 26;
const int pinLIFT = 27;
const int pinHiIdle = 28;
const int pinStart = 29;
const int pinTiltUp = 30;
const int pinTiltDn = 31;
const int pinClawOp = 32;
const int pinClawCl = 33;
const int pinDigHiLo = 34;
const int pinDigRel = 35;
const int pinEstop = 36;
const int pinKill = 37;
// const int pinHorn = 36;

//===========

void setup() {

    Serial.begin(9600);

    Serial.println("SimpleRx Starting");
    
  // Set the controller pins to output
  // Output PWM freq setup
  pinMode(Us2EXT,OUTPUT); 
  pinMode(Us3EXT,OUTPUT);
  pinMode(UsDIGR,OUTPUT);
  pinMode(UsWNCH,OUTPUT);
  pinMode(UsROTA,OUTPUT);
  pinMode(UsLIFT,OUTPUT);
 
  // Output relay variables
  pinMode(pin2EXT,OUTPUT);
  pinMode(pin3EXT,OUTPUT);
  pinMode(pinDIGR,OUTPUT);
  pinMode(pinWNCH,OUTPUT);
  pinMode(pinROTA,OUTPUT);
  pinMode(pinLIFT,OUTPUT);
  pinMode(pinHiIdle,OUTPUT);
  pinMode(pinStart,OUTPUT);
  pinMode(pinTiltUp,OUTPUT);
  pinMode(pinTiltDn,OUTPUT);
  pinMode(pinClawOp,OUTPUT);
  pinMode(pinClawCl,OUTPUT);
  pinMode(pinDigHiLo,OUTPUT); 
  pinMode(pinDigRel,OUTPUT);  
  pinMode(pinEstop,OUTPUT);  
  pinMode(pinKill,OUTPUT);  
  
  // Output relays set off initial
  digitalWrite(pin2EXT,HIGH);
  digitalWrite(pin3EXT,HIGH);
  digitalWrite(pinDIGR,HIGH);
  digitalWrite(pinWNCH,HIGH);
  digitalWrite(pinROTA,HIGH);
  digitalWrite(pinLIFT,HIGH);
  digitalWrite(pinHiIdle,HIGH);
  digitalWrite(pinStart,HIGH);
  digitalWrite(pinTiltUp,HIGH);
  digitalWrite(pinTiltDn,HIGH);
  digitalWrite(pinClawOp,HIGH);
  digitalWrite(pinClawCl,HIGH);
  digitalWrite(pinDigHiLo,HIGH);
  digitalWrite(pinDigRel,HIGH);
  digitalWrite(pinEstop,HIGH);
  digitalWrite(pinKill,HIGH);
  
  analogWrite(Us2EXT, UsMapInit);
  analogWrite(Us3EXT, UsMapInit);
  analogWrite(UsDIGR, UsMapInit);
  analogWrite(UsWNCH, UsMapInit);
  analogWrite(UsROTA, UsMapInit);
  analogWrite(UsLIFT, UsMapInit);
    
    radio.begin();
    radio.setDataRate( RF24_2MBPS );
    radio.setChannel(28);
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
}

//=============

void loop() {
    getData();
    showData();
}

//==============

void getData() {
    if ( radio.available() ) {
        radio.read( &buf, sizeof(buf) );
        newData = true;
        lastRecvTime = millis();  // set failsafe counter to time last signal recieved
    }

    if (millis() - lastRecvTime > 1000) {  // initiate failsafe 
        digitalWrite(pin2EXT,HIGH);
        digitalWrite(pin3EXT,HIGH);
        digitalWrite(pinDIGR,HIGH);
        digitalWrite(pinWNCH,HIGH);
        digitalWrite(pinROTA,HIGH);
        digitalWrite(pinLIFT,HIGH);
        digitalWrite(pinHiIdle,HIGH);
        digitalWrite(pinStart,HIGH);
        digitalWrite(pinTiltUp,HIGH);
        digitalWrite(pinTiltDn,HIGH);
        digitalWrite(pinClawOp,HIGH);
        digitalWrite(pinClawCl,HIGH);
        digitalWrite(pinDigHiLo,HIGH);
        digitalWrite(pinDigRel,HIGH);
        digitalWrite(pinEstop,HIGH);
        digitalWrite(pinKill,HIGH);
  
        analogWrite(Us2EXT, UsMapInit);
        analogWrite(Us3EXT, UsMapInit);
        analogWrite(UsDIGR, UsMapInit);
        analogWrite(UsWNCH, UsMapInit);
        analogWrite(UsROTA, UsMapInit);
        analogWrite(UsLIFT, UsMapInit);
        
        Serial.println("Radio not available");
        delay(5000);  // wait to try RX again
    }
  
}

void showData() {
    if (newData == true) {
        Serial.println("Data received ");
        Serial.println(buf[0]);
        Serial.println(buf[1]);
        Serial.println(buf[2]);
        Serial.println(buf[3]);
        Serial.println(buf[4]);
        Serial.println(buf[5]);
        Serial.println(buf[6]);
        Serial.println(buf[7]);
        Serial.println(buf[8]);
        Serial.println(buf[9]);
        Serial.println(buf[10]);
        Serial.println(buf[11]);

        // == EWMA Filtering loop:
        float filteredROTA = RotaFilter.filter(buf[4]);
        float filteredLIFT = LiftFilter.filter(buf[5]);
        analogWrite(UsROTA, filteredROTA);
        analogWrite(UsLIFT, filteredLIFT);
      
        // output PWM Us signals
        analogWrite(Us2EXT, buf[0]);
        analogWrite(Us3EXT, buf[1]);
        analogWrite(UsDIGR, buf[2]);
        analogWrite(UsWNCH, buf[3]);
        //analogWrite(UsROTA, buf[4]);
        //analogWrite(UsLIFT, buf[5]);
        
      // output relay 
      // Low-level relay board initiates relays-ON at LOW
      if (buf[6] == 0) {     // Valves
      digitalWrite(pinEstop, LOW);
      digitalWrite(pin2EXT, LOW);
      digitalWrite(pin3EXT, LOW);
      digitalWrite(pinDIGR, LOW);
      digitalWrite(pinWNCH, LOW);
      digitalWrite(pinROTA, LOW);
      digitalWrite(pinLIFT, LOW);
      }
      else {                 // (buf[6] == 1)
      digitalWrite(pinEstop, HIGH);
      digitalWrite(pin2EXT, HIGH);
      digitalWrite(pin3EXT, HIGH);
      digitalWrite(pinDIGR, HIGH);
      digitalWrite(pinWNCH, HIGH);
      digitalWrite(pinROTA, HIGH);
      digitalWrite(pinLIFT, HIGH);
      }
      
      if (buf[7] == 2) {
      digitalWrite(pinTiltUp, LOW);
      digitalWrite(pinTiltDn, HIGH);
      }
      else if (buf[7] == 3) {
      digitalWrite(pinTiltUp, HIGH);
      digitalWrite(pinTiltDn, LOW);
      }
      else {                   // (buf[7] == 4) Both OFF
      digitalWrite(pinTiltUp, HIGH);
      digitalWrite(pinTiltDn, HIGH);
      }
      
      if (buf[8] == 2) {
      digitalWrite(pinClawOp, LOW);
      digitalWrite(pinClawCl, HIGH);
      }
      else if (buf[8] == 3) {
      digitalWrite(pinClawOp, HIGH);
      digitalWrite(pinClawCl, LOW);
      }
      else {                   // (buf[8] == 4) Both OFF
      digitalWrite(pinClawOp, HIGH);
      digitalWrite(pinClawCl, HIGH);
      }
      
      if (buf[9] == 2) {
      digitalWrite(pinDigHiLo, LOW);
      digitalWrite(pinDigRel, HIGH);
      }
      else if (buf[9] == 3) {
      digitalWrite(pinDigHiLo, HIGH);
      digitalWrite(pinDigRel, LOW);
      }
      else {                   // (buf[9] == 4) Both OFF
      digitalWrite(pinDigHiLo, HIGH);
      digitalWrite(pinDigRel, HIGH);
      }
      
      if (buf[10] == 0) {     // HI IDLE
      digitalWrite(pinHiIdle, LOW);
      }
      else {                 // (buf[10] == 1)
      digitalWrite(pinHiIdle, HIGH);
      }
      
      if (buf[11] == 2) {
      digitalWrite(pinStart, LOW);
      digitalWrite(pinKill, HIGH);
      }
      else if (buf[11] == 3) {
      digitalWrite(pinStart, HIGH);
      digitalWrite(pinKill, LOW);
      }
      else {                   // (buf[11] == 4) Both OFF
      digitalWrite(pinStart, HIGH);
      digitalWrite(pinKill, HIGH);
      }
        
        newData = false;
    }
}
