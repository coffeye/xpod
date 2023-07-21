/*******************************************************************************
 * @file    mq131_module.cpp
 * @brief   
 *
 * @cite    miguel5612, https://github.com/miguel5612/MQSensorsLib
 *
 * @editor  Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 21 2023
 ******************************************************************************/
#include <Arduino.h>
#include "mq131_module.h"

MQ131_Module::MQ131_Module()
{
  heater_R0 = 0;
  status = false;
}

bool MQ131_Module::begin()
{
  if (!ads_module.begin(MQ131_I2C_ADDR))
    return false;
  else
    status = true;

#if !READ_JUST_RAW
  float calcR0 = 0;
  for(int i = 1; i<=10; i++)
  {
    calcR0 += this->calibrate();
  }

  heater_R0 = calcR0 / 10;
#endif

  return status;
}


String MQ131_Module::read4sd()
{
  String mq_data;

  if (!status)
    return "";

#if READ_JUST_RAW
  mq_data = String(ads_module.readADC_SingleEnded(MQ131_I2C_CHL));
#else
  mq_data = String(this->read()) + "," + String(raw_data);
#endif

  return mq_data;
}

String MQ131_Module::read4print()
{
  String mq_data;

  if (!status)
    return ",";

#if READ_JUST_RAW 
  mq_data = "MQ131: " + String(ads_module.readADC_SingleEnded(MQ131_I2C_CHL));
#else
  mq_data = "MQ131: " + String(this->read()) + "," + String(raw_data);
#endif

  return mq_data;
}

float MQ131_Module::read()
{
  float rs_calc, ratio, PPM;
  float sensor_volt = this->update();

  //More explained in: https://jayconsystems.com/blog/understanding-a-gas-sensor
  rs_calc = ((VOLT_RESOLUTION * O3_EXP_REG_RL) / sensor_volt) - O3_EXP_REG_RL; //Get value of RS in a gas

  //No negative values accepted.
  if(rs_calc < 0)
    rs_calc = 0;

  // Get ratio RS_air/RS_gas <- INVERTED for MQ-131 issue 28 https://github.com/miguel5612/MQSensorsLib/issues/28
  ratio = heater_R0 / rs_calc;

  //No negative values accepted or upper datasheet recomendation.
  if(ratio <= 0)
    ratio = 0;

  // <- Source excel analisis https://github.com/miguel5612/MQSensorsLib_Docs/tree/master/Internal_design_documents
  if(REG_METHOD == 1)
  {
    PPM = O3_EXP_REG_A * pow(ratio, O3_EXP_REG_B);
  }
  else 
  {
    // https://jayconsystems.com/blog/understanding-a-gas-sensor <- Source of linear ecuation
    double ppm_log = (log10(ratio) - O3_EXP_REG_B) / O3_EXP_REG_A; //Get ppm value in linear scale according to the the ratio value  
    PPM = pow(10, ppm_log); //Convert ppm value to log scale  
  }

  //No negative values accepted or upper datasheet recomendation.
  if(PPM < 0)
    PPM = 0;

  //if(_PPM > 10000) _PPM = 99999999; //No negative values accepted or upper datasheet recomendation.
  return PPM;
}

float MQ131_Module::calibrate()
{
  //More explained in: https://jayconsystems.com/blog/understanding-a-gas-sensor
  /*
  V = I x R 
  VRL = [VC / (RS + RL)] x RL 
  VRL = (VC x RL) / (RS + RL) 
  Así que ahora resolvemos para RS: 
  VRL x (RS + RL) = VC x RL
  (VRL x RS) + (VRL x RL) = VC x RL 
  (VRL x RS) = (VC x RL) - (VRL x RL)
  RS = [(VC x RL) - (VRL x RL)] / VRL
  RS = [(VC x RL) / VRL] - RL
  */
  float R0; //Define variable for R0
  float sensor_volt = this->update();

  float RS_air = ((VOLT_RESOLUTION * O3_EXP_REG_RL) / sensor_volt) - O3_EXP_REG_RL; //Calculate RS in fresh air

  if(RS_air < 0)
    RS_air = 0; //No negative values accepted.

  R0 = RS_air / RATIO_CLEAN_AIR; //Calculate R0 

  if(R0 < 0)
    R0 = 0; //No negative values accepted.

  return R0;
}

float MQ131_Module::update()
{
  int retries = 2;
  float avg = 0.0;
  uint32_t adc = 0;

  for (int i = 0; i < retries; i++)
  {
    avg += ads_module.readADC_SingleEnded(MQ131_I2C_CHL);
    delay(20);
  }

  avg = avg / retries;

  raw_data = avg;

  return ((avg * VOLT_RESOLUTION) / ((pow(2, ADC_RESOLUTION) - 1)));
}
