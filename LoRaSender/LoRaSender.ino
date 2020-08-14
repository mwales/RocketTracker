
#include <stdint.h>
#include "NmeaParsing.h"

#ifdef ARDUINO

#include <SPI.h>
#include <RH_RF95.h>

/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
*/
 
// for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int counter = 0;
void sendHelloPacket()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  char msgBuf[20];
  sprintf(msgBuf, "hello %d", counter);
  rf95.send( (uint8_t*) msgBuf, strlen(msgBuf) + 1);
  rf95.waitPacketSent();

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

void loraSetup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  delay(100);
 
  Serial.println("Feather LoRa TX Test!");
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void setup()
{
#ifdef ARDUINO
   Serial.begin(115200);
   while (!Serial)
   {
    delay(1);
   }

   loraSetup();
  
   GPSSerial.begin(9600);

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

void checkBatteryLevel()
{
  #define VBATPIN A7
   
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); 
  Serial.println(measuredvbat);
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

  if (strcmp(cmd,"batt") == 0)
  {
    checkBatteryLevel();
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
  Serial.println("batt - battery level");
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
