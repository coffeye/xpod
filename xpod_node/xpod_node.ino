/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.ino
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Rylee Beach
 * @date 	  ...
 *
 * @editor  Feb 06 2023, Ajay Kandagal, ajka9053@colorado.edu
 * @change  Added macro flags to enable/disable logging on SD-card or
 *          Serial Monitor logging.
 *
 * @editor  Feb 14 2023, Ajay Kandagal, ajka9053@colorado.edu
 * @change  Added time-stamping using MKR GPS Module.
 ******************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_BME680.h>
#include <s300i2c.h>
#include "xpod_node.h"
#include "gps_module.h"


#define CO_X4_ADDR            (0x4A)
#define IN_VOLT_PIN           (A0)
#define SEALEVELPRESSURE_HPA  (1013.25)


const sensor_info_t sensor_info_arr[SENSORS_COUNT] = {
    {.addr = 0x48, .channel = 3, .name = "FIGARO_2600"},
    {.addr = 0x49, .channel = 2, .name = "FIGARO_2602"},
    {.addr = 0x48, .channel = 2, .name = "PID"},
    {.addr = 0x4B, .channel = 0, .name = "E2V"},
    {.addr = 0x76, .channel = -1, .name = "BME"},
    {.addr = 0x31, .channel = -1, .name = "CO2"},
    {.addr = 0x4A, .channel = -1, .name = "CO"}};

sensor_data_t sensor_data_arr[DATA_COUNT] = {
    {.name = "IN_VOLT", .unit = "V"},
    {.name = "FIG2600", .unit = ""},
    {.name = "FIG2602", .unit = ""},
    {.name = "PID", .unit = ""},
    {.name = "E2V", .unit = ""},
    {.name = "CO2", .unit = ""},
    {.name = "CO", .unit = ""},
    {.name = "Temp", .unit = "C"},
    {.name = "Pres", .unit = "hpa"},
    {.name = "Hum", .unit = "%"},
    {.name = "Gas", .unit = "KOhms"},
    {.name = "Alt", .unit = "m"}};

soft_spi_sd_t soft_spi_sd = {
    .cs_pin = 43, 
    .miso_pin = 49, 
    .mosi_pin = 45, 
    .sck_pin = 47
};


#if ENABLE_DEDICATED_SPI && SDCARD_LOG_ENABLED
SoftSpiDriver<soft_spi_sd.miso_pin, soft_spi_sd.miso_pin, soft_spi_sd.cs_pin> softSpi;
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI

Adafruit_ADS1115 ads_module;
Adafruit_BME680 bme_sensor; // I2C
S300I2C co2_sensor(Wire);
SdFat sd;
File file;
GPS_Module gps_module;

void setup()
{
#if SERIAL_LOG_ENABLED
    Serial.begin(9600);
#endif // SERIAL_LOG_ENABLED

#if SDCARD_LOG_ENABLED
    if (!sd.begin(SD_CONFIG))
    {
        sd.initErrorHalt();
    }

    if (!file.open("xpod.txt", O_RDWR | O_CREAT))
    {
        sd.errorHalt("open failed");
    }
#endif // SDCARD_LOG_ENABLED

    // For reading input voltage
    pinMode(IN_VOLT_PIN, INPUT);

    // For reading co2 sensor
    Wire.begin();

    if (!co2_sensor.begin(sensor_info_arr[SENSOR_CO2].addr))
    {
        Serial.println("Error: CO2 sensor not working!");
    }

    if (!bme_sensor.begin(sensor_info_arr[SENSOR_BME].addr))
    {
        Serial.println("Error: BME680 sensor not working!");
    }
    else
    {
        // Set up oversampling and filter initialization
        bme_sensor.setTemperatureOversampling(BME680_OS_8X);
        bme_sensor.setHumidityOversampling(BME680_OS_2X);
        bme_sensor.setPressureOversampling(BME680_OS_4X);
        bme_sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme_sensor.setGasHeater(320, 150); // 320*C for 150 ms
    }

    delay(1000);
}

int itr_counter = 0;
char gps_data[32];

