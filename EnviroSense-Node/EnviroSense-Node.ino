/*******************************************************************************
 * @project Hannigan Sensor IoT
 *
 * @file    EnviroSense-Node.ino
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Rylee Beach
 * @date 	  ...
 *
 * @editor  Feb 06, 2023, Ajay Kandagal, ajka9053@colorado.edu
 * @change  Formatted the code and added flags to enable/disable logging on
 *          SD-card or Serial Monitor.
 ******************************************************************************/
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <s300i2c.h>
#include <SdFat.h>

#define ENABLE_SDCARD_LOG   0
#define ENABLE_SERIAL_LOG   1
#define ENABLE_BMS          1
#define ENABLE_ADS1115      1

#define SEALEVELPRESSURE_HPA (1013.25) // do not change

Adafruit_ADS1115 ads;
Adafruit_BME680 bme; // I2C
S300I2C s3(Wire);

#if ENABLE_SDCARD_LOG

#if SPI_DRIVER_SELECT == 2 // Must be set in SdFat/SdFatConfig.h
#define SD_FAT_TYPE 3
#else // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT //must be two in SdFat/SdFatConfig.h
#endif // SPI_DRIVER_SELECT

const uint8_t SD_CS_PIN = 43;
const uint8_t SOFT_MISO_PIN = 49;
const uint8_t SOFT_MOSI_PIN = 45;
const uint8_t SOFT_SCK_PIN = 47;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif // ENABLE_DEDICATED_SPI

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif // SD_FAT_TYPE
#endif // ENABLE_SDCARD_LOG

int counter = 0;

#define FIAGARO2600_ADS_ADDR    (0x48)
#define PID_ADS_ADDR            (0x48)
#define FIAGARO2602_ADS_ADDR    (0x49)
#define CO_X4_ADS_ADDR          (0x4A)
#define E2V_ADS_ADDR            (0x4B)

#define FIAGARO2600_ADC_CHL     (3)
#define PID_ADC_CHL             (2)
#define FIAGARO2602_ADC_CHL     (2)
// #define CO_X4_ADC_PIN      (0x4A)
#define E2V_ADC_CHL             (0)

void setup()
{
    // put your setup code here, to run once:
    pinMode(A0, INPUT);

#if ENABLE_SERIAL_LOG
    Serial.begin(9600);
#endif // ENABLE_SERIAL_LOG

#if ENABLE_SDCARD_LOG
    Wire.begin();
    s3.begin(0x31);
#endif // ENABLE_SDCARD_LOG

    if (!bme.begin(0x76))
    {
        Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
        while (1)
            ;
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
    delay(1000);

#if ENABLE_SDCARD_LOG
    if (!sd.begin(SD_CONFIG))
    {
        sd.initErrorHalt();
    }
#endif // ENABLE_SDCARD_LOG
}

void loop()
{
    /*if (!file.open("EnviroSense.txt", O_RDWR | O_CREAT)) {
      sd.errorHalt(F("open failed"));
    }*/
#if ENABLE_SDCARD_LOG
    file.open("EnviroSense.txt", FILE_WRITE);
#endif // ENABLE_SDCARD_LOG

    // put your main code here, to run repeatedly:
    float input_volt = 0;
    float fig_val1 = 0;
    float fig_val2 = 0;
    uint16_t PID_val = 0;
    uint16_t e2v_val = 0;
    uint16_t co2 = 0;
    float CO_val = 0;
    
    counter++;

    // check input voltage
    input_volt = (analogRead(A0) * 5.02 * 5.0) / 1023.0;
    // check figaro2600 no pot
    fig_val1 = figaro(FIAGARO2600_ADS_ADDR, FIAGARO2600_ADC_CHL); // addr GND
    // check figaro2602 no pot
    fig_val2 = figaro(FIAGARO2602_ADS_ADDR, FIAGARO2602_ADC_CHL); // addr +5
    // check PID // check floating value + mq-131
    PID_val = read_ads1115(PID_ADS_ADDR, PID_ADC_CHL); // addr GND
    // check e2v
    e2v_val = read_ads1115(E2V_ADS_ADDR, E2V_ADC_CHL); // addr SCL
    // check S300
    co2 = s3.getCO2ppm();
    // check CO-X4
    CO_val = CO_X4(0x4A); // addr SDA

#if ENABLE_SERIAL_LOG
    Serial.print("[");
    Serial.print(counter);
    Serial.print("] ");
    Serial.print("Volt: ");
    Serial.print(input_volt);
    Serial.print(" , ");
    Serial.print("Fig2600: ");
    Serial.print(fig_val1);
    Serial.print(" , ");
    Serial.print("Fig2602: ");
    Serial.print(fig_val2);
    Serial.print(" , ");
    Serial.print("PID: ");
    Serial.print(PID_val);
    Serial.print(" , ");
    Serial.print("e2v: ");
    Serial.print(e2v_val);
    Serial.print(" , ");
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" , ");
    Serial.print("CO: ");
    Serial.print(CO_val);
    Serial.print(" , ");
#endif //ENABLE_SERIAL_LOG

#if ENABLE_SDCARD_LOG
    file.print(counter);
    file.print(",");
    file.print(input_volt);
    file.print(",");
    file.print(fig_val1);
    file.print(",");
    file.print(fig_val2);
    file.print(",");
    file.print(PID_val);
    file.print(",");
    file.print(e2v_val);
    file.print(",");
    file.print(co2);
    file.print(",");
    file.print(CO_val);
    file.print(",");
#endif //ENABLE_SDCARD_LOG 

#if ENABLE_BMS
    BME680();
#endif

#if ENABLE_SERIAL_LOG
    Serial.println();
#endif

#if ENABLE_SDCARD_LOG
    file.println();
    file.close();
#endif

    delay(500);
    // file.rewind();

    // while (file.available()) {
    // Serial.write(file.read());
    //}
}

