#include "NmeaParsing.h"
#include "MissionData.h"

#include "Arduino.h"
#include <string.h>

#define NMEA_PARSING_DEBUG
#ifdef NMEA_PARSING_DEBUG
  #define nmeaDebug Serial.print
  #define nmeaDebugln  Serial.println
#else
  #define nmeaDebug if(0) Serial.print
  #define nmeaDebugln if(0) Serial.println
#endif

#define GPSMODE_NEW_LINE 0
#define GPSMODE_CHECKSUM_1       1       // We received the *, waiting for checksum bytes
#define GPSMODE_CHECKSUM_2       2       // Received first byte of checksum
#define GPSMODE_CHECKSUM_3       3       // Received second bytes of checksum

#define GPS_BUF_MAX_SIZE  80
char gpsBuf[GPS_BUF_MAX_SIZE + 1];
uint8_t checksumVal = 0;
uint8_t checksumVerify = 0;
uint8_t gpsBufPos = 0;
uint8_t gpsDecodeMode = GPSMODE_NEW_LINE;

uint8_t hexToInt(char data)
{
   if ( (data >= '0') && (data <= '9') )
   {
      return data - '0';
   }

   if ( (data >= 'a') && (data <= 'f') )
   {
      return data - 'a' + 10;
   }

   if ( (data >= 'A') && (data <= 'F') )
   {
      return data - 'A' + 10;
   }

   return 0xff;
}

char intToHex(uint8_t val)
{
  if (val < 10)
  {
    return '0' + val;
  }
  else
  {
    // '7' is 10 less than 'A'
    return '7' + val;
  }
}

void decodeNmeaGpggaFixDataMsg()
{
   nullTerminateGpsBuf();

   char* altitude = getGpsBufArg(9);
   char* timestamp = getGpsBufArg(1);
   char* lat1 = getGpsBufArg(2);
   char* lat2 = getGpsBufArg(3);
   char* lng1 = getGpsBufArg(4);
   char* lng2 = getGpsBufArg(5);
   char* fixArg = getGpsBufArg(6);

   if(altitude == NULL)
   {
      Serial.println("GPGGA missing some arg");
      return;
   }

   if (fixArg[0] == '0')
   {
      nmeaDebug("  No fix: ");
      return;
   }
   if (fixArg[0] == '1')
   {
      nmeaDebugln("  GPS fix");
   }
   if (fixArg[0] == '2')
   {
      nmeaDebugln("  DGPS fix");
   }

   nmeaDebug("  LAT: ");
   nmeaDebug(lat1);
   nmeaDebugln(lat2);   

   nmeaDebug("  LNG: ");
   nmeaDebug(lng1);
   nmeaDebugln(lng2);    

   nmeaDebug("  ALT: ");
   nmeaDebugln(altitude);

   mdUpdatePosition(lat1, lat2, lng1, lng2);
   mdUpdateAltitude(altitude);
   mdUpdateTimestamp(timestamp);
}

void nullTerminateGpsBuf()
{
   //printf("Buf Len = %d\n", gpsBufPos);
   for(uint8_t i = 0; i < gpsBufPos; i++)
   {
      if (gpsBuf[i] == ',')
      {
         // printf("null terminating at %d\n", i);
         gpsBuf[i] = 0;
      }
   }

   gpsBuf[gpsBufPos] = 0;
}

char* getGpsBufArg(uint8_t index)
{
   if (index == 0)
   {
      return &gpsBuf[0];
   }

   uint8_t indexCur = 0;
   for(uint8_t i = 0; i < gpsBufPos; i++)
   {
      if (gpsBuf[i] == 0)
      {
         indexCur++;

         if ( (indexCur == index) && (i+1 < gpsBufPos) )
         {
            return &gpsBuf[i+1];
         }
      }
   }

   return NULL;
}

void decodeNmeaGpvtgMadeGoodGroundSpeedMsg()
{
   nullTerminateGpsBuf();

   char* speedKmh = getGpsBufArg(7);
   if(speedKmh == NULL)
   {
      Serial.println("GPVTG missing some arg");
      return;
   }

   nmeaDebug("  SPD: ");
   nmeaDebug(speedKmh);
   nmeaDebugln(" kmh");

   mdUpdateSpeed(speedKmh);
}

