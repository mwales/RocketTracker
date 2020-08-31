#include "MissionData.h"
#include "Arduino.h"
#include <string.h>
#include <stdint.h>


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
char mdPeakAltitude[ALTITUDE_REPORT_LEN];

float peakAltitude;

void mdUpdateAltitude(char* alt)
{
  float altFloat = atof(alt);

  if (altFloat > peakAltitude)
  {
    // Update peak altitude if neccessary
    peakAltitude = altFloat;
  }
  
  uint16_t metersTimes10 = altFloat * 10.0;
  snprintf(mdLastAltitude, ALTITUDE_REPORT_LEN, "%05d", metersTimes10);
}

char* mdGetAltitude()
{
  return mdLastAltitude;
}

char* mdGetPeakAltitude()
{
  // We do conversion to string here cause we will do a lot during ascent
  uint16_t metersTimes10 = peakAltitude * 10.0;
  snprintf(mdPeakAltitude, ALTITUDE_REPORT_LEN, "%05d", metersTimes10);
  return mdPeakAltitude;
}



void mdResetPeakAltitude()
{
  peakAltitude = -500.0;
}

void mdPrintPeakAltitude()
{
  uint16_t paIntTimes10 = peakAltitude * 10.0;
  uint16_t wholePart = paIntTimes10 / 10;
  uint16_t fracPart = paIntTimes10 % 10;
  Serial.print("Peak Altitude = ");
  Serial.print(wholePart);
  Serial.print(".");
  Serial.println(fracPart);
}

#define BATTERY_REPORT_LEN      10
char mdLastBattery[BATTERY_REPORT_LEN];


// Info from blog.ampow.com/lipo-voltage-chart
char* voltageToBatteryLife(float const & battVolt)
{
  if (battVolt > 3.85)
  {
    if (battVolt > 4.2)
    {
      return "100";
    }
    if (battVolt > 4.15)
    {
      return "95";
    }
    if (battVolt > 4.11)
    {
      return "90";
    }
    if (battVolt > 4.08)
    {
      return "85";
    }
    if (battVolt > 4.02)
    {
      return "80";
    }
  
    if (battVolt > 3.98)
    {
      return "75";
    }
    if (battVolt > 3.95)
    {
      return "70";
    }
    if (battVolt > 3.91)
    {
      return "65";
    }
    if (battVolt > 3.87)
    {
      return "60";
    }
    return "55";
  }

  if (battVolt > 3.84)
  {
    return "50";
  }
  if (battVolt > 3.82)
  {
    return "45";
  }
  if (battVolt > 3.8)
  {
    return "40";
  }
  if (battVolt > 3.79)
  {
    return "35";
  }
  if (battVolt > 3.77)
  {
    return "30";
  }

  if (battVolt > 3.75)
  {
    return "25";
  }
  if (battVolt > 3.73)
  {
    return "20";
  }
  if (battVolt > 3.71)
  {
    return "15";
  }
  if (battVolt > 3.69)
  {
    return "10";
  }
  if (battVolt > 3.61)
  {
    return "5";
  }

  return "0";  
}

void mdUpdateBattery(float batteryLevel)
{
  Serial.print("batteryLevel=");
  Serial.println(batteryLevel);
  Serial.print("percent=");
  Serial.println(voltageToBatteryLife(batteryLevel));

  uint16_t millivolts = (batteryLevel * 1000.0);
  
  snprintf(mdLastBattery, BATTERY_REPORT_LEN, "%04d,%s", millivolts, voltageToBatteryLife(batteryLevel));

  Serial.println(mdLastBattery);
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
  snprintf(mdLastFix + 1, 7, "_%03d", dopInt); 
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
  uint16_t speedKmhTimes10 = speedFloat * 10.0;
  snprintf(mdLastSpeed, SPEED_REPORT_LEN, "%04d", speedKmhTimes10);
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
  strcpy(mdLastBattery, "0.0V,0");
  strcpy(mdLastNumSats, "--");
}
