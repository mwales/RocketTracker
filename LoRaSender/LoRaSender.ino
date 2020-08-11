#include <SPI.h>
#include <LoRa.h>
#include <stdint.h>

// uncomment the section corresponding to your board
// BSFrance 2017 contact@bsrance.fr 

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
  #define LORA_IDLE_0 3

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
  

#define GPSSerial Serial1

#define GPSMODE_NEW_LINE 0
#define GPSMODE_CHECKSUM_1       1       // We received the *, waiting for checksum bytes
#define GPSMODE_CHECKSUM_2       2       // Received first byte of checksum
#define GPSMODE_CHECKSUM_3       3       // Received second bytes of checksum

#define GPS_BUF_MAX_SIZE  40
char gpsBuf[GPS_BUF_MAX_SIZE];
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
    Serial.println("Fix Data Msg");
    return;
  }

  if (strncmp("GPVTG",gpsBuf, 5) == 0)
  {
    Serial.println("Track Made Good Msg");
    return;
  }
  
  if (strncmp("GPRMC",gpsBuf, 5) == 0)
  {
    Serial.println("GPS Transit Data");
    return;
  }

  if (strncmp("GPGLL",gpsBuf, 5) == 0)
  {
    Serial.println("Geo Pos Lat Long Msg");
    return;
  }

  if (strncmp("GPGSA",gpsBuf, 5) == 0)
  {
    Serial.println("DOP and Satellite Message");
    return;
  }

  if (strncmp("GPGSV",gpsBuf, 5) == 0)
  {
    Serial.println("Sat View Message");
    return;
  }

  if (strncmp("GPHDT",gpsBuf, 5) == 0)
  {
    Serial.println("True Heading");
    return;
  }

  if (strncmp("GPZDA",gpsBuf, 5) == 0)
  {
    Serial.println("Date And Time");
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
  Serial.print(gpsBuf[5]);
  
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


void setup() {
  Serial.begin(115200);
  GPSSerial.begin(9600);
  
  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND,PABOOST)) {
    Serial.println("Starting LoRa failed!");
    while (1);
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
