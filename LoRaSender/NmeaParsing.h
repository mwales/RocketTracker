#ifndef NMEA_PARSING_H
#define NMEA_PARSING_H

#include <stdint.h>


#define GPSMODE_NEW_LINE 0
#define GPSMODE_CHECKSUM_1       1       // We received the *, waiting for checksum bytes
#define GPSMODE_CHECKSUM_2       2       // Received first byte of checksum
#define GPSMODE_CHECKSUM_3       3       // Received second bytes of checksum

#define GPS_BUF_MAX_SIZE  80


extern char gpsBuf[];


uint8_t hexToInt(char data);
char intToHex(uint8_t val);

void nullTerminateGpsBuf();

void decodeNmeaGpggaFixDataMsg();

char* getGpsBufArg(uint8_t index);

void decodeNmeaGpvtgMadeGoodGroundSpeedMsg();

void decodeNmeaGprmcGpsTransitDataMsg();

void decodeNmeaGpgllLatLongMsg();

void decodeNmeaGpgsaDopAndFixMsg();

void decodeNmeaGpgsvSatInViewMsg();

void decodeNmeaMessage();

void checksumModeDataHandling(char data);

void processGpsData(char data);

#endif
