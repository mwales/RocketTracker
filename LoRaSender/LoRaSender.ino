
#include <stdint.h>
#include "NmeaParsing.h"
#include "LoraComm.h"
#include "MissionData.h"


#include <SPI.h>





#define GPSSerial Serial1

#define ROCKET_ID          0x52
#define MISSION_CNTRL_ID   0x4d

#define LORA_IDLE_0 0
#define LORA_IDLE_1 1
#define LORA_IDLE_2 2

uint16_t  loraClock = 0;
uint8_t loraState = LORA_IDLE_0;

bool missionControlAutoUpdate = false;
bool processGpsMessages = false;

uint8_t missionControlMsg = 0;
void updateMissionControl()
{
  
   missionControlMsg = (missionControlMsg + 1) % 8;
   switch(missionControlMsg)
   {
   case MC_TIMESTAMP_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_TIMESTAMP_ID, mdGetTimestamp(), true);
      return;
   case MC_POSITION_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_POSITION_ID, mdGetPosition(), true);
      return;
   case MC_ALTITUDE_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_ALTITUDE_ID, mdGetAltitude(), true);
      return;
   case MC_BATTERY_ID:
   {
      float batt = checkBatteryLevel();
      mdUpdateBattery(batt);
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_BATTERY_ID, mdGetBattery(), true);
      return;
   }
   case MC_GPS_FIX_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_GPS_FIX_ID, mdGetFix(), true);
      return;
   case MC_NUM_SATS_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_NUM_SATS_ID, mdGetNumSats(), true);
      return;
   case MC_SPEED_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_SPEED_ID, mdGetSpeed(), true);
      return;
   case MC_PEAK_ALT_ID:
      loraSendMsg(MISSION_CNTRL_ID, ROCKET_ID, MC_PEAK_ALT_ID, mdGetPeakAltitude(), true);
      return;
   }
}

void checkMissionControlUpdateClock()
  {
    if (!missionControlAutoUpdate)
    {
      return;
    }
    
    uint16_t curClock = (millis() >> 9) & 0xffff;
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
      updateMissionControl();

      loraState = 0;
    }
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

   // Initialize mission data
   setupMissionData();
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

float checkBatteryLevel()
{
  #define VBATPIN A7
   
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); 
  Serial.println(measuredvbat);

  return measuredvbat;
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

  if (strcmp(cmd,"auto") == 0)
  {
    missionControlAutoUpdate = true;
    Serial.println("Mission control update mode = auto");
    return;
  }

  if (strcmp(cmd,"man") == 0)
  {
    missionControlAutoUpdate = false;
    Serial.println("Mission control update mode = manual");
    return;
  }

  if (strcmp(cmd,"update") == 0)
  {
    updateMissionControl();
    return;
  }

  if (strcmp(cmd,"hello") == 0)
  {
    loraSendHelloPacket();
    return;
  }

  if (strcmp(cmd,"hello2") == 0)
  {
    loraSendEncHelloPacket2();
    return;
  }

  if (strcmp(cmd,"helenc") == 0)
  {
    loraSendEncHelloPacket();
    return;
  }

    if (strcmp(cmd,"gpson") == 0)
  {
    processGpsMessages = true;
    Serial.println("GPS processing enabled");
    return;
  }

    if (strcmp(cmd,"gpsoff") == 0)
  {
    processGpsMessages = false;
    Serial.println("GPS processing disabled");
    return;
  }

    if (strcmp(cmd,"peak") == 0)
  {
    mdPrintPeakAltitude();
    return;
  }

    if (strcmp(cmd,"rpeak") == 0)
  {
    mdResetPeakAltitude();
    Serial.println("Reset peak altitude to -500");
    return;
  }

  // Show help
  Serial.println("**** HELP MENU ****");
  Serial.println("gpmtk  - getVer");
  Serial.println("gfast  - cmd fast baud");
  Serial.println("gslow  - cmd slow baud");
  Serial.println("fast   - set port to fast baud");
  Serial.println("slow   - set port to slow baud");
  Serial.println("10hz   - fastest gps updates");
  Serial.println("3hz    - faster gps updates");
  Serial.println("1hz    - default gps updates");
  Serial.println("5s     - slow gps updates");
  Serial.println("batt   - battery level");
  Serial.println("auto   - update mission control automatically");
  Serial.println("man    - update mission control manually");
  Serial.println("udpate - send mission control update");
  Serial.println("hello  - send hello packet");
  Serial.println("helenc - send encrypted hello packet");
  Serial.println("hello2 - send encrypted hello packet 2nd way");
  Serial.println("gpson  - turn GPS processing on");
  Serial.println("gpsoff - turn GPS processing off");
  Serial.println("peak   - print peak altitude");
  Serial.println("rpeak  - reset peak altitude");
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
  if (processGpsMessages)
  {
    while(GPSSerial.available())
    {
      char gpsDat = GPSSerial.read();
  
      // Serial.print(gpsDat);
  
      processGpsData(gpsDat);
    }
  }

  if (Serial.available())
  {
    readDebugCommands();
  }
  
  checkMissionControlUpdateClock();

  // delay(100);
  // Serial.print(".");

}
