/* Testing MQ-2 GAS sensor with serial monitor
   Suitable for detecting of LPG, i-butane, propane, methane ,alcohol, Hydrogen or smoke
   More info: http://www.ardumotive.com/how-to-use-mq2-gas-sensor-en.html
   Dev: Michalis Vasilakis // Date: 11/6/2015 // www.ardumotive.com                     */
#include "Adafruit_CCS811.h"
#include "CircularBuffer.h"

const int loopdelay = 100;  // 10 ms loopdelay
const int buffsize = 100;  //


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
   
  pinMode(5, OUTPUT); //setup nWAKE pin
  digitalWrite(5, LOW);
   // reserve 200 bytes for the inputString:
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");    
  }
      
  inputString.reserve(200);
}



void loop()
{
    if(Serial.available()) {
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
    
    
    if(inputString.indexOf("get_tvoc_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(tvoc_buff)::index_t;
        for (index_t i = 0; i < tvoc_buff.size(); i++) {
          avg += tvoc_buff[i] / tvoc_buff.size();
        }        
        Serial.println(avg); 
    }

    if(inputString.indexOf("get_co2_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(co2_buff)::index_t;
        for (index_t i = 0; i < co2_buff.size(); i++) {
          avg += co2_buff[i] / co2_buff.size();
        }        
        Serial.println(avg); 
    }

    if(inputString.indexOf("get_a0_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(a0_buff)::index_t;
        for (index_t i = 0; i < a0_buff.size(); i++) {
          avg += a0_buff[i] / a0_buff.size();
        }        
        Serial.println(avg); 
    }

    if(inputString.indexOf("get_a1_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(a1_buff)::index_t;
        for (index_t i = 0; i < a1_buff.size(); i++) {
          avg += a1_buff[i] / a1_buff.size();
        }        
        Serial.println(avg); 
    }
    
    if(inputString.indexOf("get_a2_avg") >= 0){
        float avg = 0.0;
        // the following ensures using the right type for the index variable
        using index_t = decltype(a2_buff)::index_t;
        for (index_t i = 0; i < a2_buff.size(); i++) {
          avg += a2_buff[i] / a2_buff.size();
        }        
        Serial.println(avg); 
    }

        
    if(inputString.indexOf("get_all_avg_json") >= 0){
      float avg = 0.0;
      // the following ensures using the right type for the index variable
      using index_t = decltype(a0_buff)::index_t;
      for (index_t i = 0; i < a0_buff.size(); i++) {
        avg += a0_buff[i] / a0_buff.size();
      }        
      Serial.print("{\"a0_avg\" : ");
      Serial.print(avg);                              

      avg = 0.0;
      // the following ensures using the right type for the index variable
      using index_t = decltype(a1_buff)::index_t;
      for (index_t i = 0; i < a1_buff.size(); i++) {
        avg += a1_buff[i] / a1_buff.size();
      }        
          
      Serial.print(", \"a1_avg\" : ");
      Serial.print(avg);

      avg = 0.0;
      // the following ensures using the right type for the index variable
      using index_t = decltype(a2_buff)::index_t;
      for (index_t i = 0; i < a2_buff.size(); i++) {
        avg += a2_buff[i] / a2_buff.size();
      }  
      
      Serial.print(", \"a2_avg\" : ");
      Serial.print(avg);

      
      avg = 0.0;
      // the following ensures using the right type for the index variable
      using index_t = decltype(co2_buff)::index_t;
      for (index_t i = 0; i < co2_buff.size(); i++) {
        avg += co2_buff[i] / co2_buff.size();
      }
      
      Serial.print(", \"co2_avg\" : ");
      Serial.print(avg);

      
      avg = 0.0;
      // the following ensures using the right type for the index variable
      using index_t = decltype(tvoc_buff)::index_t;
      for (index_t i = 0; i < tvoc_buff.size(); i++) {
        avg += tvoc_buff[i] / tvoc_buff.size();
      }   
      
      Serial.print(", \"tvoc_avg\" : ");
      Serial.print(avg);
      
      Serial.println("}");
        //Serial.println(avg); 
    }

    
    //Serial.println(inputString);    
    inputString = "";
    stringComplete = false;
  }


  
  
  delay(loopdelay); // Print value every 1 sec.
}
