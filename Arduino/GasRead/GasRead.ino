/* Testing MQ-2 GAS sensor with serial monitor
   Suitable for detecting of LPG, i-butane, propane, methane ,alcohol, Hydrogen or smoke
   More info: http://www.ardumotive.com/how-to-use-mq2-gas-sensor-en.html
   Dev: Michalis Vasilakis // Date: 11/6/2015 // www.ardumotive.com                     */

const int gasPin = A0; //GAS sensor output pin to Arduino analog A0 pin
const int gasPin1 = A1; //GAS sensor output pin to Arduino analog A0 pin
const int gasPin2 = A2; //GAS sensor output pin to Arduino analog A0 pin


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup()
{
  Serial.begin(9600); //Initialize serial port - 9600 bps
   // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop()
{
  
  //Serial.println(analogRead(gasPin));
  //Serial.println(analogRead(gasPin1));
  //Serial.println(analogRead(gasPin2));

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
 
//    Serial.println(inputString);    
    inputString = "";
    stringComplete = false;
  }
  
  delay(10); // Print value every 1 sec.
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
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
