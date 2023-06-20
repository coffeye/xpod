/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.h
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Ajay Kandagal
 * @date 	  Following features are present:
 *          - Sensors Figaro 2600, Fiagaro 2602, CO, PID, E2V, CO, CO2 and BME
 *          - RTC time and GPS time stamping
 *          - SD Card logging
 *          - Motor control
 ******************************************************************************/
#ifndef _XPOD_NODE_H
#define _XPOD_NODE_H

#define SDCARD_LOG_ENABLED    1
#define SERIAL_LOG_ENABLED    1
#define RTC_ENABLED           1
#define GPS_ENABLED           0
#define PMS_ENABLED           0
#define QUAD_ENABLED          1
#define MQ131_ENABLED         0
#define MOTOR_ENABLED         0
#define MET_STATION           1

#define MOTOR_CTRL_IN_PIN     A14
#define MOTOR_CTRL_OUT_PIN    2
#define CO2_I2C_ADDR          0x31
#define IN_VOLT_PIN           A0
#define SD_CARD_CS_PIN        53

#define STATUS_RUNNING        12
#define STATUS_ERROR          11
#define STATUS_HALTED         13

#endif // _XPOD_NODE_H