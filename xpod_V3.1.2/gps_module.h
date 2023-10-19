/*******************************************************************************
 * @file    gps_module.h
 * @brief   Gets location from GPS Module.
 *
 * @author 	Rohan Jha
 * @date 	  May 25, 2023
 ******************************************************************************/
#ifndef _GPS_MODULE_H
#define _GPS_MODULE_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

#define GPS_BAUDRATE  (9600) 

class GPS_Module {
  public:
    GPS_Module();
    int begin();
    String get_gps_info();
    String get_gps_dtinfo();
    String get_gps_info_sd();
    String get_gps_info_serial();
  private:

    bool gps_status;
    TinyGPSPlus tinyGps;
};

#endif 