// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
#include <EEPROM.h>
char sID[7];

#define OLEDINSTALLED true

#if (OLEDINSTALLED == true)
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#endif



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
  
   for (int i=0; i<6; i++) {
   sID[i] = EEPROM.read(i);
   }
  
  Serial.begin(115200);

  #if (OLEDINSTALLED == true)
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Clear the buffer.
  //display.clearDisplay();
  //display.display();
  //delay(50);
  //display.clearDisplay();
  #endif

  
  inputString.reserve(200);
  emon1.current(A2, 15);             // Current: input pin, calibration.
  emon2.current(A3, 20);             // Current: input pin, calibration.
  emon3.current(A0, 606.0);             // Current: input pin, calibration.
  emon4.current(A1, 606.0);             // Current: input pin, calibration.

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



  double Irms1 = emon1.calcIrms(1480);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(1480);  // Calculate Irms only
  double Irms3 = emon3.calcIrms(1480);  // Calculate Irms only
  double Irms4 = emon4.calcIrms(1480);  // Calculate Irms only


  powersum1 = powersum1 + Irms1;
  powersum2 = powersum2 + Irms2;
  powersum3 = powersum3 + Irms3;
  powersum4 = powersum4 + Irms4;
  AverageCount++;

  #if (OLEDINSTALLED == true)
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(Irms1);
  display.setCursor(0,15);
  display.println(Irms2);
  display.display();  

  #endif
  




  
  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    
    if(inputString.indexOf("get_power") >= 0){
      //double Irms = emon1.calcIrms(1480);  // Calculate Irms only          
      
      Serial.print("{\"power1\" : { \"power\" : ");
      Serial.print((powersum1/AverageCount) * 113.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum1/AverageCount);          // Irms    
      
      Serial.print("}, \"power2\" : { \"power\" : ");
      Serial.print((powersum2/AverageCount) * 113.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");      
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum2/AverageCount);           // Irms 
      
      Serial.print("}, \"power3\" : { \"power\" : ");
      Serial.print((powersum3/AverageCount) * 226.0);         // Apparent power
      Serial.print(", \"averagecount\" : ");      
      Serial.print(AverageCount);         
      Serial.print(", \"current\" : ");
      Serial.print(powersum3/AverageCount);           // Irms      
      
      Serial.print("}, \"power4\" : { \"power\" : ");
      Serial.print((powersum4/AverageCount) * 226.0);         // Apparent power
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
      Serial.print("power, ");
      Serial.println(sID);
    }
    if(inputString.indexOf("get_serial") >= 0){        
      Serial.println(sID);
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
