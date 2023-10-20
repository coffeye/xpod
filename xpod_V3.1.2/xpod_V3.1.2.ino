/*******************************************************************************
 * @project Hannigan Lab's Next Gen. Air Quality Pods
 *
 * @file    xpod_node.cpp
 * @brief   Collects data from sensors over ADC-16bit module and logs the data
 *          on both serial monitor and the SD card.
 *
 * @author 	Rohan Jha
 * @editor  Percy Smith
 * @date 	  October 18, 2023
 * @log     Incorporates OPC
 *          Contains CO Fix
 *          Fixes File Name Issue
 ******************************************************************************/
#include "xpod_node.h"

#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <s300i2c.h>
#include <avr/wdt.h>

#include "digipot.h"
#include "ads_module.h"
#include "bme_module.h"

String xpodID = "OPOD12";
SdFat sd;
SdFile file;

#if GPS_ENABLED
#include "gps_module.h"
GPS_Module gps_module;
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

#if MQ_ENABLED
#include "mq_module.h"
MQ_Module mq_module;
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

#if MET_ENABLED
  #include "wind_vane.h"
  long lastWindCheck = 0;
  volatile long lastWindIRQ = 0;
  volatile byte windClicks = 0;
  wind_vane windVane;
#endif

/*************  Global Declarations  *************/
// Modules
S300I2C co2_sensor(Wire);
ADS_Module ads_module;
BME_Module bme_module;

// Variables
String fileName;

/******************  Functions  ******************/
#if MET_ENABLED
  void wspeedIRQ()
  {
    if(millis() - lastWindIRQ > 10)
    {
      lastWindIRQ = millis();
      windClicks++;
    }
  }
#endif

void setup()
{
  #if SERIAL_LOG_ENABLED
  Serial.begin(9600);
  Serial.println();
  #endif
  
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
    if (!sd.begin(SD_CARD_CS_PIN)) {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Card failed, or not present");
      #endif
      digitalWrite(SD_CARD_CS_PIN,HIGH);
      // while(1);
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
  
  #if MQ_ENABLED
    if (!mq_module.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize MQ sensor!");
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
    if (!gps_module.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize GPS module!");
      #endif
    }
  #endif

  #if MET_ENABLED
    attachInterrupt(digitalPinToInterrupt(3), wspeedIRQ, FALLING);  // attaching wind speed interrupt to pin 3
  #endif

  initpots();
  DownPot(0);
  DownPot(1);
  //DownPot(2);
  SetPotLevel(2, 80);

  delay(1000);
  file.close();
  
  //delay(10000);
  wdt_enable(WDTO_8S);
  //Serial.println("reset");
}

void loop()
{
  wdt_reset();
  unsigned long long int startLoop = millis();
  int motor_ctrl_val;
  float in_volt_val;

  #if OPC_ENABLED
    opcData = opc.getData();
  #endif

  in_volt_val = (analogRead(IN_VOLT_PIN) * 5.02 * 5) / 1023.0; //Follow up with rylee
    #if MET_ENABLED
      float wind_speed = get_wind_speed();
      float wind_dir_volt = windVane.get_direction();
      float wind_dir_degree = windVane.degree_direction(wind_dir_volt);
      String wind_dir_cardinal = windVane.cardinal_direction(wind_dir_volt);
    #endif //MET_ENABLED Data Gathering

  #if SERIAL_LOG_ENABLED
    digitalWrite(SD_CARD_CS_PIN,LOW);
    if(!Serial) {  //check if Serial is available... if not,
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
    Serial.print(",");
  
    Serial.print("CO2:");
    Serial.print(co2_sensor.getCO2ppm());
    Serial.print(",");
  
    Serial.print(bme_module.read4print());
    Serial.print(",");

    #if QUAD_ENABLED
      Serial.print(quad_module.read());
      Serial.print(",");
    #endif 

    #if MQ_ENABLED
      Serial.print(mq_module.read4print());
      Serial.print(",");
    #endif
    
    #if PMS_ENABLED
      Serial.print(pms_module.read4print());
    #endif

    #if OPC_ENABLED
      Serial.print(opc.read4print(opcData));
    #endif

    #if GPS_ENABLED
      Serial.print(gps_module.get_gps_info_serial());
      Serial.print(",");
    #endif 
      
    #if MET_ENABLED
      Serial.print("Wind Speed: " + String(wind_speed));
      Serial.print(", ");
      Serial.print("Wind Direction: " + String(wind_dir_degree) + " (" + wind_dir_cardinal + ")");
      Serial.print(", ");
    #endif

  #endif  //SERIAL_LOG_ENABLED
  bme_module.read4print(); // This is necessary to make the bme read for sd logging for some reason

  #if SDCARD_LOG_ENABLED
    digitalWrite(STATUS_RUNNING, HIGH);
    digitalWrite(SD_CARD_CS_PIN,LOW);

    DateTime now = rtc.now();
    fileName = xpodID + "_" + String(now.year()) + "_" + String(now.month()) + "_" + String(now.day()) + ".txt";
    char fileNameArray[fileName.length()+1];
    fileName.toCharArray(fileNameArray, sizeof(fileNameArray)); //Well damn, that function is nice.
    file.open(fileNameArray, O_CREAT | O_APPEND | O_WRITE);

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
    
      #if MET_ENABLED
        file.print(String(wind_speed));
        file.print(",");
        file.print(String(wind_dir_degree));
        file.print(",");
      #else
        file.print(",,");
      #endif

      #if MQ_ENABLED
        file.print(mq_module.read4sd());
        file.print(",");
      #else
        file.print(",");
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
        file.print(gps_module.get_gps_info_sd());
        file.print(gps_module.get_gps_dtinfo());
        file.print(",");
      #else
        // file.print(",,,,,,,,");
      #endif
    
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

  // // Motor control
  motor_ctrl_val = analogRead(MOTOR_CTRL_IN_PIN);
  motor_ctrl_val = (((float)motor_ctrl_val / 1024) * 255);
  //motor_ctrl_val = 200; // use this to hardcode the motor speed; the number ranges from 0-255
  analogWrite(MOTOR_CTRL_OUT_PIN, motor_ctrl_val);
  delay(2000);

  // // Status indication

  // // This all controls how long the loop lasts
  // if((SAMPLE_TIME * 1000) - (millis() - startLoop) >= 0){
  //   int n = floor(((SAMPLE_TIME * 1000.0) - (millis() - startLoop)) / 6000.0);
  //   for(int i = 0; i < n; i++){  
  //     wdt_reset();
  //     delay(6000);
  //   }
  //   wdt_reset();
  //   delay((SAMPLE_TIME * 1000) - (millis() - startLoop));
  // }else{
  //   delay(100);
  // }
  digitalWrite(STATUS_RUNNING, LOW);
  Serial.print("\n");
}
#if MET_ENABLED
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
