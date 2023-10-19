/*******************************************************************************
 * @file    bme_module.h
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 18 2023
 ******************************************************************************/
#ifndef _BME_MODULE_H
#define _BME_MODULE_H

#include <Adafruit_BME680.h>

#define BME_SENSOR_ADDR       (0x76)
#define SEALEVELPRESSURE_HPA  (1013.25)

class BME_Module
{
  public:
    BME_Module();
    bool begin();
    String read4sd();
    String read4print();

  private:
    Adafruit_BME680 bme_sensor;
    bool status;
};

#endif  //_BME_MODULE_H