void BME680()
{
#if ENABLE_SERIAL_LOG
    Serial.print("Temperature: ");
    Serial.print(bme.temperature);
    Serial.print(" C");
    Serial.print(" , ");

    Serial.print("Pressure: ");
    Serial.print(bme.pressure / 100.0);
    Serial.print(" hPa");
    Serial.print(" , ");

    Serial.print("Humidity: ");
    Serial.print(bme.humidity);
    Serial.print(" %");
    Serial.print(" , ");

    Serial.print("Gas: ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.print(" KOhms");
    Serial.print(" , ");

    Serial.print("Altitude: ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print(" m");
    Serial.print("\t");
#endif //ENABLE_SERIAL_LOG

#if ENABLE_SDCARD_LOG
    file.print(bme.temperature);
    file.print(" C");
    file.print(",");

    file.print(bme.pressure / 100.0);
    file.print(" hPa");
    file.print(",");

    file.print(bme.humidity);
    file.print(" %");
    file.print(",");

    file.print(bme.gas_resistance / 1000.0);
    file.print(" KOhms");
    file.print(",");

    file.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    file.print(" m");
#endif //ENABLE_SDCARD_LOG
}

uint16_t read_ads1115(int addr, int pin)
{
    if (!ads.begin(addr))
    {
        Serial.print("Failed to initialize AD ");
        Serial.println(addr);
        return 0;
    }
    return ads.readADC_SingleEnded(pin);
}

float CO_X4(int addr)
{
    int16_t worker, Aux;
    float multiplier = 0.1875F; // ADS1115  @ +/- 6.144V gain (16-bit results)

    if (!ads.begin(addr))
    {
        Serial.print("Failed to initialize AD ");
        Serial.println(addr);
        return 0;
    }

    worker = ads.readADC_Differential_0_1();
    Aux = ads.readADC_Differential_2_3();
    float result = worker - Aux;
    // Serial.print("Result: ");
    // Serial.print(result);
    // Serial.print("(");
    // Serial.print(result * multiplier);
    // Serial.println("mV)");
    return result;
}

float figaro(int addr, int pin)
{

    float figaro_volts = 0.0;
    float contaminants = 0.0;
    float sum = 0.0;
    float Vsum = 0.0;
    int n = 0;
    int avg = 20;
    int16_t adc = 0;

    if (!ads.begin(addr))
    {
        Serial.print("Failed to initialize AD ");
        Serial.println(addr);
        return 0;
    }

    while (n != avg)
    {
        adc = ads.readADC_SingleEnded(pin);
        figaro_volts = ads.computeVolts(adc);
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
