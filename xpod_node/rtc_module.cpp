/*******************************************************************************
 * @file    rtc_module.cpp
 * @brief   Gets Date and Time from the RTC module.
 *
 * @author 	Rohan Jha
 * @date 	  May 25, 2023
 ******************************************************************************/
 #include <Arduino.h>
 #include "rtc_module.h"

RTC_Module::RTC_Module()
{
  rtc_status = false;
}

int RTC_Module::begin()
{
  if (!rtc.begin())
  {
    rtc_status = false;
    return rtc_status;
  }

  rtc_status = true;
  // May 26, 2023 at 3am you would call:
  // rtc.adjust(DateTime(2023, 6, 7, 17, 15, 0));
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  return rtc_status;
}

String RTC_Module::getDateTime()
{
  String date_time_str;

  
  if (!rtc_status)
    return "\0";

  DateTime rtc_date_time = rtc.now();

  date_time_str += String(rtc_date_time.month()) + "/";
  date_time_str += String(rtc_date_time.day()) + "/";
  date_time_str += String(rtc_date_time.year()) + ",";
  date_time_str += String(rtc_date_time.hour()) + ":";
  date_time_str += String(rtc_date_time.minute()) + ":";
  date_time_str += String(rtc_date_time.second());
  
  // Serial.println(date_time_str);
  delay(100);
  return date_time_str;

}
