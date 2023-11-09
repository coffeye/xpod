/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.cpp
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	1. Rohan Jha 
 * @author  2. Malola Simman 
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
#include <avr/wdt.h>

#include "s300i2c.h"
#include "digipot.h"
#include "ads_module.h"
#include "bme_module.h"
#include "blynk.h"
#include <avr/sleep.h>

unsigned int CO2=0;
/*************  Global Declarations  *************/
// Modules
S300I2C co2_sensor(Wire);
ADS_Module ads_module;
BME_Module bme_module;


// File Variables
File file;
// xpod id
String xpodID = "XPOD"; 
String fileName;

#if GPS_ENABLED
#include "gps_module.h"
GPSModule gpsModule;
#endif

#if RTC_ENABLED
#include <RTClib.h>
RTC_DS3231 rtc;
DateTime rtc_date_time;
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

#if OPC_ENABLED
#include "OPC.h"
OPC opc;
particleData opcData;
#endif


/******************  Functions  ******************/
#if MET_STATION
//Wind direction sensor(Potentiometer) on analog pin 15
const byte WDIR = A15;
//Wind speed variables
long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;
void wspeedIRQ()  {
  if (millis() - lastWindIRQ > 10)  { // Ignore switch-bounce glitches less than 10ms (142MPH max reading) after the reed switch closes
    lastWindIRQ = millis(); //Grab the current time
    windClicks++; //There is 1.492MPH for each click per second.
  }
}
#endif

void setup()
{
  #if SERIAL_LOG_ENABLED
  Serial.begin(9600);
  Serial.println();
  #endif
  
  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  InitWifiModule();
  delay(10);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);

  // In voltage
  pinMode(IN_VOLT_PIN, INPUT);

  // Motor control
  pinMode(MOTOR_CTRL_IN_PIN, INPUT);
  pinMode(MOTOR_CTRL_OUT_PIN, OUTPUT);
  pinMode(SD_CARD_CS_PIN, OUTPUT);

  // Status LEDs
  pinMode(STATUS_RUNNING, OUTPUT);
  pinMode(STATUS_ERROR, OUTPUT);
  pinMode(STATUS_HALTED, OUTPUT);

  Wire.begin();
  #if SDCARD_LOG_ENABLED
    digitalWrite(STATUS_HALTED, HIGH);
    if (!SD.begin(SD_CARD_CS_PIN)) {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Card failed, or not present");
      #endif
      digitalWrite(SD_CARD_CS_PIN,HIGH);
    }

  #endif
  
  SPI.transfer(0);
  #if RTC_ENABLED 
    if (!rtc.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize RTC module");
      #endif
    }
    else{
      //rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));    // Only run uncommented once to initialize RTC
      rtc_date_time = rtc.now();
    }
  #endif

  if (!co2_sensor.begin(CO2_I2C_ADDR))
  {
    #if SERIAL_LOG_ENABLED
      Serial.println("Error: Failed to initialize CO2 sensor!");
    #endif
  }


  if (!bme_module.begin())
  {
    #if SERIAL_LOG_ENABLED
      Serial.println("Error: Failed to initialize BME sensor!");
    #endif
  }

  if (!ads_module.begin())
  {
    #if SERIAL_LOG_ENABLED
      Serial.println("Error: Failed to initialize one of the ADS1115 module!");
    #endif
  }
  
  #if QUAD_ENABLED
    if (!quad_module.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize Quad Stat!");
      #endif
    }
  #endif
  
  #if MQ131_ENABLED
    if (!mq131_module.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize MQ131 sensor!");
      #endif

    }
  #endif
  
  #if PMS_ENABLED
    if (!pms_module.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize PM sensor!");
      #endif
    }
  #endif

  #if OPC_ENABLED
    if(!opc.begin()){
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize OPC!");
      #endif
    }
  #endif

  #if GPS_ENABLED
    if (!gpsModule.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize GPS module!");
      #endif
    }
  #endif

  #if MET_STATION
    attachInterrupt(digitalPinToInterrupt(3), wspeedIRQ, FALLING);  // attaching wind speed interrupt to pin 3
  #endif

  initpots();
  DownPot(0);
  DownPot(1);
  //DownPot(2);
  SetPotLevel(2, 80);

  delay(1000);
  file.flush();
  file.close();

  delay(10000);
  wdt_enable(WDTO_8S);

}