void decodeNmeaGprmcGpsTransitDataMsg()
{
   nullTerminateGpsBuf();

   char* groundSpeedKnots = getGpsBufArg(7);
   char* lat1 = getGpsBufArg(3);
   char* lat2 = getGpsBufArg(4);
   char* lng1 = getGpsBufArg(5);
   char* lng2 = getGpsBufArg(6);

   if(groundSpeedKnots == NULL)
   {
      Serial.println("GPRMC missing some arg");
      return;
   }

   nmeaDebug("  LAT: ");
   nmeaDebug(lat1);
   nmeaDebugln(lat2);

   nmeaDebug("  LNG: ");
   nmeaDebug(lng1);
   nmeaDebugln(lng2);

   mdUpdatePosition(lat1, lat2, lng1, lng2);
}

void decodeNmeaGpgllLatLongMsg()
{
   nullTerminateGpsBuf();

   char* lat1 = getGpsBufArg(1);
   char* lat2 = getGpsBufArg(2);
   char* lng1 = getGpsBufArg(3);
   char* lng2 = getGpsBufArg(4);

   if(lng2 == NULL)
   {
      Serial.println("GPGLL missing some arg");
      return;
   }

   nmeaDebug("  LAT: ");
   nmeaDebug(lat1);
   nmeaDebugln(lat2);

   nmeaDebug("  LNG: ");
   nmeaDebug(lng1);
   nmeaDebugln(lng2);

   mdUpdatePosition(lat1, lat2, lng1, lng2);
}

void decodeNmeaGpgsaDopAndFixMsg()
{
   nullTerminateGpsBuf();

   char* fixArg = getGpsBufArg(2);
   char* pdop = getGpsBufArg(15);
   char* hdop = getGpsBufArg(16);
   char* vdop = getGpsBufArg(17);

   if(vdop == NULL)
   {
      Serial.println("GPGSA missing some arg");
      return;
   }

   if (fixArg[0] == '1')
   {
      nmeaDebug("  No fix: ");
      return;
   }
   if (fixArg[0] == '2')
   {
      nmeaDebugln("  2D fix");
   }
   if (fixArg[0] == '3')
   {
      nmeaDebugln("  3D fix");
   }

   nmeaDebug("  PDOP: ");
   nmeaDebugln(pdop);

   nmeaDebug("  HDOP: ");
   nmeaDebugln(hdop);

   nmeaDebug("  VDOP: ");
   nmeaDebugln(vdop);

   mdUpdateFix(fixArg, pdop, hdop, vdop);
}

void decodeNmeaGpgsvSatInViewMsg()
{
   nullTerminateGpsBuf();

   char* satsInView = getGpsBufArg(3);

   if(satsInView == NULL)
   {
      Serial.println("GPGSV missing some arg");
      return;
   }

   nmeaDebug("  #SAT: ");
   nmeaDebugln(satsInView);

   mdUpdateNumSats(satsInView);
}

