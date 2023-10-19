/*******************************************************************************
 * @file    pms_module.h
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 18 2023
 ******************************************************************************/
#ifndef _PMS_MODULE_H
#define _PMS_MODULE_H

#include <Adafruit_PM25AQI.h>

#define PMS_SERIAL       (Serial1)
#define PMS_SERIAL_BR    (9600)

class PMS_Module
{
  public:
    PMS_Module();
    bool begin();
    String read4sd();
    String read4print();

  private:
    Adafruit_PM25AQI pms_sensor;
    bool status;
};

#endif  //_PMS_MODULE_H