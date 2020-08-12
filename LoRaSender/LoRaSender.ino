
#include <stdint.h>

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

void decodeNmeaGpggaFixDataMsg()
{
   nullTerminateGpsBuf();

   char* altitude = getGpsBufArg(9);
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
      Serial.print("  No fix: ");
      return;
   }
   if (fixArg[0] == '1')
   {
      Serial.println("  GPS fix");
   }
   if (fixArg[0] == '2')
   {
      Serial.println("  DGPS fix");
   }

   Serial.print("  LAT: ");
   Serial.print(lat1);
   Serial.println(lat2);

   Serial.print("  LNG: ");
   Serial.print(lng1);
   Serial.println(lng2);

   Serial.print("  ALT: ");
   Serial.println(altitude);
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

   Serial.print("  SPD: ");
   Serial.print(speedKmh);
   Serial.println(" kmh");
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

   Serial.print("  LAT: ");
   Serial.print(lat1);
   Serial.println(lat2);

   Serial.print("  LNG: ");
   Serial.print(lng1);
   Serial.println(lng2);

   Serial.print("  SPD: ");
   Serial.print(groundSpeedKnots);
   Serial.println(" knots");

   //printf("geo:");
   //convertGpsDegressMinutesToDecimal(lat1,lat2);
   //printf(",");
   //convertGpsDegressMinutesToDecimal(lng1,lng2);
   //printf("\n");

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

   Serial.print("  LAT: ");
   Serial.print(lat1);
   Serial.println(lat2);

   Serial.print("  LNG: ");
   Serial.print(lng1);
   Serial.println(lng2);

   //printf("geo:");
   //convertGpsDegressMinutesToDecimal(lat1,lat2);
   //printf(",");
   //convertGpsDegressMinutesToDecimal(lng1,lng2);
   //printf("\n");
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

   if (fixArg[0] == '0')
   {
      Serial.print("  No fix: ");
      return;
   }
   if (fixArg[0] == '1')
   {
      Serial.println("  GPS fix");
   }
   if (fixArg[0] == '2')
   {
      Serial.println("  DGPS fix");
   }

   Serial.print("  PDOP: ");
   Serial.println(pdop);

   Serial.print("  HDOP: ");
   Serial.println(hdop);

   Serial.print("  VDOP: ");
   Serial.println(vdop);
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

   Serial.print("  #SAT: ");
   Serial.println(satsInView);
}

void decodeNmeaMessage()
{

   Serial.print("\nValid: ");

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
      Serial.println("Bearing origin to dest msg - WTF");
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


/*
void smNewLineMode()
{
  if (gpsBufPos >= 1)
  {
    // First character better be a G
    if (gpsBuf[0] != 'G')
    {
      gpsBufPos = 0;
      return;
    }
  }

  if (gpsBufPos >= 5)
  {
    // We have start of sentence!
  }
}

void doGpsStateMachine()
{
  switch(gpsDecodeMode)
  {
    case GPSMODE_NEW_LINE:


}

void processGps()
{
  while(GPSSerial.available())
  {
    gpsBuf[gpsBufPos++] = GPSSerial.read();

    doGpsStateMachine();
  }
}


      Serial.print(gpsData);

      if ( (gpsData == '\n') || (gpsData == '\r') )
      {
        newGpsLine = true;
      }
    }
}

*/



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
  
  //checkLoraState();

  //Serial.print(".");
  
  // send packet
  //sendHelloPacket();
  //delay(100);
}
