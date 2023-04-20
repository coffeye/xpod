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
#include "xpod_node.h"

// SdFat software SPI
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;

#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI

SdFat sd;
File file;
S300I2C co2_sensor(Wire);
LDT_Module ldt_module;
ADS_Module ads_module;
BME_Module bme_module;

void setup()
{
  Serial.begin(9600);

#if SDCARD_LOG_ENABLED
  if (!sd.begin(SD_CONFIG))
  {
    Serial.println("Failed to initialize SD card");
    sd.initErrorHalt();
  }

  // check if file is present
  if (!file.open("xpod.txt", O_RDWR | O_AT_END))
  {
    file.open("xpod.txt", O_RDWR | O_CREAT);
  }
#endif

  Wire.begin();
  
  if (!ldt_module.gpsBegin())
    Serial.println("Error: Failed to initialize GPS module!");

  if (!ldt_module.rtcBegin())
    Serial.println("Error: Failed to initialize RTC module!");

  if (!bme_module.begin())
    Serial.println("Error: Failed to initialize BME sensor!");

  if (!ads_module.begin())
    Serial.println("Error: Failed to initialize one of the ADS1115 module!");

  if (!co2_sensor.begin(CO2_I2C_ADDR))
    Serial.println("Error: CO2 sensor not working!");

  pinMode(IN_VOLT_PIN, INPUT);

#if MOTOR_ENABLED
  pinMode(MOTOR_CONTROL_POT_PIN, INPUT);
  // Currently pot VCC is connected to D51, in future this
  // needs to be connected to Power supply directly.
  pinMode(51, OUTPUT);
  digitalWrite(51, HIGH);
  
  pinMode(MOTOR_CONTROL_PIN, OUTPUT);
  digitalWrite(MOTOR_CONTROL_PIN, LOW);
#endif

  delay(1000);
}

void loop()
{
  float in_volt_val = (analogRead(IN_VOLT_PIN) * 5.02 * 5) / 1023.0;

#if SERIAL_LOG_ENABLED
  Serial.println();

#if RTC_ENABLED
  Serial.print(ldt_module.getRtcDateTime());
#else
  Serial.print(ldt_module.getGpsDateTime());
#endif
  Serial.print(",");

  Serial.print("Volt:");
  Serial.print(in_volt_val);
  Serial.print(",");

  Serial.print(ads_module.read4print_raw());
  Serial.print(",");

  Serial.print("CO2:");
  Serial.print(co2_sensor.getCO2ppm());
  Serial.print(",");

  Serial.print(bme_module.get_bme4print());
#endif

#if SDCARD_LOG_ENABLED
  file.println();

#if RTC_ENABLED
  file.print(ldt_module.getRtcDateTime());
#else
  file.print(ldt_module.getGpsDateTime());
#endif
  file.print(",");

  file.print(in_volt_val);
  file.print(",");

  file.print(ads_module.read4sd_raw());
  file.print(",");

  file.print(co2_sensor.getCO2ppm());
  file.print(",");

  file.print(bme_module.get_bme4sd());
  file.flush();
#endif

#if MOTOR_ENABLED
  int motor_control_pot_val = analogRead(MOTOR_CONTROL_POT_PIN);
  motor_control_pot_val = (((float)motor_control_pot_val / 1024) * 255);
  analogWrite(MOTOR_CONTROL_PIN, motor_control_pot_val);
#endif
}