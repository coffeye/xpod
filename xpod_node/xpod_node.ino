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

#define CO_X4_ADDR (0x4A)
#define IN_VOLT_PIN (A0)
#define GPS_INDICATOR_LED (13)
#define SEALEVELPRESSURE_HPA (1013.25)

sensor_info_t sensor_info_arr[SENSORS_COUNT] = {
  { .addr = 0x48, .channel = 3, .name = "FIG_2600", .module = new Adafruit_ADS1115 },
  { .addr = 0x48, .channel = 0, .name = "FIG_3", .module = new Adafruit_ADS1115 },
  { .addr = 0x49, .channel = 2, .name = "FIG_2602", .module = new Adafruit_ADS1115 },
  { .addr = 0x49, .channel = 0, .name = "FIG_4", .module = new Adafruit_ADS1115 },
  { .addr = 0x48, .channel = 2, .name = "PID", .module = new Adafruit_ADS1115 },
  { .addr = 0x4B, .channel = 0, .name = "E2V", .module = new Adafruit_ADS1115 },
  { .addr = 0x76, .channel = -1, .name = "BME", .module = NULL },
  { .addr = 0x31, .channel = -1, .name = "CO2", .module = NULL },
  { .addr = 0x4A, .channel = -1, .name = "CO", .module = new Adafruit_ADS1115 }
};

sensor_data_t sensor_data_arr[DATA_COUNT] = {
  { .name = "IN_VOLT", .unit = "V" },
  { .name = "FIG2600", .unit = "" },
  { .name = "FIG3", .unit = "" },
  { .name = "FIG2602", .unit = "" },
  { .name = "FIG4", .unit = "" },
  { .name = "PID", .unit = "" },
  { .name = "E2V", .unit = "" },
  { .name = "CO2", .unit = "" },
  { .name = "CO", .unit = "" },
  { .name = "Temp", .unit = "C" },
  { .name = "Pres", .unit = "hpa" },
  { .name = "Hum", .unit = "%" },
  { .name = "Gas", .unit = "KOhms" },
  { .name = "Alt", .unit = "m" }
};

// SdFat software SPI
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;

#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI

Adafruit_BME680 bme_sensor;  // I2C
S300I2C co2_sensor(Wire);
SdFat sd;
File file;
GPS_Module gps_module;

#if RTC_ENABLED
RTC_DS3231 rtc_module;
#endif

#if PM_ENABLED
Adafruit_PM25AQI pm_sensor = Adafruit_PM25AQI();
#endif

uint32_t itr_counter = 0;
uint16_t fig00_raw16;
uint16_t fig02_raw16;
char gps_data[32];

void setup() {
#if SERIAL_LOG_ENABLED
  Serial.begin(9600);
#endif  // SERIAL_LOG_ENABLED


#if SDCARD_LOG_ENABLED
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }

  // check if file is present
  if (!file.open("xpod.txt", O_RDWR | O_AT_END)) {
    file.open("xpod.txt", O_RDWR | O_CREAT);
  }
#endif  // SDCARD_LOG_ENABLED

  // For reading input voltage
  pinMode(IN_VOLT_PIN, INPUT);

  pinMode(GPS_INDICATOR_LED, OUTPUT);
  // digitalWrite(GPS_INDICATOR_LED, LOW);

  // For reading co2 sensor
  Wire.begin();

  if (!co2_sensor.begin(sensor_info_arr[SENSOR_CO2].addr)) {
#if SERIAL_LOG_ENABLED
    Serial.println("Error: CO2 sensor not working!");
#endif
  }

  if (!bme_sensor.begin(sensor_info_arr[SENSOR_BME].addr)) {
#if SERIAL_LOG_ENABLED
    Serial.println("Error: BME680 sensor not working!");
#endif
  } else {
    // Set up oversampling and filter initialization
    bme_sensor.setTemperatureOversampling(BME680_OS_8X);
    bme_sensor.setHumidityOversampling(BME680_OS_2X);
    bme_sensor.setPressureOversampling(BME680_OS_4X);
    bme_sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme_sensor.setGasHeater(320, 150);  // 320*C for 150 ms
  }

  for (int i = 0; i < SENSORS_COUNT; i++) {
    if (sensor_info_arr[i].module != NULL && !sensor_info_arr[i].module->begin(sensor_info_arr[i].addr)) {
#if SERIAL_LOG_ENABLED
      Serial.print("Failed to initialize ");
      Serial.println(sensor_info_arr[i].name);
#endif
      //delete sensor_info_arr[i].module;
      sensor_info_arr[i].module = NULL;
    }
  }

