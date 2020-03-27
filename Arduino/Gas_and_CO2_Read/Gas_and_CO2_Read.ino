/* Testing MQ-2 GAS sensor with serial monitor
   Suitable for detecting of LPG, i-butane, propane, methane ,alcohol, Hydrogen or smoke
   More info: http://www.ardumotive.com/how-to-use-mq2-gas-sensor-en.html
   Dev: Michalis Vasilakis // Date: 11/6/2015 // www.ardumotive.com                     */
#include "Adafruit_CCS811.h"
#include "CircularBuffer.h"

const int loopdelay = 10;  // 10 ms loopdelay
const int buffsize = 20;  // 10 ms loopdelay


CircularBuffer<int, buffsize> a0_buff;
CircularBuffer<int, buffsize> a1_buff;
CircularBuffer<int, buffsize> a2_buff;
CircularBuffer<int, buffsize> co2_buff;
CircularBuffer<int, buffsize> tvoc_buff;


Adafruit_CCS811 ccs;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup()
{


  Serial.begin(115200); //Initialize serial port
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
  
  

  a0_buff.push(analogRead(A0));
  a1_buff.push(analogRead(A1));
  a2_buff.push(analogRead(A2));
  if(!ccs.readData()){
    co2_buff.push(ccs.geteCO2());
    tvoc_buff.push(ccs.getTVOC());
  }
  else{
        Serial.println("CSS ERROR!");      
      } 



  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    
    if(inputString.indexOf("get_a0") >= 0){
      Serial.println(a0_buff.last());    
    }
    if(inputString.indexOf("get_a1") >= 0){
      Serial.println(a1_buff.last());    
    }
    if(inputString.indexOf("get_a2") >= 0){
      Serial.println(a2_buff.last());    
    }
    if(inputString.indexOf("get_co2") >= 0){
      Serial.println(co2_buff.last());    
    }
    
    if(inputString.indexOf("get_tvoc") >= 0){
      Serial.println(tvoc_buff.last());        
    }
    
    
    if(inputString.indexOf("get_a0_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(tvoc_buff)::index_t;
        for (index_t i = 0; i < tvoc_buff.size(); i++) {
          avg += tvoc_buff[i] / tvoc_buff.size();
        }        
        Serial.println(avg); 
    }
    //Serial.println(inputString);    
    inputString = "";
    stringComplete = false;
  }


  
  
  delay(loopdelay); // Print value every 1 sec.
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