void decodeNmeaMessage()
{
  nmeaDebugln("decodeNmeaMessage - enter");
  
   nmeaDebug("\nValid: ");

   if (gpsBufPos < 5)
   {
      Serial.println("NMEA message too small");
      return;
   }

   if (strncmp("GPGGA",gpsBuf, 5) == 0)
   {
      decodeNmeaGpggaFixDataMsg();
      return;
   }

   if (strncmp("GPVTG",gpsBuf, 5) == 0)
   {
      decodeNmeaGpvtgMadeGoodGroundSpeedMsg();
      return;
   }

   if (strncmp("GPRMC",gpsBuf, 5) == 0)
   {
      decodeNmeaGprmcGpsTransitDataMsg();
      return;
   }

   if (strncmp("GPGLL",gpsBuf, 5) == 0)
   {
      decodeNmeaGpgllLatLongMsg();
      return;
   }

   if (strncmp("GPGSA",gpsBuf, 5) == 0)
   {
      decodeNmeaGpgsaDopAndFixMsg();
      return;
   }

   if (strncmp("GPGSV",gpsBuf, 5) == 0)
   {
      decodeNmeaGpgsvSatInViewMsg();
      return;
   }

   if (strncmp("GPHDT",gpsBuf, 5) == 0)
   {
      Serial.println("True Heading");
      return;
   }

   if (strncmp("GPZDA",gpsBuf, 5) == 0)
   {
      Serial.println("Date And Time - Never Seen Before!!!");
      return;
   }

   if (strncmp("GPR00",gpsBuf, 5) == 0)
   {
      Serial.println("Waypoint IDS Msg = WTF");
      return;
   }

   if (strncmp("GPRMA",gpsBuf, 5) == 0)
   {
      Serial.println("Loran C - WTF");
      return;
   }

   if (strncmp("GPBOD",gpsBuf, 5) == 0)
   {
      nmeaDebugln("Bearing origin to dest msg - WTF");
      return;
   }

   if (strncmp("GPBWC",gpsBuf, 5) == 0)
   {
      Serial.println("Bearing and dist to waypoint - WTF");
      return;
   }

   if (strncmp("GPRMB",gpsBuf, 5) == 0)
   {
      Serial.println("Min Nav Msg - WTF");
      return;
   }

   Serial.print("Totally Unknown WTF message: ");
   Serial.print(gpsBuf[0]);
   Serial.print(gpsBuf[1]);
   Serial.print(gpsBuf[2]);
   Serial.print(gpsBuf[3]);
   Serial.print(gpsBuf[4]);
   Serial.println(gpsBuf[5]);

}

void checksumModeDataHandling(char data)
{
   // We are in checksum verification mode already
   if ( (gpsDecodeMode & 0x03) == GPSMODE_CHECKSUM_1)
   {
      uint8_t val = hexToInt(data);
      if (val == 0xff)
      {
         Serial.println("NMEA checksum 1 invalid char");
         gpsDecodeMode = GPSMODE_NEW_LINE;
         gpsBufPos = 0;
         return;
      }

      checksumVerify = val << 4;
      gpsDecodeMode = (gpsDecodeMode & 0xFC) | GPSMODE_CHECKSUM_2;
      return;
   }

   if ( (gpsDecodeMode & 0x03) == GPSMODE_CHECKSUM_2)
   {
      uint8_t val = hexToInt(data);
      if (val == 0xff)
      {
         Serial.println("NMEA checksum 2 invalid char");
         gpsDecodeMode = GPSMODE_NEW_LINE;
         gpsBufPos = 0;
         return;
      }

      checksumVerify |= val;
      if (checksumVerify == checksumVal)
      {
         gpsDecodeMode = GPSMODE_NEW_LINE;

         decodeNmeaMessage();
      }
      else
      {
         Serial.println(" - NMEA Checksum invalid");
      }

      gpsDecodeMode = GPSMODE_NEW_LINE;
      gpsBufPos = 0;
      return;
   }

   /*if ( (gpsDecodeMode & 0x03) == GPSMODE_CHECKSUM_3)
    {
      Serial.println("NMEA checksum too many chars");
      gpsDecodeMode = GPSMODE_NEW_LINE;
      return;
    }*/
}

void processGpsData(char data)
{
   if ( (data == '\n') || (data == '\r') || (data == '$') )
   {
      gpsDecodeMode = GPSMODE_NEW_LINE;
      checksumVal = 0;
      gpsBufPos = 0;
      return;
   }

   // Are we in checksum verification mode already?
   if (gpsDecodeMode & 0x3)
   {
      checksumModeDataHandling(data);
   }
   else
   {
      // We are still receiving / expecting NMEA message body
      if (data == '*')
      {
         // Checksum delimiter, all other bytes will be checksum bytes!
         gpsDecodeMode = (gpsDecodeMode & 0xFC) | GPSMODE_CHECKSUM_1;
         return;
      }

      checksumVal ^= data;

      if (gpsBufPos < GPS_BUF_MAX_SIZE)
      {
         gpsBuf[gpsBufPos] = data;
         gpsBufPos++;
      }

   }
}
