/*******************************************************************************
 * @file    bme_module.cpp
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 18 2023
 ******************************************************************************/
#include <Arduino.h>
#include "bme_module.h"

BME_Module::BME_Module()
{
  status = false;
}

bool BME_Module::begin()
{
  if (bme_sensor.begin(BME_SENSOR_ADDR))
  {
    status = true;
    
    // Set up oversampling and filter initialization
    bme_sensor.setTemperatureOversampling(BME680_OS_8X);
    bme_sensor.setHumidityOversampling(BME680_OS_2X);
    bme_sensor.setPressureOversampling(BME680_OS_4X);
    bme_sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme_sensor.setGasHeater(320, 150);
  }

  return status;
}

String BME_Module::read4sd()
{
  String bms_data_str;



  bms_data_str = String(bme_sensor.temperature) + ",";
  bms_data_str += String(bme_sensor.pressure / 100.0) + ",";
  bms_data_str += String(bme_sensor.humidity) ;
  // bms_data_str += String(bme_sensor.gas_resistance / 1000.0) + ",";
  // bms_data_str += String(bme_sensor.readAltitude(SEALEVELPRESSURE_HPA));

  return bms_data_str;
}

String BME_Module::read4print()
{
  String bms_data_str;

  // if (!status)
  //   return "";

  bms_data_str = "Temp:" + String(bme_sensor.temperature) + " C,";
  bms_data_str += "Pressure:" + String(bme_sensor.pressure / 100.0) + " hPa,";
  bms_data_str += "Humidity:" + String(bme_sensor.humidity) + " %,";
  bms_data_str += "Gas:" + String(bme_sensor.gas_resistance / 1000.0) + " KOhms,";
  bms_data_str += "Altitude:" + String(bme_sensor.readAltitude(SEALEVELPRESSURE_HPA)) + " m";

  return bms_data_str;
}