void loop()
{
  wdt_reset();
  unsigned long long int startLoop = millis();
  int motor_ctrl_val;
  float in_volt_val;

  Blynk.run();
  
  #if OPC_ENABLED
    opcData = opc.getData();
  #endif

  in_volt_val = (analogRead(IN_VOLT_PIN) * 5.02 * 5) / 1023.0;//Follow up with rylee

  #if SERIAL_LOG_ENABLED
    digitalWrite(SD_CARD_CS_PIN,LOW);
    if(!Serial) 
    {  //check if Serial is available... if not,
      Serial.end();      // close serial port
      delay(100);        //wait 100 millis
      Serial.begin(9600); // reenable serial again
    }

    #if RTC_ENABLED
      rtc_date_time = rtc.now();
      Serial.print(rtc_date_time.timestamp());
      Serial.print(",");
    #endif 

    Serial.print("Volt:");
    Serial.print(in_volt_val);
    Serial.print(",");
  
    Serial.print(ads_module.read4print_raw());
    ads_module.read4blynk();
    Serial.print(",");
  
    Serial.print("CO2:");
    Serial.print(co2_sensor.getCO2ppm());
    co2_sensor.read4blynk();
    Serial.print(",");
  
    Serial.print(bme_module.read4print());
    bme_module.read4blynk();
    Serial.print(",");

    #if QUAD_ENABLED
      Serial.print(quad_module.read());
      Serial.print(",");
    #endif 

    #if MQ131_ENABLED
      Serial.print(mq131_module.read4print());
      Serial.print(",");
      mq131_module.read4blynk();
    #endif
    
    #if PMS_ENABLED
      Serial.print(pms_module.read4print());
      pms_module.read4blynk();
            Serial.print(",");

    #endif

    #if OPC_ENABLED
      Serial.print(opc.read4print(opcData));
            Serial.print(",");

    #endif

    #if GPS_ENABLED
      if(gpsModule.isDataAvailable()){
        Serial.print(gpsModule.readData());
        Serial.print(",");
        gpsModule.read4blynk();
      }
      else{
        Serial.print("GPS connection fails");
        Serial.print(",");
      }
      
    #endif 
      
    #if MET_STATION
      String data = "Wind Speed"+String(get_wind_speed()) + "," + String(analogRead(A15)) ;
      Serial.print(data);
    #endif

  #endif  //SERIAL_LOG_ENABLED
  bme_module.read4print(); // This is necessary to make the bme read for sd logging for some reason

  #if SDCARD_LOG_ENABLED
    digitalWrite(SD_CARD_CS_PIN,LOW);
    DateTime now = rtc.now();
    fileName = "x_" + String(now.month()) + "_" + String(now.day()) + ".txt";
   // fileName = "x_" + String(now.year()) + "_" + String(now.month()) + "_" + String(now.day()) + ".txt";
    char fileNameArray[fileName.length()+1];
    fileName.toCharArray(fileNameArray, sizeof(fileNameArray));
    
    file = SD.open(fileNameArray, FILE_WRITE);

    if (file)
    {
      #if RTC_ENABLED
        rtc_date_time = rtc.now();
        file.print("\r\n");
        file.print(rtc_date_time.timestamp());
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
    
      #if QUAD_ENABLED
        file.print(quad_module.read());
        file.print(",");
      #else
        file.print(",,,,,,,,");
      #endif
    
      #if MQ131_ENABLED
        file.print(mq131_module.read4sd());
        file.print(",");
      #else
        file.print(",,");
      #endif
      
    
      #if PMS_ENABLED
        file.print(pms_module.read4sd());
      #else
        file.print(",,,,,,,,,");
      #endif

      #if OPC_ENABLED
        file.print(opc.read4sd(opcData));
      #else
        file.print(",,,,,");
      #endif
    
      #if GPS_ENABLED
      if(gpsModule.isDataAvailable()){
        file.print(gpsModule.readData());
      }
      else{
        file.print(",,,,,");
      }
        
        
      #else
        file.print(",,,,,,,,");
      #endif
      
    
      #if MET_STATION
        data = String(get_wind_speed()) + "," + String(analogRead(A15)) ;
        file.print(data);
      #else
        file.print(",,");
      #endif
    
      file.flush();
      file.close();
    }
    else
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Failed to open SD CARD");
      #endif
      digitalWrite(SD_CARD_CS_PIN,HIGH);
      // while(1);
    }
  #endif //SDCARD_LOG_ENABLED

  // Motor control
  motor_ctrl_val = analogRead(MOTOR_CTRL_IN_PIN);
  motor_ctrl_val = (((float)motor_ctrl_val / 1024) * 255);
  //motor_ctrl_val = 200; // use this to hardcode the motor speed; the number ranges from 0-255
  analogWrite(MOTOR_CTRL_OUT_PIN, motor_ctrl_val);

  // Status indication
  digitalWrite(STATUS_RUNNING, HIGH);
  // This all controls how long the loop lasts
  if((SAMPLE_TIME * 1000) - (millis() - startLoop) >= 0)
  {
    int n = floor(((SAMPLE_TIME * 1000.0) - (millis() - startLoop)) / 6000.0);
    for(int i = 0; i < n; i++)
    {
      wdt_reset();
      delay(6000);
    }
    wdt_reset();
    delay((SAMPLE_TIME * 1000) - (millis() - startLoop));
  }
  else
  {
    delay(100);
  }
  digitalWrite(STATUS_RUNNING, LOW);
  Serial.print("\n");

}
#if MET_STATION
//Returns the instataneous wind speed
float get_wind_speed(){
  float deltaTime = millis() - lastWindCheck; //750ms

  deltaTime /= 1000.0; //Covert to seconds

  float windSpeed = (float)windClicks / deltaTime; //3 / 0.750s = 4

  windClicks = 0; //Reset and start watching for new wind
  lastWindCheck = millis();

  windSpeed *= 1.492; //4 * 1.492 = 5.968MPH

  return (windSpeed);
}
#endif
