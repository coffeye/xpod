/*******************************************************************************
 * @file    gps_module.h
 * @brief   Gets Time and Date information from MKR GPS shield over UART.
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 10 2023
 *
 * @change  Feb 14 2023, Fixed issue while getting date and time data from the 
            GPS module.
 ******************************************************************************/
#ifndef _GPS_MODULE_H
#define _GPS_MODULE_H

#include <TinyGPSPlus.h>
#include "xpod_node.h"

#define GPS_SERIAL    (Serial3)
#define GPS_BAUDRATE  (9600)
#define CURRENT_YEAR  (2023)

class GPS_Module {
  public:
    GPS_Module();
    void getGpsDate(char* date_str);
    void getGpsTime(char* time_str);
    void getGpsDateTime(char* str);

  private:
    void smartDelay(unsigned long ms);
    TinyGPSPlus gps;
    bool gps_locked;
};

#endif  //_GPS_MODULE_H