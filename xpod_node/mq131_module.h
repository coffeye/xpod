/*******************************************************************************
 * @file    mq131_module.ch
 * @brief   
 *
 * @cite    miguel5612, https://github.com/miguel5612/MQSensorsLib
 *
 * @editor  Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 21 2023
 ******************************************************************************/
#ifndef _MQ131_Module_H
#define _MQ131_Module_H

#include <Adafruit_ADS1X15.h>

#define READ_JUST_RAW     1

#define ADC_RESOLUTION    16
#define VOLT_RESOLUTION   5
#define RATIO_CLEAN_AIR   15
#define O3_EXP_REG_A      23.943
#define O3_EXP_REG_B      -1.11
#define O3_EXP_REG_RL     10
#define REG_METHOD        1
#define MQ131_I2C_ADDR    0x4B
#define MQ131_I2C_CHL     1

class MQ131_Module
{
  public:
    MQ131_Module();
    bool begin();

    float read();
    String read4sd();
    String read4print();

  private:
    float calibrate();
    float update();

    Adafruit_ADS1115 ads_module;
    float heater_R0;
    uint16_t raw_data;
    bool status;
};

#endif  //_MQ131_Module_H