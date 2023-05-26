/*******************************************************************************
 * @file    rtc_module.h
 * @brief   Gets Date and Time from the RTC module.
 *
 * @author 	Rohan Jha
 * @date 	  May 25, 2023
 ******************************************************************************/
#ifndef _RTC_MODULE_H
#define _RTC_MODULE_H
#include <RTClib.h>

class RTC_Module {
  public:
    RTC_Module();
    int begin();
    String getDateTime();

  private:
    RTC_DS3231 rtc;
    bool rtc_status;
};

#endif