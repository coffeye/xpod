/*******************************************************************************
  * @file    wind_vane.cpp
  * @brief   
  *
  * @cite    Modest Maker (https://www.youtube.com/watch?v=KHrTqdmYoAk)
  *
  * @editor  Percy Smith, percy.smith@colorado.edu
  * @date 	  August 23, 2023
  ******************************************************************************/
#include "wind_vane.h"

// Here we're defining the wind vane "object"
wind_vane::wind_vane()
{
  status = false;
}

// This retrieves voltage reading and turns it into a voltage
float wind_vane::get_direction() 
{
  int sensorValue = analogRead(WINDVANE_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);

  return (voltage);
}

// This translates the directional voltage into a cardinal direction
String wind_vane::cardinal_direction(float directionVoltage)
{
  float windVane = directionVoltage;
	String compass = "";
	if(windVane > 4.61)       compass = "W";     //W
	else if(windVane > 4.33)  compass = "NW";    //NW
	else if(windVane > 4.03)  compass = "WNW";   //WNW
	else if(windVane > 3.84)  compass = "N";     //N
	else if(windVane > 3.43)  compass = "NNW";   //NNW
	else if(windVane > 3.06)  compass = "SW";    //SW
	else if(windVane > 2.92)  compass = "WSW";   //WSW
	else if(windVane > 2.23)  compass = "NE";    //NE
	else if(windVane > 1.96)  compass = "NNE";   //NNE
	else if(windVane > 1.38)  compass = "S";     //S
	else if(windVane > 1.17)  compass = "SSW";   //SSW
	else if(windVane > 0.88)  compass = "SE";    //SE
	else if(windVane > 0.60)  compass = "SSE";   //SSE
	else if(windVane > 0.43)  compass = "E";     //E
	else if(windVane > 0.39)  compass = "ENE";   //ENE
	else                      compass = "ESE";   //ESE

  return(compass);
}

// This will translate the directional voltage into the degrees of the direction
float wind_vane::degree_direction(float directionVoltage)
{
  float windVane = directionVoltage;
  float degrees;
  if(windVane > 4.61)       degrees = 270;      //W
	else if(windVane > 4.33)  degrees = 315;      //NW
	else if(windVane > 4.03)  degrees = 282.5;    //WNW
	else if(windVane > 3.84)  degrees = 0;        //N
	else if(windVane > 3.43)  degrees = 337.5;    //NNW
	else if(windVane > 3.06)  degrees = 225;      //SW
	else if(windVane > 2.92)  degrees = 247.5;    //WSW
	else if(windVane > 2.23)  degrees = 45;       //NE
	else if(windVane > 1.96)  degrees = 22.5;     //NNE
	else if(windVane > 1.38)  degrees = 180;      //S
	else if(windVane > 1.17)  degrees = 202.5;    //SSW
	else if(windVane > 0.88)  degrees = 135;      //SE
	else if(windVane > 0.60)  degrees = 157.5;    //SSE
	else if(windVane > 0.43)  degrees = 90;       //E
	else if(windVane > 0.39)  degrees = 67.5;     //ENE
	else                      degrees = 112.5;    //ESE

  return(degrees);
}
