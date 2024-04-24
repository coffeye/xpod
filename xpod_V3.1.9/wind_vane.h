/*******************************************************************************
  * @file    wind_vane.h
  * @brief   
  *
  * @cite    Modest Maker (https://www.youtube.com/watch?v=KHrTqdmYoAk)
  *
  * @editor  Percy Smith, percy.smith@colorado.edu
  * @date 	  August 23, 2023
  ******************************************************************************/
#ifndef wind_vane_h
#define wind_vane_h

#include <Arduino.h>

#define WINDVANE_PIN      A15

class wind_vane
{
  public:
    wind_vane();
    float get_direction();
    String cardinal_direction(float directionVoltage);
    float degree_direction(float directionVoltage);
  private:
    bool status;
};

#endif /* wind_vane.h */