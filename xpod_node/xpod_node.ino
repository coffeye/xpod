/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.cpp
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
#include "xpod_node.h"

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <s300i2c.h>

#include "ads_module.h"
#include "bme_module.h"

#if GPS_ENABLED
#include "gps_module.h"
GPS_Module gps_module;
#endif

#if RTC_ENABLED
#include "rtc_module.h"
RTC_Module rtc_module;
#endif


#if QUAD_ENABLED
#include "quad_module.h"
QUAD_Module quad_module;
#endif

#if MQ131_ENABLED
#include "mq131_module.h"
MQ131_Module mq131_module;
#endif

#if PMS_ENABLED
#include "pms_module.h"
PMS_Module pms_module;
#endif


/*************  Global Declarations  *************/
// Modules
S300I2C co2_sensor(Wire);
ADS_Module ads_module;
BME_Module bme_module;

// Variables
File file;

/******************  Functions  ******************/
void setup()
{
  Serial.begin(9600);
  Serial.println();

  // In voltage
  pinMode(IN_VOLT_PIN, INPUT);

  // Motor control
  pinMode(MOTOR_CTRL_IN_PIN, INPUT);
  pinMode(MOTOR_CTRL_OUT_PIN, OUTPUT);

  // Status LEDs
  pinMode(STATUS_RUNNING, OUTPUT);
  pinMode(STATUS_ERROR, OUTPUT);
  pinMode(STATUS_HALTED, OUTPUT);

  Wire.begin();

#if SDCARD_LOG_ENABLED
  if (!SD.begin(SD_CARD_CS_PIN)) {
    digitalWrite(STATUS_HALTED, HIGH);
    Serial.println("Error: Card failed, or not present");
    while(1);
  }

  file = SD.open("xpod.txt", FILE_WRITE);

  if (!file) {
    Serial.println("Error: Failed to open file");
    digitalWrite(STATUS_HALTED, HIGH);
    while(1);
  }
#endif

  if (!co2_sensor.begin(CO2_I2C_ADDR))
    Serial.println("Error: Failed to initialize CO2 sensor!");

  if (!bme_module.begin())
    Serial.println("Error: Failed to initialize BME sensor!");

  if (!ads_module.begin())
    Serial.println("Error: Failed to initialize one of the ADS1115 module!");

#if QUAD_ENABLED
  if (!quad_module.begin())
    Serial.println("Error: Failed to initialize Quad Stat!");
#endif

#if MQ131_ENABLED
  if (!mq131_module.begin())
    Serial.println("Error: Failed to initialize MQ131 sensor!");
#endif

#if RTC_ENABLED 
  if (!rtc_module.begin())
    Serial.println("Error: Failed to initialize RTC module");
#endif

#if GPS_ENABLED
  if (!gps_module.begin())
    Serial.println("Error: Failed to initialize GPS module!");
#endif

#if PMS_ENABLED
  if (!pms_module.begin())
    Serial.println("Error: Failed to initialize PM sensor!");
#endif

  delay(1000);
}

void loop()
{
  int motor_ctrl_val;
  float in_volt_val;

  in_volt_val = (analogRead(IN_VOLT_PIN) * 5.02 * 5) / 1023.0;

#if SERIAL_LOG_ENABLED

#if RTC_ENABLED
  Serial.print(rtc_module.getDateTime());
  Serial.print(",");
#endif 

  Serial.print("Volt:");
  Serial.print(in_volt_val);
  Serial.print(",");

  Serial.print(ads_module.read4print_raw());
  Serial.print(",");

  Serial.print("CO2:");
  Serial.print(co2_sensor.getCO2ppm());
  Serial.print(",");

  // Serial.print(bme_module.read4print());
  // Serial.print(",");

#if QUAD_ENABLED
  Serial.print(quad_module.read());
  Serial.print(",");
#endif 

#if GPS_ENABLED
  Serial.print(gps_module.get_gps_info());
  Serial.print(",");
#endif 

#if MQ131_ENABLED
  Serial.print(mq131_module.read4print());
  Serial.print(",");
#endif

#if PMS_ENABLED
  Serial.print(pms_module.read4print());
#endif

  Serial.println();
#endif  //SERIAL_LOG_ENABLED

#if SDCARD_LOG_ENABLED

#if RTC_ENABLED
  file.print(rtc_module.getDateTime());
  file.print(",");
#endif

  file.print(in_volt_val);
  file.print(",");

  file.print(ads_module.read4sd_raw());
  file.print(",");

  file.print(co2_sensor.getCO2ppm());
  file.print(",");

  file.print(bme_module.read4sd());
  file.print(",");

#if GPS_ENABLED
  file.print(gps_module.get_gps_info());
  file.print(",");
#endif

#if QUAD_ENABLED
  file.print(quad_module.read());
  file.print(",");
#endif

#if MQ131_ENABLED
  file.print(mq131_module.read4sd());
  file.print(",");
#endif

#if PMS_ENABLED
  file.print(pms_module.read4sd());
#endif

  file.println();
  file.flush();
#endif //SDCARD_LOG_ENABLED

  // Motor control
  motor_ctrl_val = analogRead(MOTOR_CTRL_IN_PIN);
  motor_ctrl_val = (((float)motor_ctrl_val / 1024) * 255);
  analogWrite(MOTOR_CTRL_OUT_PIN, motor_ctrl_val);

  // Status indication
  digitalWrite(STATUS_RUNNING, HIGH);
  delay(100);
  digitalWrite(STATUS_RUNNING, LOW);
}