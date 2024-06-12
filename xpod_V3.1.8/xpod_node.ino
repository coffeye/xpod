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
#include <SdFat.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "s300i2c.h"
#include "digipot.h"
#include "ads_module.h"
#include "bme_module.h"
#include <SchedTask.h>

/*************  Global Declarations  *************/
unsigned int CO2=0;

// Modules
S300I2C co2_sensor(Wire);
ADS_Module ads_module;
BME_Module bme_module;


// xpod id
String xpodID = "XPODv4"; 
String fileName;
SdFat sd;
SdFile file;

#if WIFI_ENABLED
  #include "blynk.h"
#endif

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
#include "wind_vane.h"
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



void logDataToSdCard() 
{

  char fileNameArray[fileName.length()+1];
  fileName.toCharArray(fileNameArray, sizeof(fileNameArray));
  bme_module.read4print(); // This is necessary to make the bme read for sd logging for some reason

  float in_volt_val;

  #if OPC_ENABLED
    opcData = opc.getData();
  #endif

  in_volt_val = (analogRead(IN_VOLT_PIN) * 5.02 *5 ) / 1023.0;
  #if SERIAL_LOG_ENABLED
    if(!Serial) 
    { 
      //check if Serial is available... if not,
      Serial.end();      // close serial port
      delay(100);        //wait 100 millis
      Serial.begin(9600); // reenable serial again
    }

    #if RTC_ENABLED
      {
      rtc_date_time = rtc.now();
      
      if((rtc_date_time.year() <= 2023) || (rtc_date_time.year() > 2030))
      {
        Serial.print("Invalid Date reinitialize the date and time");
        Serial.print(",");
        digitalWrite(STATUS_HALTED,HIGH);
        delay(300);
        digitalWrite(STATUS_HALTED,LOW);
        delay(300);
      }
      else
      {
        Serial.print(rtc_date_time.timestamp());
        Serial.print(",");
      }
     }
    #endif 

    Serial.print("Volt:");
    Serial.print(in_volt_val);
    Serial.print(",");
  
    Serial.print(ads_module.read4print_raw());

    #if WIFI_ENABLED
      ads_module.read4blynk();
    #endif

    Serial.print(",");
  
    Serial.print("CO2:");
    Serial.print(co2_sensor.getCO2ppm());

    #if WIFI_ENABLED
      co2_sensor.read4blynk();
    #endif

    Serial.print(",");
  
    Serial.print(bme_module.read4print());

    #if WIFI_ENABLED
      bme_module.read4blynk();
    #endif

    Serial.print(",");

    #if QUAD_ENABLED
      Serial.print(quad_module.read());
      Serial.print(",");
    #endif 

    #if MQ131_ENABLED
      Serial.print(mq131_module.read4print());
      Serial.print(",");

      #if WIFI_ENABLED
        mq131_module.read4blynk();
      #endif

    #endif
    
    #if PMS_ENABLED
      Serial.print(pms_module.read4print());

      #if WIFI_ENABLED
        pms_module.read4blynk();
      #endif

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
        #if WIFI_ENABLED
          gpsModule.read4blynk();
        #endif
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

  #if SDCARD_LOG_ENABLED
   digitalWrite(SD_CARD_CS_PIN,LOW);
   while(!sd.begin(SD_CARD_CS_PIN))
  {
      #if SERIAL_LOG_ENABLED
      Serial.println("error: SD card init failed in loop");
      #endif
      sd.begin(SD_CARD_CS_PIN);
      // just to indicates sd card fail to init
      digitalWrite(STATUS_HALTED,HIGH);
      delay(100);
      digitalWrite(STATUS_HALTED,LOW);
      delay(100);  
  }
       

    if (sd.begin(SD_CARD_CS_PIN) &&  file.open(fileNameArray, O_CREAT | O_APPEND | O_WRITE))
    {
      #if RTC_ENABLED
      {
        rtc_date_time = rtc.now();
        if((rtc_date_time.year() == 2000) || (rtc_date_time.year() == 2099))
        {
          file.print("\r\n");
          file.print("Invalid Date reinitialize the date and time");
          file.print(",");
          digitalWrite(STATUS_HALTED,HIGH);
          delay(300);
          digitalWrite(STATUS_HALTED,LOW);
          delay(300);
        }
        else{
          file.print("\r\n");
          file.print(rtc_date_time.timestamp());
          file.print(",");
        }
      }
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
      //just indicate SD CARD writing
        digitalWrite(STATUS_RUNNING,HIGH);
        _delay_ms(100);
        digitalWrite(STATUS_RUNNING,LOW);

      file.close();
    }
    else
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Failed to open SD CARD");
      #endif
      digitalWrite(SD_CARD_CS_PIN,HIGH);
      
      //just to indicate SD CARD not writing
      digitalWrite(STATUS_HALTED,HIGH);
      delay(200);
      digitalWrite(STATUS_HALTED,LOW);
      delay(1000);
    }
  #endif //SDCARD_LOG_ENABLED
}


