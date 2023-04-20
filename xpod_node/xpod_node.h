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

#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <s300i2c.h>

#include "ldt_module.h"
#include "ads_module.h"
#include "bme_module.h"

#define SDCARD_LOG_ENABLED    1
#define SERIAL_LOG_ENABLED    1
#define RTC_ENABLED           1
#define PM_ENABLED            0
#define MOTOR_ENABLED         1

#define MOTOR_CONTROL_POT_PIN (A14)
#define MOTOR_CONTROL_PIN     (3)
#define CO2_I2C_ADDR          (0x31)
#define IN_VOLT_PIN           (A0)

const uint8_t SD_CS_PIN = 43;
const uint8_t SOFT_MISO_PIN = 49;
const uint8_t SOFT_MOSI_PIN = 45;
const uint8_t SOFT_SCK_PIN  = 47;

#endif // _XPOD_NODE_H