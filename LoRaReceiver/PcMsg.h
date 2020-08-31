#ifndef PC_MSG_H
#define PC_MSG_H

#include <stdint.h>

void sendRssiMsgToPc(uint8_t rssi);

void sendMsgToPc(uint8_t msgType, uint8_t len, uint8_t* payload);

#endif
