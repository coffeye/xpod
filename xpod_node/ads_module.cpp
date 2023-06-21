/*******************************************************************************
 * @file    ads_module.cpp
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 18 2023
 ******************************************************************************/
#include "ads_module.h"

ADS_Module::ADS_Module()
{
  ads_module[ADS_SENSOR_FIG2600].addr = 0x48;
  ads_module[ADS_SENSOR_FIG2600].channel = 3;

  ads_module[ADS_SENSOR_FIG2602].addr = 0x49;
  ads_module[ADS_SENSOR_FIG2602].channel = 2;

#if FIGARO3_ENABELD
  ads_module[ADS_SENSOR_FIG3].addr = 0x48;
  ads_module[ADS_SENSOR_FIG3].channel = 0;
#endif

#if FIGARO4_ENABELD
  ads_module[ADS_SENSOR_FIG4].addr = 0x49;
  ads_module[ADS_SENSOR_FIG4].channel = 0;
#endif

  ads_module[ADS_SENSOR_PID].addr = 0x48;
  ads_module[ADS_SENSOR_PID].channel = 2;

  ads_module[ADS_SENSOR_E2V].addr = 0x4B;
  ads_module[ADS_SENSOR_E2V].channel = 0;

  ads_module[ADS_SENSOR_CO].addr = 0x4A;
  ads_module[ADS_SENSOR_CO].channel = -1;

  for (int i = 0; i < ADS_SENSOR_COUNT; i++)
    ads_module[i].status = false;
}

bool ADS_Module::begin()
{
  for (int i = 0; i < ADS_SENSOR_COUNT; i++)
  {
    if (ads_module[i].module.begin(ads_module[i].addr))
      ads_module[i].status = true;
  }

  for (int i = 0; i < ADS_SENSOR_COUNT; i++)
  {
    if (ads_module[i].status == false)
      return false;
  }

  return true;
}

float ADS_Module::read_figaro(ads_sensor_id_e ads_sensor_id)
{
  ads_module_t *sensor = &ads_module[ads_sensor_id];

  const int samples = 20;
  float volts = 0.0;
  float contaminants = 0.0;
  float v_sum = 0.0;
  float c_sum = 0.0;
  int16_t adc = 0;

  if (!sensor->status)
    return -999;

  for (int i = 0; i < samples; i++)
  {
    adc = sensor->module.readADC_SingleEnded(sensor->channel);
    volts = sensor->module.computeVolts(adc);

    // rs/ro, change 0.1 to voltage in clean air, (5/voltage_dirty) / (5/Voltage_clean)
    contaminants = ((5.000 / volts) - 1) / ((5.000 / 0.1) - 1);

    c_sum += contaminants;
    v_sum += volts;
  }

  contaminants = c_sum / samples;
  volts = v_sum / samples;

  if (contaminants > 1.000)
    contaminants = 1.000;

  // return adc value, rs/ro, heater resistance.
  // Use other code to calc rs/ro, calc heater resistance,
  return volts;
}

float ADS_Module::read_co()
{
  ads_module_t *sensor = &ads_module[ADS_SENSOR_CO];

  float val;
  const float multiplier = 0.1875F;  // ADS1115  @ +/- 6.144V gain (16-bit results)

  if (!sensor->status)
    return -999;

  return (sensor->module.readADC_Differential_0_1() - sensor->module.readADC_Differential_2_3());
}

uint16_t ADS_Module::read_raw(ads_sensor_id_e ads_sensor_id)
{
  ads_module_t *sensor = &ads_module[ads_sensor_id];

  if (!sensor->status)
    return -999;

  return sensor->module.readADC_SingleEnded(sensor->channel);
}

String ADS_Module::read4sd()
{
  String out_str = "";

  out_str += String(read_figaro(ADS_SENSOR_FIG2600)) + ",";
  out_str += String(read_figaro(ADS_SENSOR_FIG2602)) + ",";
#if FIGARO3_ENABELD
  out_str += String(read_figaro(ADS_SENSOR_FIG3)) + ",";
#endif
#if FIGARO4_ENABELD
  out_str += String(read_figaro(ADS_SENSOR_FIG4)) + ",";
#endif
  out_str += String(read_raw(ADS_SENSOR_PID)) + ",";
  out_str += String(read_raw(ADS_SENSOR_E2V)) + ",";
  out_str += String(read_co());

  return out_str;
}

String ADS_Module::read4print()
{
  String out_str = "";

  out_str += "FIG2600:" + String(read_figaro(ADS_SENSOR_FIG2600)) + ",";
  out_str += "FIG2602:" + String(read_figaro(ADS_SENSOR_FIG2602)) + ",";
#if FIGARO3_ENABELD
  out_str += "FIG3:" + String(read_figaro(ADS_SENSOR_FIG3)) + ",";
#endif
#if FIGARO4_ENABELD
  out_str += "FIG4:" + String(read_figaro(ADS_SENSOR_FIG4)) + ",";
#endif
  out_str += "PID:" + String(read_raw(ADS_SENSOR_PID)) + ",";
  out_str += "E2V:" + String(read_raw(ADS_SENSOR_E2V)) + ",";
  out_str += "CO:" + String(read_co());

  return out_str;
}

String ADS_Module::read4sd_raw()
{
  String out_str = "";

  // out_str += String(read_figaro(ADS_SENSOR_FIG2600)) +"\t";
  out_str += String(read_raw(ADS_SENSOR_FIG2600)) + ",";

  // out_str += String(read_figaro(ADS_SENSOR_FIG2602)) + "\t";
  out_str += String(read_raw(ADS_SENSOR_FIG2602)) + ",";


  // out_str += String(read_figaro(ADS_SENSOR_FIG3)) + "\t";
  out_str += String(read_raw(ADS_SENSOR_FIG3)) + ",";

  // out_str += String(read_figaro(ADS_SENSOR_FIG4)) + "\t";
  out_str += String(read_raw(ADS_SENSOR_FIG4)) + ",";


  out_str += String(read_raw(ADS_SENSOR_PID)) + ",";
  // out_str += String(read_raw(ADS_SENSOR_E2V)) + "\t";

  out_str += String(read_co());

  return out_str;
}

String ADS_Module::read4print_raw()
{
  String out_str = "";
  
//   out_str += "FIG2600:" + String(read_figaro(ADS_SENSOR_FIG2600));
//   out_str += "(" + String(read_raw(ADS_SENSOR_FIG2600)) + "),";

//   out_str += "FIG2602:" + String(read_figaro(ADS_SENSOR_FIG2602));
//   out_str += "(" + String(read_raw(ADS_SENSOR_FIG2602)) + "),";

#if FIGARO3_ENABELD
  out_str += "FIG3:" + String(read_figaro(ADS_SENSOR_FIG3)) + ",";
  out_str += "(" + String(read_raw(ADS_SENSOR_FIG3)) + "),";
#endif

#if FIGARO4_ENABELD
  out_str += "FIG4:" + String(read_figaro(ADS_SENSOR_FIG4)) + ",";
  out_str += "(" + String(read_raw(ADS_SENSOR_FIG4)) + "),";
#endif

//   out_str += "PID:" + String(read_raw(ADS_SENSOR_PID)) + ",";
//   out_str += "E2V:" + String(read_raw(ADS_SENSOR_E2V)) + ",";

  out_str += "CO:" + String(read_co());

  return out_str;
}