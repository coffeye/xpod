/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.ino
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Ajay Kandagal, ajka9053@colorado.edu
 * @date 	  Feb 09 2023
 ******************************************************************************/
#ifndef _XPOD_NODE_H
#define _XPOD_NODE_H

#define SDCARD_LOG_ENABLEB  0
#define SERIAL_LOG_ENABLED  1
#define DEBUG_LOG_ENABLED   0
#define FIGARO_RAW_ENABLED  1

enum sensor_data_e
{
    DATA_INVOLT = 0,
    DATA_FIG2600,
    DATA_FIG2602,
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
    SENSOR_FIG2602,
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
    uint8_t addr;
    int8_t channel;
    char *name;
} sensor_info_t;

typedef struct
{
    const uint8_t cs_pin;
    const uint8_t miso_pin;
    const uint8_t mosi_pin;
    const uint8_t sck_pin;
} soft_spi_sd_t;

#endif // _XPOD_NODE_H