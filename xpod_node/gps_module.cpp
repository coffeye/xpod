/*******************************************************************************
 * @file    gps_module.cpp
 * @brief   Gets Location,Date and Time from the GPS module.
 *
 * @author 	Rohan Jha
 * @date 	  May 25, 2023
 ******************************************************************************/
#include "gps_module.h"

#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin

SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX);

GPS_Module::GPS_Module()
{
  gps_status = false;
}

int GPS_Module::begin()
{
  ssGPS.begin(GPS_BAUDRATE);

  if (millis() > 5000 && tinyGps.charsProcessed() < 10)
{
  Serial.println("No GPS detected: check wiring.");
  return gps_status;
}
  gps_status = true;
  return gps_status;
}

String GPS_Module::get_gps_info()
{
  String gps_data;

  if (!gps_status)
    return "\0";
  // Print latitude, longitude, altitude in feet, course, speed, date, time,
  // and the number of visible satellites.
  char c = Serial.read();
  while (ssGPS.available() > 0)
    tinyGps.encode(ssGPS.read());
  if (tinyGps.location.isValid())
  {
    gps_data+= ("LAT:" + String(tinyGps.location.lat()));
    gps_data+=("Long:" + String(tinyGps.location.lng()));
  }
  else
  {
    Serial.println("INVALID LOCATION");
  }
  gps_data+=("Alt:"+ String(tinyGps.altitude.feet()));
  gps_data+=("Course:"+String(tinyGps.course.deg()));
  gps_data+=("Speed:"+ String(tinyGps.speed.mph()));
  gps_data+=("Sats:"+ String(tinyGps.satellites.value()));
  delay(100);
  
  return gps_data;

}
