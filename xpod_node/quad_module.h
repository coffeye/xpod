/*******************************************************************************
 * @file    quad_module.h
 * @brief   
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 20 2023
 ******************************************************************************/
#ifndef _QUAD_Module_H
#define _QUAD_Module_H

#include <MCP342x.h>

#define BME_SENSOR_ADDR       (0x76)
#define SEALEVELPRESSURE_HPA  (1013.25)

#define APLHA_ONE_ADDR        (0x69)
#define APLHA_TWO_ADDR        (0x6E)

class QUAD_Module
{
  public:
    QUAD_Module();
    bool begin();

    String read();
  private:
    MCP342x alpha_one;
    MCP342x alpha_two;
    bool status;
};

#endif  //_QUAD_Module_H