#ifndef MISSION_DATA_H
#define MISSION_DATA_H


void mdUpdatePosition(char* lat, char* ns, char* longitude, char* ew);

char* mdGetPosition();

void mdUpdateTimestamp(char* ts);

char* mdGetTimestamp();

void mdUpdateAltitude(char* alt);

char* mdGetAltitude();

void mdUpdateBattery(float batteryLevel);

char* mdGetBattery();

void mdUpdateFix(char* fix, char* pdop, char* hdop, char* vdop);

char* mdGetFix();

void mdUpdateNumSats(char* numSats);

char* mdGetNumSats();

void mdUpdateSpeed(char* speed);

char* mdGetSpeed();



void setupMissionData();

#endif
