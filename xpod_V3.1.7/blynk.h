#define BLYNK_TEMPLATE_ID "TMPL2-_U7f2uX"
#define BLYNK_TEMPLATE_NAME "Simman3"
#define BLYNK_AUTH_TOKEN "sKPYgn3jcuimveDEcJSmUGmeBFd-LgG5"

#include <Arduino.h>
#include "SoftwareSerial.h"
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include "bme_module.h"
#include "ads_module.h"
#include "gps_module.h"
#include "mq131_module.h"
#include "pms_module.h"
#include "s300i2c.h"

char ssid[] = "Pls_no_hack";
char pass[] = "BMTC_8922";

// char ssid[] = "OnePlus Nord";
// char pass[] = "Jagatguru";
extern gps;
#define BLYNK_PRINT Serial

#define EspSerial Serial3

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

#define DEBUG true

ESP8266 wifi(&EspSerial);


// Send AT commands to module with timeout handling
String sendDataWithTimeout(String command, const int timeout, boolean debug) {
  String response = "";
  EspSerial.print(command);
  long int startTime = millis(); 
  if (millis() - startTime < timeout) { // changed loop to condition
    if (EspSerial.available()) { // changed loop to condition
      char c = EspSerial.read();
      response += c;
    }
    if (response.endsWith("OK\r\n")) {
      //break; // Command succeeded
    }
  }
  if (debug) {
    Serial.print("Wifi debug Response :");
    Serial.print(response);
    Serial.println();
  }
  return response;
}

void InitWifiModule() {
  sendDataWithTimeout("AT+RST\r\n", 2000, DEBUG); // reset
  sendDataWithTimeout("AT+CWJAP=\"Pls_no_hack\",\"BMTC_8922\"\r\n", 5000, DEBUG); // Connect network (increased timeout)
  delay(3000);
  sendDataWithTimeout("AT+CWMODE=1\r\n", 1000, DEBUG); // Set the module's operating mode to station mode
  sendDataWithTimeout("AT+CIFSR\r\n", 1000, DEBUG); // Show IP Address
  sendDataWithTimeout("AT+CIPMUX=1\r\n", 1000, DEBUG); // Multiple connections
  sendDataWithTimeout("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // Start comm port 80
}


String BME_Module::read4blynk()
{
  String bms_data_str;

  if (!status)
    return "";

  Blynk.virtualWrite(V0, String(bme_sensor.temperature));
  Blynk.virtualWrite(V1, String(bme_sensor.pressure / 100.0));
  Blynk.virtualWrite(V2, String(bme_sensor.humidity));
  Blynk.virtualWrite(V3, String(bme_sensor.gas_resistance / 1000.0));
  Blynk.virtualWrite(V4, String(bme_sensor.readAltitude(SEALEVELPRESSURE_HPA)));

   Blynk.virtualWrite(V0, String(bme_sensor.temperature));
  Blynk.virtualWrite(V1, String(bme_sensor.pressure / 100.0));
  Blynk.virtualWrite(V2, String(bme_sensor.humidity));
  Blynk.virtualWrite(V3, String(bme_sensor.gas_resistance / 1000.0));
  Blynk.virtualWrite(V4, String(bme_sensor.readAltitude(SEALEVELPRESSURE_HPA)));

  return bms_data_str;
}

String ADS_Module::read4blynk()
{
  String out_str = "";

  Blynk.virtualWrite(V5, String(read_figaro(ADS_SENSOR_FIG2600)));
  Blynk.virtualWrite(V6, String(read_figaro(ADS_SENSOR_FIG2602)));
#if FIGARO3_ENABELD
  Blynk.virtualWrite(V7, String(read_figaro(ADS_SENSOR_FIG3)));     
#endif
#if FIGARO4_ENABELD
  Blynk.virtualWrite(V8, String(read_figaro(ADS_SENSOR_FIG4))); 
#endif
  Blynk.virtualWrite(V9, String(read_raw(ADS_SENSOR_PID)));
  Blynk.virtualWrite(V10, String(read_raw(ADS_SENSOR_E2V)));
  Blynk.virtualWrite(V11, String(read_co()));
 
  return out_str;
}
String S300I2C::read4blynk()
{
   String Co2data = "";
   Blynk.virtualWrite(V12,String(getCO2ppm()));
   return Co2data;
}


String GPSModule::read4blynk()
{
  String gpsdata = "";
  #if GPS_ENABLED
    String out = readData();
    // Blynk.virtualWrite(V5, out);
    gps val;
    Blynk.virtualWrite(V13, String(val.latitude));
    Blynk.virtualWrite(V14, String(val.longitude));
    Blynk.virtualWrite(V15, String(val.altitude));
    Blynk.virtualWrite(V16, String(val.course));
    Blynk.virtualWrite(V17, String(val.speed));
  #endif
  return gpsdata;
}

String MQ131_Module::read4blynk()
{
  String mq_data;

  if (!status)
    return "";

  Blynk.virtualWrite(V18, String(ads_module.readADC_SingleEnded(MQ131_I2C_CHL)));

// #if READ_JUST_RAW
//   // mq_data = String(ads_module.readADC_SingleEnded(MQ131_I2C_CHL));
//   Blynk.virtualWrite(V17, String(ads_module.readADC_SingleEnded(MQ131_I2C_CHL)));
// #else
//   // mq_data = String(this->read()) + "," + String(raw_data);
//   // Blynk.virtualWrite(V18, String(this->read()) + "," + String(raw_data));
// #endif

  return mq_data;
}

String PMS_Module::read4blynk()
{
  String pms_data_str;
  PM25_AQI_Data data;
  int read_tries = 3;

  if (!status)
    return "";

  // pms_data_str = String(data.pm10_env) + ",";
  Blynk.virtualWrite(V19, String(data.pm10_env));
  // pms_data_str += String(data.pm25_env) + ",";
  Blynk.virtualWrite(V20, String(data.pm25_env));
  // pms_data_str += String(data.pm100_env) + ",";
  Blynk.virtualWrite(V21, String(data.pm100_env));

  // pms_data_str += String(data.particles_03um) + ",";
  Blynk.virtualWrite(V22, String(data.particles_03um));
  // pms_data_str += String(data.particles_05um) + ",";
  Blynk.virtualWrite(V23, String(data.particles_05um));
  // pms_data_str += String(data.particles_10um) + ",";
  Blynk.virtualWrite(V24, String(data.particles_10um));
  // pms_data_str += String(data.particles_25um) + ",";
  Blynk.virtualWrite(V25, String(data.particles_25um));
  // pms_data_str += String(data.particles_50um) + ",";
  Blynk.virtualWrite(V26, String(data.particles_50um));
  // pms_data_str += String(data.particles_100um);
  Blynk.virtualWrite(V27, String(data.particles_100um));

  return pms_data_str;
}

