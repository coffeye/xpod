/*******************************************************************************
 * @file    gps_module.cpp
 * @brief   Gets Time and Date information from MKR GPS shield over UART.
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 10 2023
 *
 * @change  Feb 14 2023, Fixed issue while getting date and time data from the 
            GPS module.
 ******************************************************************************/
#include <Arduino.h>
#include "gps_module.h"

GPS_Module::GPS_Module()
{
    GPS_SERIAL.begin(GPS_BAUDRATE);
    gps_locked = false;
}

void GPS_Module::getGpsDate(char *date_str)
{
  TinyGPSDate gps_date = gps.date;

  if (!gps_date.isValid()) {
    *date_str = '\0';
  }
  else {
    if (gps_date.year() == CURRENT_YEAR)
      gps_locked = true;
    else
      gps_locked = false;

    if (gps_locked)
      sprintf(date_str, "%02d/%02d/%02d ", gps_date.month(), gps_date.day(), gps_date.year());
    else
      *date_str = '\0';
  }

  smartDelay(0);
}

void GPS_Module::getGpsTime(char* time_str)
{
  TinyGPSTime gps_time = gps.time;

  if (!gps_time.isValid()) {
    *time_str = '\0';
  }
  else {
    if (gps_locked)
      sprintf(time_str, "%02d:%02d:%02d ", gps_time.hour(), gps_time.minute(), gps_time.second());
    else
      *time_str = '\0';
  }

  smartDelay(0);
}

void GPS_Module::getGpsDateTime(char* str)
{
  char gps_date[12];
  char gps_time[12];

  this->getGpsDate(gps_date);
  this->getGpsTime(gps_time);

  if (gps_locked) {
    *str = '\0';
  }
  else {
    strcpy(str, gps_date);
    strcat(str, ",");
    strcat(str, gps_time);
  }  
}

void GPS_Module::smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}