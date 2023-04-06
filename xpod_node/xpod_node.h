/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.h
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Rylee Beach
 * @date 	  ...
 *
 * @editor  Ajay Kandagal, ajka9053@colorado.edu
 * @change  Feb 06 2023, Added macro flags to enable/disable logging on SD-card 
 *          or Serial monitor logging.
 * @change  Feb 14 2023, Added time-stamping using MKR GPS Module.
 * @change  Feb 25 2023, Added SD card logging functionality. 
 * @change  Mar 02 2023, Fixed memeory leak, caused by Adafruit_ADS1X15.begin().
 * @change  Mar 27 2023, Integrated GPS data logging into main code.
 * @change  Apr 03 2023, RTC timestamping integrated into main code.
 * @change  Apr 05 2023, Motor control through pot Added. Reading from 3rd and 
 *          4th Figaro sensors added.
 ******************************************************************************/
#ifndef _XPOD_NODE_H
#define _XPOD_NODE_H


#define SDCARD_LOG_ENABLED    0
#define DEBUG_LOG_ENABLED     0
#define FIGARO_RAW_ENABLED    0
#define SERIAL_LOG_ENABLED    1
#define RTC_ENABLED           0
#define PM_ENABLED            0
#define MOTOR_ENABLED         0
#define FIGARO_THIRD_ENABELD  0

#define MOTOR_CONTROL_POT_PIN (A14)
#define MOTOR_CONTROL_PIN     (3)


#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_BME680.h>
#include <s300i2c.h>

#if PM_ENABLED
#include <Adafruit_PM25AQI.h>
#endif

#if RTC_ENABLED
#include <RTClib.h>
#endif

#include "gps_module.h"

const uint8_t SD_CS_PIN = 43;
const uint8_t SOFT_MISO_PIN = 49;
const uint8_t SOFT_MOSI_PIN = 45;
const uint8_t SOFT_SCK_PIN  = 47;

enum sensor_data_e
{
    DATA_INVOLT = 0,
    DATA_FIG2600,
    DATA_FIG3,
    DATA_FIG2602,
    DATA_FIG4,
    DATA_PID,
    DATA_E2V,
    DATA_CO2,
    DATA_CO,
    DATA_TEMPERATURE,
    DATA_PRESSURE,
    DATA_HUMIDITY,
    DATA_GAS,
    DATA_ALTITUDE,
    DATA_COUNT
};

enum sensor_info_e
{
    SENSOR_FIG2600 = 0,
    SENSOR_FIG3,
    SENSOR_FIG2602,
    SENSOR_FIG4,
    SENSOR_PID,
    SENSOR_E2V,
    SENSOR_BME,
    SENSOR_CO2,
    SENSOR_CO,
    SENSORS_COUNT
};

typedef struct
{
    const char *name;
    const char *unit;
    float value;
} sensor_data_t;

typedef struct
{
    const uint8_t addr;
    const int8_t channel;
    const char *name;
    Adafruit_ADS1115 *module;
} sensor_info_t;

#endif // _XPOD_NODE_H