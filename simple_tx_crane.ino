// SimpleTx - the master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//LRF pin number
#define CE_PIN 49
#define CSN_PIN 53

//tx-rx encryption code thing
const byte slaveAddress[5] = {'T','E','R','A','X'};


RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 50;

// Define map output parameters
const int mapLo = 23; //63; slow all  // calibrated to 25% PWM
const int mapMd = 127; //127;  // calibrated to 50% PWM  
const int mapHi = 231;  //191;  // calibrated to 75% PWM
const int WmapLo = 43;  //63;   // calibrated to 25% PWM
const int WmapMd = 127;  //127;  // calibrated to 50% PWM  
const int WmapHi = 250;  //191;  // calibrated to 75% PWM

// Declare unsigned 8-bit joystick array
// 6 Bytes for joystick proportional axiis
// 6 bytes for digital pins
uint8_t joystickArray[12]; 

// Define Joystick Connections
#define joy2EXT    A0 
#define joy3EXT    A1
#define joyDIGR    A2
#define joyWNCH    A3
#define joyROTA    A4
#define joyLIFT    A5

// Define Joystick Values - Start at 512 (middle position)
int joypos2EXT = 512;
int joypos3EXT = 512;
int joyposDIGR = 512;
int joyposWNCH = 512;
int joyposROTA = 512;
int joyposLIFT = 512;

// Define Digital pin input
const int pinEstop = 22;
// const int pinValves = 23;  // placeholder, pin 23 not used
const int pinHiIdle = 24;
const int pinDigWin = 25;   // select between digger/winch
const int pinTiltUp = 27;
const int pinTiltDn = 26;
const int pinClawOp = 29;
const int pinClawCl = 28;
const int pinHiLo = 31;
const int pinDigRel = 30;
const int pinStart = 33;
const int pinKill = 32;

void setup() {

    Serial.begin(9600);

    Serial.println("SimpleTx Starting");

    radio.begin();
    radio.setDataRate( RF24_2MBPS );
    radio.setChannel(28);
    // radio.setRetries(3,5); // delay, count
    radio.openWritingPipe(slaveAddress);
    
  pinMode(pinEstop, INPUT_PULLUP);
  pinMode(pinHiIdle, INPUT_PULLUP);
  pinMode(pinDigWin, INPUT_PULLUP);
  pinMode(pinTiltUp, INPUT_PULLUP);
  pinMode(pinTiltDn, INPUT_PULLUP);
  pinMode(pinClawOp, INPUT_PULLUP);
  pinMode(pinClawCl, INPUT_PULLUP);
  pinMode(pinHiLo, INPUT_PULLUP);
  pinMode(pinDigRel, INPUT_PULLUP);
  pinMode(pinStart, INPUT_PULLUP);
  pinMode(pinKill, INPUT_PULLUP);

  digitalWrite(pinEstop, HIGH);
  digitalWrite(pinHiIdle, HIGH);
  digitalWrite(pinDigWin, HIGH);
  digitalWrite(pinTiltUp, HIGH);
  digitalWrite(pinTiltDn, HIGH);
  digitalWrite(pinClawOp, HIGH);
  digitalWrite(pinClawCl, HIGH);
  digitalWrite(pinHiLo, HIGH);
  digitalWrite(pinDigRel, HIGH);
  digitalWrite(pinStart, HIGH);
  digitalWrite(pinKill, HIGH);
  
}

//====================

void loop() {
    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
        getdata();
        send();
        prevMillis = millis();
    }
}

void getdata() {
    // Print to Serial Monitor
  Serial.println("Reading joystick values ");
  
  // Read the Joystick positions
  joypos2EXT = analogRead(joy2EXT); 
  joypos3EXT = analogRead(joy3EXT);
  joyposDIGR = analogRead(joyDIGR);
  joyposWNCH = analogRead(joyWNCH);
  joyposROTA = analogRead(joyROTA);
  joyposLIFT = analogRead(joyLIFT);

  joystickArray[0] = map(joypos2EXT, 0, 1023, mapLo, mapHi);
  joystickArray[1] = map(joypos3EXT, 1023, 0, mapLo, mapHi);
  joystickArray[4] = map(joyposROTA, 1023, 0, mapLo, mapHi);
  joystickArray[5] = map(joyposLIFT, 1023, 0, mapLo, mapHi);

  if (digitalRead(pinDigWin) == HIGH) {
        joystickArray[2] = mapMd;
        joystickArray[3] = map(joyposWNCH, 0, 1023, WmapLo, WmapHi);
  }
  else { // (digitalRead(pinDigWin) == LOW)  // use winch joystick axis to control digger
        joystickArray[2] = map(joyposWNCH, 0, 1023, mapLo, mapHi);
        joystickArray[3] = mapMd;
  }

  if (digitalRead(pinEstop) == LOW) joystickArray[6]=0;
  if (digitalRead(pinEstop) == HIGH) joystickArray[6]=1;
  
  if (digitalRead(pinTiltUp) == LOW) joystickArray[7]=2;
  else if (digitalRead(pinTiltDn) == LOW) joystickArray[7]=3;
  else  joystickArray[7]=4;
  
  if (digitalRead(pinClawOp) == LOW) joystickArray[8]=2;
  else if (digitalRead(pinClawCl) == LOW) joystickArray[8]=3;
  else  joystickArray[8]=4;
  
  if (digitalRead(pinHiLo) == LOW) joystickArray[9]=2;
  else if (digitalRead(pinDigRel) == LOW) joystickArray[9]=3;
  else  joystickArray[9]=4;
  
  if (digitalRead(pinHiIdle) == LOW) joystickArray[10]=0;
  if (digitalRead(pinHiIdle) == HIGH) joystickArray[10]=1;
  
  if (digitalRead(pinStart) == LOW) joystickArray[11]=2;
  else if (digitalRead(pinKill) == LOW) joystickArray[11]=3;
  else  joystickArray[11]=4;


  /* sample code for custom map
  // joystick customize lower/upper ranges and mid

  if (joypos2EXT < 460)
  {
    joystickArray[0] = map(joypos2EXT, 0, 460, 63, 127);
  }
  else if (joypos2EXT > 564)
  {
    joystickArray[0] = map(joyposVert, 564, 1023, 128, 191; 
  }
  else
  {
    // This is Stopped
    joystickArray[0] = 127;

  }
  
  //repeat for other axiis and adjust as needed
  */
  

  //Display the Joystick values in the serial monitor.
  Serial.print("2nd Extension: ");
  Serial.println(joystickArray[0]);
  Serial.print("3rd Extension: ");
  Serial.println(joystickArray[1]);
  Serial.print("Digger:        ");
  Serial.println(joystickArray[2]);
  Serial.print("Winch:         ");
  Serial.println(joystickArray[3]);
  Serial.print("Rotation:     ");
  Serial.println(joystickArray[4]);
  Serial.print("Lift:         ");
  Serial.println(joystickArray[5]);
    
}

void send() {

    bool rslt;
    rslt = radio.write( &joystickArray, sizeof(joystickArray) );

    Serial.print("Data Sent ");
    /*
    if (rslt) {
        Serial.println("  Acknowledge received");
        // updateMessage(); //placeholder to do something after acknowledgement recieved
    }
    else {
        Serial.println("  Tx failed");
    }
    */
}
