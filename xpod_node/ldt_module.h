/*******************************************************************************
 * @file    ldt_module.h
 * @brief   Gets Time and Date information from MKR GPS shield over UART.
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 10 2023
 *
 * @change  Feb 14 2023, Fixed issue while getting date and time data from the 
 *          GPS module.
 * @change  Feb 18 2023, RTC module added.
 ******************************************************************************/
#ifndef _LDT_MODULE_H
#define _LDT_MODULE_H

#include <TinyGPSPlus.h>
#include <RTClib.h>
#include "xpod_node.h"

#define GPS_SERIAL    (Serial3)
#define GPS_BAUDRATE  (9600)
#define CURRENT_YEAR  (2023)

class LDT_Module {
  public:
    LDT_Module();
    int gpsBegin();
    int rtcBegin();
    String getGpsDateTime();
    String getRtcDateTime();

  private:
    void smartDelay(unsigned long ms);
    
    TinyGPSPlus gps;
    RTC_DS3231 rtc;
    bool gps_status;
    bool rtc_status;
};

#endif  //_LDT_MODULE_H