// blynk task checks for connection, if fails try to connect
void blynktask(){
  #if WIFI_ENABLED
    bool status = Blynk.run();
    if(!status)
    {
      // Set ESP8266 baud rate
      EspSerial.begin(ESP8266_BAUD);
      InitWifiModule();
      delay(10);
      Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
      status = Blynk.run();
    }
  #endif
}

// Blynk task 
SchedTask blynkTask(2000, 3000, blynktask); // define the blynkTask task (dispatch in 2 sec, every 3 sec)

// sdcard task
SchedTask sdCardTask(0, 3000, logDataToSdCard); // define the sdCardTask task (dispatch now, every 3 sec)

void setup()
{
  #if SERIAL_LOG_ENABLED
  Serial.begin(9600);
  Serial.println();
  #endif
  
  #if WIFI_ENABLED
  {
     // Set ESP8266 baud rate
    EspSerial.begin(ESP8266_BAUD);
    InitWifiModule();
    delay(10);
    Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
  }
  #endif
  
  // In voltage
  pinMode(IN_VOLT_PIN, INPUT);

  // Motor control
  pinMode(MOTOR_CTRL_IN_PIN, INPUT);
  pinMode(MOTOR_CTRL_OUT_PIN, OUTPUT);
  pinMode(SD_CARD_CS_PIN, OUTPUT);

 // status LEDs
  pinMode(STATUS_RUNNING, OUTPUT);
 // pinMode(STATUS_ERROR, OUTPUT); // pin needs to be changed
  pinMode(STATUS_HALTED, OUTPUT);

  Wire.begin();
    
  #if RTC_ENABLED 
    if (!rtc.begin())
    {
      #if SERIAL_LOG_ENABLED
        Serial.println("Error: Failed to initialize RTC module");
      #endif
      //just to indicate RTC not initialized
        digitalWrite(STATUS_HALTED,HIGH);
        delay(1000);
        digitalWrite(STATUS_HALTED,LOW);
        delay(1000);

    }
    else
    {
      // uncomment this line give current date and time if needed
       rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));    // Only run uncommented once to initialize RTC
      rtc_date_time = rtc.now();

    }
     if(rtc.lostPower())
     {
       #if SERIAL_LOG_ENABLED
         Serial.println("Error: RTC module power lost, change battery");
       #endif
       //just to indicate rtc power lost
         digitalWrite(STATUS_HALTED,HIGH);
         delay(300); //delay for 300msec
         digitalWrite(STATUS_HALTED,LOW);
         delay(300);
     }
    
  #endif

  #if SDCARD_LOG_ENABLED
     digitalWrite(SD_CARD_CS_PIN,LOW);

  DateTime now = rtc.now();
  fileName = xpodID + "_" +  String(now.month()) + "_" + String(now.day()) + "_"  + String(now.year()) + ".txt";
  char fileNameArray[fileName.length()+1];
  fileName.toCharArray(fileNameArray, sizeof(fileNameArray)); //Well damn, that function is nice.


   if(!sd.begin(SD_CARD_CS_PIN))
  {
    #if SERIAL_LOG_ENABLED
    Serial.println("insert sd card to begin");
    #endif
    sd.begin(SD_CARD_CS_PIN);
    // just to indicate sd card fail to init
    digitalWrite(STATUS_HALTED,HIGH);
    delay(1000);
    digitalWrite(STATUS_HALTED,LOW);
    delay(1000);  
  }
  digitalWrite(SD_CARD_CS_PIN,HIGH);
  SPI.transfer(0);

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

  delay(10000);
  wdt_enable(WDTO_8S);


}


void loop()
{
  wdt_reset();
  unsigned long long int startLoop = millis();
  
  // Dispatches the tasl to log data to sd card and blynk server 
  SchedBase::dispatcher();

  // Motor control
  int motor_ctrl_val = analogRead(MOTOR_CTRL_IN_PIN);
  motor_ctrl_val = (((float)motor_ctrl_val / 1024) * 255);

  //motor_ctrl_val = 200; // use this to hardcode the motor speed; the number ranges from 0-255
  analogWrite(MOTOR_CTRL_OUT_PIN, motor_ctrl_val);

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
 // digitalWrite(STATUS_RUNNING, LOW);
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
