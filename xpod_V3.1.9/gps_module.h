/*******************************************************************************
 * @file    gps_module.h
 * @brief   GPS Header file declares required class methods and variables
 *
 * @author  Malola Simman Srinivasan Kannan , masr4788@colorado.edu
 * @date    September 8 2023
 ******************************************************************************/
#ifndef GPSMODULE_H
#define GPSMODULE_H

#include <SoftwareSerial.h>
#include <Arduino.h>
typedef struct gps_val{
   double latitude;
  double longitude;
  double course;
  double speed;
  double altitude;
}gps;
class GPSModule {
public:
  GPSModule();
  bool begin(); // Remove the parameter from begin method
  String readData();
  String parseNMEASentence(const char* sentence);
  bool isDataAvailable();
  String read4blynk();
  double latitude;
  double longitude;
  double course;
  double speed;
  double altitude;
  int day;
  int month;
  int year;
  int hours;
  int min;
  int sec;

private:
  SoftwareSerial gpsSerial;
  char strGPSData[1024];
  bool gps_status;
};

#endif