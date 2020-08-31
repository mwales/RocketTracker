#include "PcMsg.h"
#include "Arduino.h"


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
