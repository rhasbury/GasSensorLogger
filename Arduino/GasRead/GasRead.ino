/* Testing MQ-2 GAS sensor with serial monitor
   Suitable for detecting of LPG, i-butane, propane, methane ,alcohol, Hydrogen or smoke
   More info: http://www.ardumotive.com/how-to-use-mq2-gas-sensor-en.html
   Dev: Michalis Vasilakis // Date: 11/6/2015 // www.ardumotive.com                     */
#include "Adafruit_CCS811.h"

const int gasPin = A0; //GAS sensor output pin to Arduino analog A0 pin
const int gasPin1 = A1; //GAS sensor output pin to Arduino analog A0 pin
const int gasPin2 = A2; //GAS sensor output pin to Arduino analog A0 pin

// constants for Sharp GP2Y1010AU0F Particle Sensor
const int ledPower = 2;
const int delayTime=280;
const int dustPin=A3;
const int delayTime2=40;

Adafruit_CCS811 ccs;

 /*
 Sharp pin 1 (V-LED)   => 5V (connected to 150ohm resister)
 Sharp pin 2 (LED-GND) => Arduino GND pin
 Sharp pin 3 (LED)     => Arduino pin D2
 Sharp pin 4 (S-GND)   => Arduino GND pin
 Sharp pin 5 (Vo)      => Arduino A3 pin
 Sharp pin 6 (Vcc)     => 5V
 */


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup()
{
  pinMode(ledPower,OUTPUT);
  pinMode(PD4, OUTPUT);
  digitalWrite(PD4,LOW);
  Serial.begin(9600); //Initialize serial port - 9600 bps
  Serial.println(SDA);
  Serial.println(SCL);
   // reserve 200 bytes for the inputString:
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");    
  }
      
  inputString.reserve(200);
}

void loop()
{
  

  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    
    if(inputString.indexOf("get_a0") >= 0){
      Serial.println(analogRead(gasPin));    
    }
    if(inputString.indexOf("get_a1") >= 0){
      Serial.println(analogRead(gasPin1));    
    }
    if(inputString.indexOf("get_a2") >= 0){
      Serial.println(analogRead(gasPin2));    
    }
    if(inputString.indexOf("get_co2") >= 0){
      if(!ccs.readData()){      
        Serial.println(ccs.geteCO2());              
      }
      else{
        Serial.println("ERROR!");      
      }
    }

    if(inputString.indexOf("get_tvoc") >= 0){
      if(!ccs.readData()){                    
        Serial.println(ccs.getTVOC());
      }
      else{
        Serial.println("ERROR!");      
      }
      
    }

    
    if(inputString.indexOf("get_dust") >= 0){
      
      
      digitalWrite(ledPower,LOW); // power on the LED
      delayMicroseconds(delayTime);
      //dustVal=analogRead(dustPin); // read the dust value
      Serial.println(analogRead(dustPin));      
      delayMicroseconds(delayTime2);
      digitalWrite(ledPower,HIGH); // turn the LED off
      //delayMicroseconds(offTime);
      
      
      //Serial.println(analogRead(gasPin2));    
    }


    
    //Serial.println(inputString);    
    inputString = "";
    stringComplete = false;
  }
  
  delay(100); // Print value every 1 sec.
}


/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' or inChar == ';') {
      stringComplete = true;
    }
  }
}
