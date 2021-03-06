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

uint8_t computeChecksum(uint8_t* buf, uint8_t bufLen)
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
    case 0x10:
      return "Event GPS 2D Fix Acquired";
    case 0x11:
      return "Event GPS 3d Fix Acquired";
    case 0x12:
      return "Event GPS Lost";
    case 0x14:
      return "Launch Detected";
    case 0x15:
      return "Apogee Detected";
    case 0x16:
      return "Landing Detected";
    case 0x17:
      return "ASCII Msg";
    case 0x20:
      return "CMD Enter Launch Detect Mode";
    case 0x21:
      return "CMD Get Position";
    default:
      return "Unknown";  
    
  }
}

void printHexDigit(uint8_t data)
{
  if (data < 16)
  {
    Serial.print("0");
  }
  Serial.print((int) data, HEX);
}

void sendMsgToPc(uint8_t msgType, uint8_t len, uint8_t* payload)
{
  Serial.print(">");
  printHexDigit(msgType);
  for(uint8_t i = 0; i < len; i++)
  {
    printHexDigit(payload[i]);
  }
  Serial.println("!");
}

void sendRssiMsgToPc(uint8_t rssi)
{
  char rssiMsg[5];
  snprintf(rssiMsg, 5, "%03d", rssi);
  sendMsgToPc(0x0f, strlen(rssiMsg), rssiMsg);
}

void processMsg(uint8_t* ctBuf, uint8_t len, uint8_t rssi)
{
  Serial.println("\n\n");
  Serial.print("** New Packet **  RSSI=");
  Serial.println(rssi);
  
  digitalWrite(LED, HIGH);
  //RH_RF95::printBuffer("CT Received: ", ctBuf, len);    
  
  
  // Serial.print("Got: ");
  // Serial.println((char*)buf);
  
  if (len < 5)
  {
    Serial.println("Message missing header!");
    return;
  }
  
  if (len > 25)
  {
    Serial.println("Message too large for mission control");
    return;
  }

  uint8_t decMsg[25];
  obfuscateData(decMsg, (uint8_t*) &ctBuf[5], ctBuf[2], len - 5);
  uint8_t csCalc = computeChecksum( decMsg, len-5);
  
  //Serial.print("Checksum Provided=");
  //Serial.print(ctBuf[4]);
  //Serial.print("   Calculated=");
  //Serial.print( (int) csCalc);

  if (csCalc != ctBuf[4])
  {
    Serial.println("Checksum INVALID!");
    return;
  }
  
  Serial.println("Checksum OK");

  // Print decrypted message
  // Serial.print("DestID=");
  // Serial.println((char) ctBuf[0]);
  
  //Serial.print("SrcID=");
  //Serial.println((char) ctBuf[1]);

  //Serial.print("MsgIndex=");
  //Serial.println(ctBuf[2]);

  Serial.print("MsgType=");
  //Serial.print(ctBuf[3]);
  Serial.print("=");
  Serial.println(messageTypeToString(ctBuf[3]));  
  
  decMsg[len - 5] = 0;  // null terminate

  Serial.print("DecMsg=");
  Serial.println((char*)decMsg);  

  RH_RF95::printBuffer("PT: ", decMsg, len-5);
 
  // Send a reply
  uint8_t data[] = "And hello back to you";
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
  Serial.println("Sent a reply");
  digitalWrite(LED, LOW);

  sendMsgToPc(ctBuf[3], len-5, decMsg);
  sendRssiMsgToPc(rssi);
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
      uint8_t rssi = rf95.lastRssi();
      processMsg(buf, len, rssi);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }

  delay(5);
  //Serial.print(".");
}
