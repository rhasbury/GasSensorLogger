
// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;                   // Create an instance
EnergyMonitor emon3;                   // Create an instance
EnergyMonitor emon4;                   // Create an instance

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

double powersum1 = 0;
double powersum2 = 0;
double powersum3 = 0;
double powersum4 = 0;
int AverageCount = 0;


void setup()
{  
  Serial.begin(115200);
  inputString.reserve(200);
  emon1.current(0, 111.11);             // Current: input pin, calibration.
  emon2.current(1, 111.11);             // Current: input pin, calibration.
  emon3.current(2, 606.1);             // Current: input pin, calibration.
  emon4.current(3, 606.1);             // Current: input pin, calibration.
}

void loop()
{
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

  powersum1 = powersum1 + emon1.calcIrms(1480);
  powersum2 = powersum2 + emon2.calcIrms(1480);
  powersum3 = powersum3 + emon3.calcIrms(1480);
  powersum4 = powersum4 + emon4.calcIrms(1480);
  AverageCount++;
  
  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    
    if(inputString.indexOf("get_power") >= 0){
      //double Irms = emon1.calcIrms(1480);  // Calculate Irms only          
      
      Serial.print("{\"power1\" : { \"power\" : ");
      Serial.print((powersum1/AverageCount) * 120.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum1/AverageCount);          // Irms    
      
      Serial.print("}, \"power2\" : { \"power\" : ");
      Serial.print((powersum2/AverageCount) * 120.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");      
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum2/AverageCount);           // Irms 
      
      Serial.print("}, \"power3\" : { \"power\" : ");
      Serial.print((powersum3/AverageCount) * 120.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");      
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum3/AverageCount);           // Irms      
      
      Serial.print("}, \"power4\" : { \"power\" : ");
      Serial.print((powersum4/AverageCount) * 120.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");      
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum4/AverageCount);           // Irms 
      
      Serial.println("}}");
      powersum1 = 0;
      powersum2 = 0;
      powersum3 = 0;
      powersum4 = 0;
      AverageCount = 0;
    }   

    if(inputString.indexOf("whatis") >= 0){        
      Serial.println("power");
    }

          
    inputString = "";
    stringComplete = false;
  }
  if(AverageCount > 10000){
      powersum1 = 0;
      powersum2 = 0;
      powersum3 = 0;
      powersum4 = 0;
      AverageCount = 0;
  }
  //delay(100);   

}
