#ifndef LORA_COMM_H
#define LORA_COMM_H

#include <stdint.h>
#include <SPI.h>
#include <RH_RF95.h>


#define MSG_HEADER_LEN     5

#define MC_TIMESTAMP_ID    0
#define MC_POSITION_ID     1
#define MC_ALTITUDE_ID     2
#define MC_BATTERY_ID      3
#define MC_GPS_FIX_ID      4
#define MC_NUM_SATS_ID     5
#define MC_SPEED_ID        6
/**
 * LoRa messaging ICD
 *
 * Message Header
 * ==============
 * uint8_t destination ID
 * uint8_t source ID
 * uint8_t msg index
 * uint8_t message type
 * uint8_t checksum
 *
 * Rocket -> Mission Control Messages
 * ==================================
 *
 * 0x00 Timestamp report
 *      HHMMSS
 * 0x01 Position report
 *      llll.llNyyyyy.yyE
 * 0x02 Altitude report
 *      mmmm.m
 * 0x03 Battery level xxx is voltage x 100 (000-100)
 *      xxxV,ppp
 * 0x04 GPS Fix / Precision
 *      F_xxx.x (F 0=None, 1=GPS, 2=DGPS)
 * 0x05 Num Satellites
 *      xx
 * 0x06 Speed Update kmh x 10
 *      xxxx
 * 0x0f RSSI Update (not sent over LoRa)
 *      xxx
 *      
 * 0x10 GPS 2D Fix Aquired
 * 0x11 GPS 3D Fix Acquired
 * 0x12 GPS Lost
 * 0x14 Launch detected
 * 0x15 Apogee detected
 * 0x16 Landing detected
 * 0x17 ASCII
 *
 * Mission Control -> Rocket Messages
 * ==================================
 *
 * 0x20 Enter launch detection mode
 * 0x21 Get position
 *
 */


char* messageTypeToString(uint8_t msgType);

bool loraSendMsg(uint8_t destId, uint8_t sourceId, uint8_t msgType, char* data, bool waitFlag);
bool loraSendMsgLen(uint8_t destId, uint8_t sourceId, uint8_t msgType, char* data, bool waitFlag, uint8_t msgLen);

void loraSetup();

// Sends an unencrypted hello packet.  "hello" debug command
void loraSendHelloPacket();

// Sends an encrypted ASCII hello packet (manually packed). "helenc" debug command
void loraSendEncHelloPacket();

// Sends an encrypted ASCII hello packet (uses loraSendMsg). "hello2" debug command
void loraSendEncHelloPacket2();


#endif