#if RTC_ENABLED
  if (!rtc_module.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc_module.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc_module.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
#endif

#if PM_ENABLED
  if (!pm_sensor.begin_UART(&Serial1)) {
    Serial.println("Could not find PM sensor!");
  }
#endif

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

void loop() {

#if MOTOR_ENABLED
  int motor_control_pot_val = analogRead(MOTOR_CONTROL_POT_PIN);
  motor_control_pot_val = (((float)motor_control_pot_val / 1024) * 255);
  analogWrite(MOTOR_CONTROL_PIN, motor_control_pot_val);
#endif

#if PM_ENABLED
  PM25_AQI_Data data;
  if (pm_sensor.read(&data))
  {
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
    Serial.print("Particles > 10 um / 0.1L air:"); Serial.println(data.particles_100um);
  }
  else
  {
    Serial.println("Could not read from PM sensor");
  }
#endif

  sensor_data_arr[DATA_INVOLT].value = (analogRead(IN_VOLT_PIN) * 5.02 * 5.0) / 1023.0;
  sensor_data_arr[DATA_CO2].value = co2_sensor.getCO2ppm();
  sensor_data_arr[DATA_FIG2600].value = read_figaro(&sensor_info_arr[SENSOR_FIG2600]);
  sensor_data_arr[DATA_FIG3].value = read_figaro(&sensor_info_arr[SENSOR_FIG3]);
  sensor_data_arr[DATA_FIG2602].value = read_figaro(&sensor_info_arr[SENSOR_FIG2602]);
  sensor_data_arr[DATA_PID].value = read_ads1115(&sensor_info_arr[SENSOR_PID]);
  sensor_data_arr[DATA_E2V].value = read_ads1115(&sensor_info_arr[SENSOR_E2V]);
  sensor_data_arr[DATA_CO].value = read_co(&sensor_info_arr[SENSOR_CO]);  // addr SDA
  sensor_data_arr[DATA_TEMPERATURE].value = bme_sensor.temperature;
  sensor_data_arr[DATA_PRESSURE].value = bme_sensor.pressure / 100.0;
  sensor_data_arr[DATA_HUMIDITY].value = bme_sensor.humidity;
  sensor_data_arr[DATA_GAS].value = bme_sensor.gas_resistance / 1000.0;
  sensor_data_arr[DATA_ALTITUDE].value = bme_sensor.readAltitude(SEALEVELPRESSURE_HPA);

#if RTC_ENABLED
  DateTime rtc_data = rtc_module.now();

  sprintf(gps_data, "%02d/%02d/%02d,%02d:%02d:%02d", rtc_data.month(), rtc_data.day(), 
  rtc_data.year(), rtc_data.hour(), rtc_data.minute(), rtc_data.second());
#else
  gps_module.getGpsDateTime(gps_data);
#endif

#if SERIAL_LOG_ENABLED
  // Time-stamping using GPS Module
  Serial.print("[");
  if (strlen(gps_data) != 0) {
    // digitalWrite(GPS_INDICATOR_LED, HIGH);
    Serial.print(gps_data);
  } else {
    // digitalWrite(GPS_INDICATOR_LED, LOW);
    Serial.print(itr_counter);
  }
  Serial.print("] ");

  Serial.print(sensor_data_arr[DATA_INVOLT].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_INVOLT].value);
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_FIG2600].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_FIG2600].value);
#if FIGARO_RAW_ENABLED
  Serial.print("(");
  Serial.print(fig00_raw16);
  Serial.print(")");
#endif
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_FIG2602].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_FIG2602].value);
#if FIGARO_RAW_ENABLED
  Serial.print("(");
  Serial.print(fig02_raw16);
  Serial.print(")");
#endif
  Serial.print(" , ");
#if FIGARO_THIRD_ENABELD
  Serial.print(sensor_data_arr[DATA_FIG3].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_FIG3].value);
  Serial.print(" , ");
