
#include <stdint.h>
#include "NmeaParsing.h"

#ifdef ARDUINO

#include <SPI.h>
#include <LoRa.h>

//  //LoR32u4 433MHz V1.0 (white board)
//  #define SCK     15
//  #define MISO    14
//  #define MOSI    16
//  #define SS      1
//  #define RST     4
//  #define DI0     7
//  #define BAND    433E6 
//  #define PABOOST true

//  //LoR32u4 433MHz V1.2 (white board)
//  #define SCK     15
//  #define MISO    14
//  #define MOSI    16
//  #define SS      8
//  #define RST     4
//  #define DI0     7
//  #define BAND    433E6 
//  #define PABOOST true 

  //LoR32u4II 868MHz or 915MHz (black board)
  #define SCK     15
  #define MISO    14
  #define MOSI    16
  #define SS      8
  #define RST     4
  #define DI0     7
  #define BAND    915E6 // 868E6  // 915E6
  #define PABOOST true 


int counter = 0;
void sendHelloPacket()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
}

#define LORA_IDLE_0 0
#define LORA_IDLE_1 1
#define LORA_IDLE_2 2

uint16_t  loraClock = 0;
uint8_t loraState = LORA_IDLE_0;

void checkLoraState()
  {
    uint16_t curClock = (millis() >> 10) & 0xffff;
    if (curClock == loraClock)
    {
      return;
    }

    // We will only get to this point at 1Hz
    loraClock = curClock;
    loraState++;
    if ((loraState & 0x7) > 5)
    {
      // 5 Hz idle update
      sendHelloPacket();

      loraState = 0;
    }
  }


#endif


#define GPSSerial Serial1


void setup()
{
#ifdef ARDUINO
   Serial.begin(115200);
   GPSSerial.begin(9600);


   while (!Serial);
   Serial.println("LoRa Sender");
   LoRa.setPins(SS,RST,DI0);
   if (!LoRa.begin(BAND,PABOOST))
   {
      Serial.println("Starting LoRa failed!");
      while (1);
   }
#endif

   for(uint8_t i = 0; i < GPS_BUF_MAX_SIZE + 1; i++)
   {
      gpsBuf[i] = 0;
   }
}



void sendNmeaCmd(char* cmd)
{
  uint8_t checksum = 0;
  uint8_t cmdLen = strlen(cmd);
  for(uint8_t i = 0; i < cmdLen; i++)
  {
    checksum ^= cmd[i];
  }
  Serial.print("\nSending to GPS: ");
  
  Serial.print('$');
  GPSSerial.print('$');
  for(uint8_t i = 0; i < cmdLen; i++)
  {
    Serial.print(cmd[i]);
    GPSSerial.print(cmd[i]);
  }

  Serial.print('*');
  Serial.print(intToHex(checksum >> 4));
  Serial.println(intToHex(checksum & 0x0f));


  GPSSerial.print('*');
  GPSSerial.print(intToHex(checksum >> 4));
  GPSSerial.print(intToHex(checksum & 0x0f));
  GPSSerial.print('\r');
  GPSSerial.print('\n');
  
}

void handleDebugCommand(char* cmd)
{
  if (strcmp(cmd, "gpmtk") == 0)
  {
    sendNmeaCmd("GPMTK605");
    return;
  }

  if (strcmp(cmd, "gfast") == 0)
  {
    sendNmeaCmd("PMTK251,115200");
    return;
  }

  if (strcmp(cmd, "gslow") == 0)
  {
    sendNmeaCmd("PMTK251,9600");
    return;
  }

  if (strcmp(cmd, "fast") == 0)
  {
    Serial.println("Changing GPS baud to 115200");
    GPSSerial.flush();
    GPSSerial.begin(115200);
    return;
  }

  if (strcmp(cmd, "slow") == 0)
  {
    Serial.println("Changing GPS baud to 9600");
    GPSSerial.flush();
    GPSSerial.begin(9600);
    return;
  }

  if (strcmp(cmd, "10hz") == 0)
  {
    sendNmeaCmd("PMTK220,100");
    return;
  }

  if (strcmp(cmd, "3hz") == 0)
  {
    sendNmeaCmd("PMTK220,333");
    return;
  }

  if (strcmp(cmd, "1hz") == 0)
  {
    sendNmeaCmd("PMTK220,1000");
    return;
  }

  if (strcmp(cmd, "5s") == 0)
  {
    sendNmeaCmd("PMTK220,5000");
    return;
  }

  // Show help
  Serial.println("**** HELP MENU ****");
  Serial.println("gpmtk - getVer");
  Serial.println("gfast - cmd fast baud");
  Serial.println("gslow - cmd slow baud");
  Serial.println("fast - set port to fast baud");
  Serial.println("slow - set port to slow baud");
  Serial.println("10hz - fastest gps updates");
  Serial.println("3hz - faster gps updates");
  Serial.println("1hz - default gps updates");
  Serial.println("5s - slow gps updates");  
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
  while(GPSSerial.available())
  {
    char gpsDat = GPSSerial.read();

    Serial.print(gpsDat);

    processGpsData(gpsDat);

    if ( (gpsDat == '\r') || (gpsDat == '\n') )
    {
      // After every newline, send a hello packet
      checkLoraState();
    }
  }

  if (Serial.available())
  {
    readDebugCommands();
  }
  
  checkLoraState();

}
