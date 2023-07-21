/*******************************************************************************
 * @file    quad_module.cpp
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 20 2023
 ******************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include "quad_module.h"

QUAD_Module::QUAD_Module()
{
  status = true;
}

bool QUAD_Module::begin()
{
  alpha_one = MCP342x(APLHA_ONE_ADDR);
  alpha_two = MCP342x(APLHA_TWO_ADDR);

  MCP342x::generalCallReset();
  delay(1);

  // Wire.requestFrom(APLHA_ONE_ADDR, (uint8_t)1);

  // if (!Wire.available())
  //   status = false;
  
  // Wire.requestFrom(APLHA_TWO_ADDR, (uint8_t)1);

  // if (!Wire.available())
  //   status = false;

  return status;
}

String QUAD_Module::read()
{
  MCP342x::Config status;
  String quad_data;
  long value = 0;

  // Initiate a conversion; convertAndRead() will wait until it can be read
  alpha_one.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_one.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_one.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_one.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";


  alpha_two.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_two.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_two.convertAndRead(MCP342x::channel3, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value) + ",";

  alpha_two.convertAndRead(MCP342x::channel4, MCP342x::oneShot, MCP342x::resolution16, 
                          MCP342x::gain1, 1000000, value, status);
  quad_data += String(value);
    
  // //Read ADCs on-board and on-Quadstat
  // for (int i = 1; i <= 16; i++) 
  // {
  //   if (i <= 4) 
  //     quad_data += alpha_one.GetValue(i) + ",";
  //   else if (i <= 8) 
  //     quad_data += alpha_two.GetValue(i - 4) + ",";
  // }

  return quad_data;
}