#endif
  Serial.print(sensor_data_arr[DATA_PID].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_PID].value);
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_E2V].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_E2V].value);
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_CO2].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_CO2].value);
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_CO].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_CO].value);
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_TEMPERATURE].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_TEMPERATURE].value);
  Serial.print(" C");
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_PRESSURE].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_PRESSURE].value);
  Serial.print(" hpa");
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_HUMIDITY].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_HUMIDITY].value);
  Serial.print(" %");
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_GAS].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_GAS].value);
  Serial.print(" KOhms");
  Serial.print(" , ");

  Serial.print(sensor_data_arr[DATA_ALTITUDE].name);
  Serial.print(": ");
  Serial.print(sensor_data_arr[DATA_ALTITUDE].value);
  Serial.print(" m");

  Serial.println();
#endif

#if SDCARD_LOG_ENABLED
  file.println();
  if (strlen(gps_data) != 0) {
    // digitalWrite(GPS_INDICATOR_LED, HIGH);
    file.print(gps_data);
  } else {
    // digitalWrite(GPS_INDICATOR_LED, LOW);
    file.print(itr_counter);
  }
  file.print(",");
  file.print(sensor_data_arr[DATA_INVOLT].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_FIG2600].value);
  file.print(",");
#if FIGARO_RAW_ENABLED
  file.print(fig00_raw16);
  file.print(",");
#endif
  file.print(sensor_data_arr[DATA_FIG2602].value);
  file.print(",");
#if FIGARO_RAW_ENABLED
  file.print(fig02_raw16);
  file.print(",");
#endif
#if FIGARO_THIRD_ENABELD
  file.print(sensor_data_arr[DATA_FIG3].value);
  file.print(",");
#endif
  file.print(sensor_data_arr[DATA_PID].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_E2V].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_CO2].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_CO].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_TEMPERATURE].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_PRESSURE].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_HUMIDITY].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_GAS].value);
  file.print(",");
  file.print(sensor_data_arr[DATA_ALTITUDE].value);
  file.flush();
#endif

  itr_counter++;
}

uint16_t read_ads1115(const sensor_info_t *sensor_info) {
  if (sensor_info->module == NULL)
    return -999;

  return sensor_info->module->readADC_SingleEnded(sensor_info->channel);
}

float read_co(const sensor_info_t *sensor_info) {
  int16_t worker, Aux;
  float multiplier = 0.1875F;  // ADS1115  @ +/- 6.144V gain (16-bit results)

  if (sensor_info->module == NULL)
    return -999;

  worker = sensor_info->module->readADC_Differential_0_1();
  Aux = sensor_info->module->readADC_Differential_2_3();
  float result = worker - Aux;
  // Serial.print("Result: ");
  // Serial.print(result);
  // Serial.print("(");
  // Serial.print(result * multiplier);
  // Serial.println("mV)");
  return result;
}

float read_figaro(const sensor_info_t *sensor_info) {
  float figaro_volts = 0.0;
  float contaminants = 0.0;
  float c_sum = 0.0;
  float v_sum = 0.0;
  int samples = 20;
  int16_t adc = 0;
#if FIGARO_RAW_ENABLED
  uint32_t raw_sum = 0.0;
#endif

  if (sensor_info->module == NULL)
    return -999;

  for (int i = 0; i < samples; i++) {
    adc = sensor_info->module->readADC_SingleEnded(sensor_info->channel);
    figaro_volts = sensor_info->module->computeVolts(adc);
    // rs/ro, change 0.1 to voltage in clean air, (5/voltage_dirty) / (5/Voltage_clean)
    contaminants = ((5.000 / figaro_volts) - 1) / ((5.000 / 0.1) - 1);
    c_sum = c_sum + contaminants;
    v_sum = v_sum + figaro_volts;
#if FIGARO_RAW_ENABLED
    raw_sum = raw_sum + adc;
#endif
  }

  contaminants = c_sum / samples;
  figaro_volts = v_sum / samples;

#if FIGARO_RAW_ENABLED
  if (sensor_info->addr == 0x48 && sensor_info->channel == 3)
    fig00_raw16 = raw_sum / samples;
  else if (sensor_info->addr == 0x49 && sensor_info->channel == 2)
    fig02_raw16 = raw_sum / samples;
#endif

  if (contaminants > 1.000)
    contaminants = 1.000;

  // return contaminants;
  return figaro_volts;  // return adc value, rs/ro, heater resistance. use other code to calc rs/ro, calc heater resistance,
}

#if PM_ENABLED
boolean read_pms()
{
   if (! s->available())
  {
     return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42)
  {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32)
  {
      return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i=0; i<30; i++)
  {
    sum += buffer[i];
  }

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++)
  {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&PMS_data, (void *)buffer_u16, 30);

  if (sum != PMS_data.checksum)
  {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
#endif