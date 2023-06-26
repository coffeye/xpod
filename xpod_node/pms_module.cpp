/*******************************************************************************
 * @file    bme_module.cpp
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 18 2023
 ******************************************************************************/
#include <Arduino.h>
#include "pms_module.h"

PMS_Module::PMS_Module()
{
  status = false;
}

bool PMS_Module::begin()
{
  PMS_SERIAL.begin(9600);

  if (!pms_sensor.begin_UART(&PMS_SERIAL))
    status = false;
  else
    status = true;

  return status;
}

String PMS_Module::read4sd()
{
  String pms_data_str;
  PM25_AQI_Data data;
  int read_tries = 3;

  
  while(!pms_sensor.read(&data) && read_tries) {
    read_tries--;
  }


  pms_data_str = String(data.pm10_env) + ",";
  pms_data_str += String(data.pm25_env) + ",";
  pms_data_str += String(data.pm100_env) + ",";

  pms_data_str += String(data.particles_03um) + ",";
  pms_data_str += String(data.particles_05um) + ",";
  pms_data_str += String(data.particles_10um) + ",";
  pms_data_str += String(data.particles_25um) + ",";
  pms_data_str += String(data.particles_50um) + ",";
  pms_data_str += String(data.particles_100um);
  delay(100);
  return pms_data_str;
}

String PMS_Module::read4print()
{
  String pms_data_str;
  PM25_AQI_Data data;
  int read_tries = 3;

  if (!status)
    return "";

    while(!pms_sensor.read(&data) && read_tries) {
    read_tries--;
  }
  pms_data_str = "PM10_ENV:" + String(data.pm10_env) + ",";
  pms_data_str += "PM10_ENV:" + String(data.pm25_env) + ",";
  pms_data_str += "PM10_ENV:" + String(data.pm100_env) + ",";

  pms_data_str += "PM_03um:" + String(data.particles_03um) + ",";
  pms_data_str += "PM_05um:" + String(data.particles_05um) + ",";
  pms_data_str += "PM_10um:" + String(data.particles_10um) + ",";
  pms_data_str += "PM_25um:" + String(data.particles_25um) + ",";
  pms_data_str += "PM_30um:" + String(data.particles_50um) + ",";
  pms_data_str += "PM_100um:" + String(data.particles_100um);
  delay(100);
  return pms_data_str;
}