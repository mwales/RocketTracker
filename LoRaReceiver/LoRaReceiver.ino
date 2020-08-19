/**
 * Receiver portion of the rocket tracker.  Started out as Feather9x_RX example code
 * (I think from Adafruit)
 */

#include <stdint.h>
#include <SPI.h>
#include "RH_RF95.h"
 
// for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

 
/* for feather m0 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
*/

 
 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
 
// Blinky on receipt
#define LED 13




/**
 * Obfuscator pattern must be the exact same as transmitter.
 */

uint8_t obfuscatorPattern[] = { 0x33, 0xAA, 0x3A, 0xA3,
                                0x55, 0xCC, 0x5C, 0xC5,
                                0x99, 0x95, 0x59, 0x39,
                                0x93, 0x66, 0xA9, 0x9A};

void obfuscateData(uint8_t* dest, uint8_t* src, uint8_t msgIndex, uint8_t msgLen)
{
   uint8_t patternIndex = msgIndex;
   for(uint8_t i = 0; i < msgLen; i++)
   {
      dest[i] = src[i] ^ obfuscatorPattern[patternIndex % 16];
      patternIndex++;
   }
}

uint8_t computeChecksum(char* buf, uint8_t bufLen)
{
   uint8_t cs = 0;
   for(int i = 0; i < bufLen; i++)
   {
      cs ^= buf[i];
   }

   return cs;
}


 
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  delay(100);
 
  Serial.println("Feather LoRa RX Test!");

  Serial.print("RST = ");
  Serial.println(RFM95_RST);
  Serial.print("CS = ");
  Serial.println(RFM95_CS);
  Serial.print("INT = ");
  Serial.println(RFM95_INT);
  
 
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

char* messageTypeToString(uint8_t msgType)
{
  switch(msgType)
  {
    case 0:
      return "Timestamp";
    case 1:
      return "Position";
    case 2:
      return "Altitude";
    case 3:
      return "Battery";
    case 4:
      return "GPS Fix";
    case 5:
      return "Num Sats";
    case 6:
      return "Speed";
    default:
      return "Unknown";  
    
  }
}
 
void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t decBuf[25];
    uint8_t len = sizeof(buf);
 
    if (rf95.recv(buf, &len))
    {
      Serial.println("\n\n");
      Serial.println("** New Packet **");
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);    

      
      Serial.print("Got: ");
      Serial.println((char*)buf);

      if (len < 5)
      {
        Serial.println("Message missing header!");
      }
      else if (len > 25)
      {
        Serial.println("Message too large for mission control");
      }
      else
      {
        // Print decrypted message
        Serial.print("DestID=");
        Serial.println((char) buf[0]);
        
        Serial.print("SrcID=");
        Serial.println((char) buf[1]);

        Serial.print("MsgIndex=");
        Serial.println(buf[2]);

        Serial.print("MsgType=");
        Serial.print(buf[3]);
        Serial.print("=");
        Serial.println(messageTypeToString(buf[3]));

        Serial.print("Checksum Provided=");
        Serial.print(buf[4]);

        

        uint8_t decMsg[25];
        obfuscateData(decMsg, (uint8_t*) &buf[5], buf[2], len - 5);

        Serial.print("   Calculated=");
        Serial.println( (int) computeChecksum( decMsg, len-5));
        
        decMsg[len - 5] = 0;  // null terminate

        Serial.print("DecMsg=");
        Serial.println((char*)decMsg);

        

        RH_RF95::printBuffer("DecMsg: ", decMsg, len-5);
      }

      
       Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
 
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
