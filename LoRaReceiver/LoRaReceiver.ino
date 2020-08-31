/**
 * Receiver portion of the rocket tracker.  Started out as Feather9x_RX example code
 * (I think from Adafruit)
 */

#include <stdint.h>


#include "LoraComm.h"
 


 

 
// Blinky on receipt
#define LED 13



 
void setup()
{
  pinMode(LED, OUTPUT);

 
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  delay(100);
 
  loraSetup();
}







void handleDebugCommand(char* cmd)
{
  if (strcmp(cmd, "launch") == 0)
  {
    //sendNmeaCmd("GPMTK605");
    return;
  }

  if (strcmp(cmd, "stand") == 0)
  {
    //sendNmeaCmd("PMTK251,115200");
    return;
  }

  if (strcmp(cmd, "where") == 0)
  {
    //sendNmeaCmd("PMTK251,9600");
    return;
  }

  if (strcmp(cmd, "howhi") == 0)
  {
    //Serial.println("Changing GPS baud to 115200");
    //GPSSerial.flush();
    //GPSSerial.begin(115200);
    return;
  }

  // Show help
  Serial.println("**** HELP MENU ****");
  Serial.println("launch  - go into launch mode");
  Serial.println("stand  - cmd fast baud");
  Serial.println("where  - cmd slow baud");
  Serial.println("howhi   - set port to fast baud");
  
}

void readDebugCommands()
{
  char shortBuf[10];
  char* currentChar = shortBuf;
  *currentChar = Serial.read();
  while( (*currentChar != '\n') && (*currentChar != '\r') )
  {
    while(!Serial.available())
    {
      delay(1);
      
    }
    currentChar++;
    *currentChar = Serial.read();
  }

  // Add null terminator (and replace newline)
  *currentChar = 0;

  Serial.print("Received command: ");
  Serial.println(shortBuf);

  handleDebugCommand(shortBuf);
}
 
void loop()
{
  loraReceiveMessage();

  if (Serial.available())
  {
    readDebugCommands();
  }

  delay(5);
  //Serial.print(".");
}
