#ifndef ARDUINO

#include "LoRaSender.ino"
#include <stdlib.h>




Ser Serial;

void convertGpsDegressMinutesToDecimal(char* number, char* direction)
{
   uint8_t indexOfDecimal = 0xff;
   for(int i = 0; i < strlen(number); i++)
   {
      if (number[i] == '.')
      {
         indexOfDecimal = i;
         break;
      }
   }

   if (indexOfDecimal == 0xff)
   {
      // No decimal found
      printf("Error");
      return;
   }

   char wholeBuf[8];
   strncpy(wholeBuf, number, 8);
   if (indexOfDecimal < 2)
   {
      printf("Error");
      return;
   }

   wholeBuf[indexOfDecimal - 2] = 0;

   char* retVal = (char*) malloc(100);
   double minutes = strtod(number + indexOfDecimal - 2, NULL);

   double degrees = strtod(wholeBuf, NULL);

   double decimalVal = degrees + minutes / 60.0;

   if ( (direction[0] == 'S') ||
        (direction[0] == 's') ||
        (direction[0] == 'W') ||
        (direction[0] == 'w') )
   {
      decimalVal *= -1.0;
   }

   printf("%g", decimalVal);
}



char const * testData = "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPGSV,3,1,11,16,81,256,24,26,53,034,30,51,46,227,31,04,42,322,19*7E\n"
                        "$GPGSV,3,2,11,03,39,249,23,22,31,224,23,27,31,158,23,31,27,066,27*70\n"
                        "$GPGSV,3,3,11,08,11,184,20,32,10,135,21,09,07,318,18*47\n"
                        "$GPRMC,060046.000,A,2755.6922,N,08032.8222,W,0.08,150.01,080820,,,D*7B\n"
                        "$GPVTG,150.01,T,,M,0.08,N,0.15,K,D*31\n"
                        "$GPGGA,060047.000,2755.6922,N,08032.8221,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*68\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060047.000,A,2755.6922,N,08032.8221,W,0.12,150.01,080820,,,D*72\n"
                        "$GPVTG,150.01,T,,M,0.12,N,0.21,K,D*3D\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060048.000,A,2755.6922,N,08032.8221,W,0.14,150.01,080820,,,D*7B\n"
                        "$GPVTG,150.01,T,,M,0.14,N,0.27,K,D*3D\n"
                        "$GPGGA,060049.000,2755.6921,N,08032.8221,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*65\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060049.000,A,2755.6921,N,08032.8221,W,0.17,150.01,080820,,,D*7A\n"
                        "$GPVTG,150.01,T,,M,0.17,N,0.32,K,D*3A\n"
                        "$GPGGA,060050.000,2755.6921,N,08032.8221,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*6D\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060050.000,A,2755.6921,N,08032.8221,W,0.12,150.01,080820,,,D*77\n"
                        "$GPVTG,150.01,T,,M,0.12,N,0.22,K,D*3E\n"
                        "$GPGGA,060051.000,2755.6921,N,08032.8221,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*6C\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPGSV,3,1,11,16,81,256,24,26,53,034,29,51,46,227,31,04,42,322,19*76\n"
                        "$GPGSV,3,2,11,03,39,249,23,22,31,224,23,27,31,158,23,31,27,066,27*70\n"
                        "$GPGSV,3,3,11,08,11,184,20,32,10,135,19,09,07,318,17*43\n"
                        "$GPRMC,060051.000,A,2755.6921,N,08032.8221,W,0.13,150.01,080820,,,D*77\n"
                        "$GPVTG,150.01,T,,M,0.13,N,0.25,K,D*38\n"
                        "$GPGGA,060052.000,2755.6921,N,08032.8220,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*6E\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060052.000,A,2755.6921,N,08032.8220,W,0.16,150.01,080820,,,D*70\n"
                        "$GPVTG,150.01,T,,M,0.16,N,0.30,K,D*39\n"
                        "$GPGGA,060053.000,2755.6921,N,08032.8220,W,2,08,0.97,14.1,M,-30.6,M,0000,0000*6F\n"
                        "$GPGSA,A,3,03,26,31,22,16,27,32,04,,,,,1.32,0.97,0.90*07\n"
                        "$GPRMC,060053.000,A,2755.6921,N,08032.8220,W,0.08,150.01,080820,,,D*7E\n"
                        "$GPGGA,065657.000,2755.6972,N,08032.8214,W,2,10,0.86,13.6,M,-30.6,M,0000,0000*60\n";

/**
 * I use main function on Linux build to test GPS parsing
 */
int main (int argc, char** argv)
{

   setup();

   printf("Len = %ld\n", strlen(testData));

   for (int i = 0; i < strlen(testData); i++)
   {
      char gpsDat = testData[i];
      Serial.print(gpsDat);
      processGpsData(gpsDat);
   }
}

#endif