void loop()
{
    sensor_data_arr[DATA_INVOLT].value = (analogRead(IN_VOLT_PIN) * 5.02 * 5.0) / 1023.0;
    sensor_data_arr[DATA_FIG2600].value = read_figaro(&sensor_info_arr[SENSOR_FIG2600]);
    sensor_data_arr[DATA_FIG2602].value = read_figaro(&sensor_info_arr[SENSOR_FIG2602]);
    sensor_data_arr[DATA_PID].value = read_ads1115(&sensor_info_arr[SENSOR_PID]);
    sensor_data_arr[DATA_E2V].value = read_ads1115(&sensor_info_arr[SENSOR_E2V]);
    sensor_data_arr[DATA_CO2].value = co2_sensor.getCO2ppm();
    sensor_data_arr[DATA_CO].value = read_co(&sensor_info_arr[SENSOR_CO]); // addr SDA
    sensor_data_arr[DATA_TEMPERATURE].value = bme_sensor.temperature;
    sensor_data_arr[DATA_PRESSURE].value = bme_sensor.pressure / 100.0;
    sensor_data_arr[DATA_HUMIDITY].value = bme_sensor.humidity;
    sensor_data_arr[DATA_GAS].value = bme_sensor.gas_resistance / 1000.0;
    sensor_data_arr[DATA_ALTITUDE].value = bme_sensor.readAltitude(SEALEVELPRESSURE_HPA);

#if SERIAL_LOG_ENABLED
    // Time-stamping using GPS Module
    Serial.print("[");
    gps_module.getGpsDateTime(gps_data);
    if (strlen(gps_data) != 0) {
      Serial.print(gps_data);
    }
    else {
      Serial.print(itr_counter);
    }
    Serial.print("]");

    for (int i = 0; i < DATA_COUNT; i++)
    {
        Serial.print(sensor_data_arr[i].name);
        Serial.print(": ");
        Serial.print(sensor_data_arr[i].value);

        if (sensor_data_arr[i].unit != "")
        {
            Serial.print(" ");
            Serial.print(sensor_data_arr[i].unit);
        }

        if (i != (DATA_COUNT - 1))
            Serial.print(" , ");
        else
            Serial.println();
    }
#endif

#if SDCARD_LOG_ENABLED
    file.print(itr_counter);
    file.print(",");
    for (int i = 0; i < DATA_COUNT; i++)
    {
        file.print(sensor_data_arr[i].value);

        if (i != (DATA_COUNT - 1))
            file.print(" , ");
        else
            file.println();
    }
    file.close();
#endif

    // delay(500);
    itr_counter++;
}

uint16_t read_ads1115(const sensor_info_t *sensor_info)
{
    if (!ads_module.begin(sensor_info->addr))
    {
#if DEBUG_LOG_ENABLED
        Serial.print("Failed to initialize ");
        Serial.println(sensor_info->name);
#endif
        return -999;
    }
    return ads_module.readADC_SingleEnded(sensor_info->channel);
}

float read_co(const sensor_info_t *sensor_info)
{
    int16_t worker, Aux;
    float multiplier = 0.1875F; // ADS1115  @ +/- 6.144V gain (16-bit results)

    if (!ads_module.begin(sensor_info->addr))
    {
#if DEBUG_LOG_ENABLED
        Serial.print("Failed to initialize ");
        Serial.println(sensor_info->name);
#endif
        return -999;
    }

    worker = ads_module.readADC_Differential_0_1();
    Aux = ads_module.readADC_Differential_2_3();
    float result = worker - Aux;
    // Serial.print("Result: ");
    // Serial.print(result);
    // Serial.print("(");
    // Serial.print(result * multiplier);
    // Serial.println("mV)");
    return result;
}

float read_figaro(const sensor_info_t *sensor_info)
{
    float figaro_volts = 0.0;
    float contaminants = 0.0;
    float sum = 0.0;
    float Vsum = 0.0;
    int n = 0;
    int avg = 20;
    int16_t adc = 0;

    if (!ads_module.begin(sensor_info->addr))
    {
#if DEBUG_LOG_ENABLED
        Serial.print("Failed to initialize ");
        Serial.println(sensor_info->name);
#endif
        return -999;
    }

    while (n != avg)
    {
        adc = ads_module.readADC_SingleEnded(sensor_info->channel);
        figaro_volts = ads_module.computeVolts(adc);
        contaminants = ((5.000 / figaro_volts) - 1) / ((5.000 / 0.1) - 1); // rs/ro, change 0.1 to voltage in clean air, (5/voltage_dirty) / (5/Voltage_clean)
        sum = sum + contaminants;
        Vsum = Vsum + figaro_volts;
        n++;
    }
    contaminants = sum / avg;
    figaro_volts = Vsum / avg;
    if (contaminants > 1.000)
    {
        contaminants = 1.000;
    }
    n = 0;
    sum = 0.0;
    Vsum = 0.0;
    // return contaminants;
    return figaro_volts; // return adc value, rs/ro, heater resistance. use other code to calc rs/ro, calc heater resistance,
}