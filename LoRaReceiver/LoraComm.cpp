#include "LoraComm.h"
#include "Arduino.h"
#include "PcMsg.h"

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

uint8_t obfuscatorPattern[] = { 0x33, 0xAA, 0x3A, 0xA3,
                                0x55, 0xCC, 0x5C, 0xC5,
                                0x99, 0x95, 0x59, 0x39,
                                0x93, 0x66, 0xA9, 0x9A};

uint8_t counter = 0;

void obfuscateData(char* dest, char* src, uint8_t msgIndex, uint8_t msgLen)
{
   uint8_t patternIndex = msgIndex;
   for(uint8_t i = 0; i < msgLen; i++)
   {
      // Serial.print("Obfuscating a byte with ");
      // Serial.println(obfuscatorPattern[patternIndex % 16]);
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
    case 7:
      return "Peak Altitude";
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

bool loraSendMsg(uint8_t destId, uint8_t sourceId, uint8_t msgType, char* data, bool waitFlag)
{
   Serial.print("Sending ");
   Serial.print(messageTypeToString(msgType));
   
   
   uint8_t msgLen = (uint8_t) (strlen(data) & 0xff);
   if (msgLen > 20)
   {
      Serial.print("Sending too long of a ");
      Serial.print(messageTypeToString(msgType));
      Serial.print(" message ");
      Serial.println(msgLen);
      return false;
   }

   Serial.print(" (");
   Serial.print(msgLen);
   Serial.println(" bytes)");

   char msgBuf[32];
   memset(msgBuf, 0, 32);

   char myCookie = '?';
   
   msgBuf[0] = destId;
   msgBuf[1] = sourceId;
   msgBuf[2] = counter++;
   msgBuf[3] = msgType;
   msgBuf[4] = computeChecksum(data, msgLen);

   obfuscateData(msgBuf + 5, data, msgBuf[2], msgLen);

   // RH_RF95::printBuffer("BUF: ", (uint8_t*) msgBuf, msgLen + 5);
   rf95.send( (uint8_t*) msgBuf, msgLen + 5);

   if (waitFlag)
   {
      rf95.waitPacketSent();
   }

   return true;
}

bool loraSendMsgLen(uint8_t destId, uint8_t sourceId, uint8_t msgType, char* data, bool waitFlag, uint8_t msgLen)
{
   Serial.print("Sending ");
   Serial.print(messageTypeToString(msgType));
   
   if (msgLen > 20)
   {
      Serial.print("Sending too long of a ");
      Serial.print(messageTypeToString(msgType));
      Serial.print(" message ");
      Serial.println(msgLen);
      return false;
   }

   Serial.print(" (");
   Serial.print(msgLen);
   Serial.print(" bytes)");

   char msgBuf[32];
   memset(msgBuf, 0, 32);

   char myCookie = '?';
   
   msgBuf[0] = destId;
   msgBuf[1] = sourceId;
   msgBuf[2] = counter++;
   msgBuf[3] = msgType;
   msgBuf[4] = computeChecksum(data, msgLen);

   obfuscateData(msgBuf + 5, data, msgBuf[2], msgLen);

   RH_RF95::printBuffer("BUF: ", (uint8_t*) msgBuf, msgLen + 5);
   rf95.send( (uint8_t*) msgBuf, msgLen + 5);

   if (waitFlag)
   {
      rf95.waitPacketSent();
   }

   Serial.print(" - done (cookie =");
   Serial.print(myCookie);
   Serial.println(")");

   RH_RF95::printBuffer("FullBuf: ", (uint8_t*) msgBuf, 32);
}


void loraSendHelloPacket()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  char msgBuf[20];
  sprintf(msgBuf, "hello %d", counter);
  rf95.send( (uint8_t*) msgBuf, strlen(msgBuf) + 1);
  rf95.waitPacketSent();

  counter++;
}

void loraSendEncHelloPacket()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  char msgBuf[20];
  sprintf(msgBuf, "hello %d", counter);
  
  char encBuf[20];

   encBuf[0] = 'h';
   encBuf[1] = 'i';
   encBuf[2] = counter;
   encBuf[3] = 0x17;  // ASCII type
   encBuf[4] = computeChecksum(msgBuf, strlen(msgBuf)+1);
  
   obfuscateData(encBuf + 5, msgBuf, counter % 16, strlen(msgBuf)+1);
  
  rf95.send( (uint8_t*) encBuf, strlen(msgBuf) + 6);
  rf95.waitPacketSent();

  counter++;
}

void loraSendEncHelloPacket2()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  char msgBuf[20];
  sprintf(msgBuf, "hello %d", counter);

  loraSendMsg('e', 't', 0x17, msgBuf, true);
  counter++;
}

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




// This version uses larger buffer length
void loraReceiveMessage()
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
}

void processMsg(uint8_t* ctBuf, uint8_t len, uint8_t rssi)
{
  Serial.println("\n\n");
  Serial.print("** New Packet **  RSSI=");
  Serial.println(rssi);
  
  //digitalWrite(LED, HIGH);
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
  //uint8_t data[] = "And hello back to you";
  //rf95.send(data, sizeof(data));
  //rf95.waitPacketSent();
  //Serial.println("Sent a reply");
  //digitalWrite(LED, LOW);

  sendMsgToPc(ctBuf[3], len-5, decMsg);
  sendRssiMsgToPc(rssi);
}
