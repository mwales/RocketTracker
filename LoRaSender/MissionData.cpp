#include "MissionData.h"
#include "Arduino.h"
#include <string.h>


#define LAT_LONG_POS_REPORT_LEN 18
char mdLastPosition[LAT_LONG_POS_REPORT_LEN];

void mdUpdatePosition(char* lat, char* ns, char* longitude, char* ew)
{
  memcpy(mdLastPosition + 0, lat, 7);
  memcpy(mdLastPosition + 7, ns, 1);
  memcpy(mdLastPosition + 8, longitude, 8);
  memcpy(mdLastPosition + 16, ew, 1);  
}
char* mdGetPosition()
{
  return mdLastPosition;
}

#define TIMESTAMP_REPORT_LEN      7
char mdLastTimestamp[TIMESTAMP_REPORT_LEN];

void mdUpdateTimestamp(char* ts)
{
  memcpy(mdLastTimestamp, ts, 6);
}

char* mdGetTimestamp()
{
  return mdLastTimestamp;
}


#define ALTITUDE_REPORT_LEN      7
char mdLastAltitude[ALTITUDE_REPORT_LEN];

void mdUpdateAltitude(char* alt)
{
  float altFloat = atof(alt);
  snprintf(mdLastAltitude, ALTITUDE_REPORT_LEN, "%04.1f", altFloat);
}

char* mdGetAltitude()
{
  return mdLastAltitude;
}

#define BATTERY_REPORT_LEN      4
char mdLastBattery[BATTERY_REPORT_LEN];

void mdUpdateBattery(float batteryLevel)
{
  int battery = batteryLevel;
  if (battery > 100)
  {
    battery = 100;
  }

  if (battery < 0)
  {
    battery = 0;
  }

  sprintf(mdLastBattery, "%03d", battery);
}

char* mdGetBattery()
{
  return mdLastBattery;
}

#define FIX_REPORT_LEN    8
char mdLastFix[FIX_REPORT_LEN];

void mdUpdateFix(char* fix, char* pdop, char* hdop, char* vdop)
{
  float dop = 0.0;
  dop += atof(pdop);
  dop += atof(hdop);
  dop += atof(vdop);
  dop /= 0.3;

  uint8_t dopInt = dop;

  mdLastFix[0] = fix[0];
  snprintf(mdLastFix + 1, 7, "_%03.1f", dopInt); 
}

char* mdGetFix()
{
  return mdLastFix;
}

#define NUM_SATS_REPORT_LEN 3
char mdLastNumSats[NUM_SATS_REPORT_LEN];
void mdUpdateNumSats(char* numSats)
{
  uint8_t numSatsInt = atoi(numSats);
  snprintf(mdLastNumSats,NUM_SATS_REPORT_LEN,"%02d", numSatsInt);
}

char* mdGetNumSats()
{
  return mdLastNumSats;
}


#define SPEED_REPORT_LEN   6
char mdLastSpeed[SPEED_REPORT_LEN];
void mdUpdateSpeed(char* speed)
{
  float speedFloat = atof(speed);
  snprintf(mdLastSpeed, SPEED_REPORT_LEN, "%04.1f", speedFloat);
}

char* mdGetSpeed()

{
  return mdLastSpeed;
}



void setupMissionData()
{
  strcpy(mdLastPosition,"----.--N-----.--W");
  strcpy(mdLastTimestamp, "------");
  strcpy(mdLastAltitude, "------");
  strcpy(mdLastBattery, "---");
  strcpy(mdLastNumSats, "--");
}
