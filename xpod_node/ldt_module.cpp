/*******************************************************************************
 * @file    ldt_module.cpp
 * @brief   Gets Time and Date information from MKR GPS shield over UART.
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 10 2023
 *
 * @change  Feb 14 2023, Fixed issue while getting date and time data from the 
 *          GPS module.
 * @change  Feb 18 2023, RTC module added.
 ******************************************************************************/
#include <Arduino.h>
#include "ldt_module.h"

LDT_Module::LDT_Module()
{
  gps_status = false;
  rtc_status = false;
}

int LDT_Module::gpsBegin()
{
  GPS_SERIAL.begin(GPS_BAUDRATE);

  if (gps.date.isValid())
    gps_status = true;

  return gps_status;
}

int LDT_Module::rtcBegin()
{
  if (rtc.begin())
  {
    rtc_status = true;
  }
  else
  {
    rtc_status = false;
    return false;
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  return rtc_status;
}

String LDT_Module::getGpsDateTime()
{
  String date_time_str;

  if (!gps_status)
    return date_time_str;

  TinyGPSDate gps_date = gps.date;
  TinyGPSTime gps_time = gps.time;

  date_time_str += String(gps_date.month()) + "/";
  date_time_str += String(gps_date.day()) + "/";
  date_time_str += String(gps_date.year()) + ",";
  date_time_str += String(gps_time.hour()) + ":";
  date_time_str += String(gps_time.minute()) + ":";
  date_time_str += String(gps_time.second());
  smartDelay(0);

  return date_time_str;
}

String LDT_Module::getRtcDateTime()
{
  String date_time_str;

  if (!rtc_status)
    return date_time_str;

  DateTime rtc_date_time = rtc.now();

  date_time_str += String(rtc_date_time.month()) + "/";
  date_time_str += String(rtc_date_time.day()) + "/";
  date_time_str += String(rtc_date_time.year()) + ",";
  date_time_str += String(rtc_date_time.hour()) + ":";
  date_time_str += String(rtc_date_time.minute()) + ":";
  date_time_str += String(rtc_date_time.second());

  return date_time_str;
}

void LDT_Module::smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPS_SERIAL.available())
      gps.encode(GPS_SERIAL.read());
  } while (millis() - start < ms);
}