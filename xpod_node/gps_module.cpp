/*******************************************************************************
 * @file    gps_module.cpp
 * @brief   Defines the GPS functions 
 *
 * @author  Malola Simman Srinivasan Kannan , masr4788@colorado.edu
 * @date    September 8 2023
 ******************************************************************************/
#include "gps_module.h"
#include <string.h>
#include <stdlib.h>

#define ARDUINO_GPS_RX 11
#define ARDUINO_GPS_TX 10

#define GPS_BAUDRATE 9600

GPSModule gpsModule1; // Create an instance of the GPSModule class

GPSModule::GPSModule() : gpsSerial(ARDUINO_GPS_RX, ARDUINO_GPS_TX) {
  gps_status = false;
}

bool GPSModule::begin() {
  gpsSerial.begin(GPS_BAUDRATE);

  gps_status = true;
  return gps_status;
 
}

bool GPSModule::isDataAvailable() {
  return gpsSerial.available();
}

String GPSModule::readData() {
  String data;

  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    static int i = 0; // Use static to retain i between loop iterations

    if (c == '$') {
      i = 0;
    }

    if (i < sizeof(strGPSData) - 1) {
      strGPSData[i++] = c; 
      strGPSData[i] = '\0'; // Null-terminate the string
    }

    if (c == '\n') {
      data = parseNMEASentence(strGPSData);
    }
  }
  
  return data;
}

String GPSModule::parseNMEASentence(const char* sentence)
{

  // Check if the sentence starts with "$GPRMC" or "$GPGGA"
  if (strncmp(sentence, "$GPRMC", 6) == 0) 
  {
    // Ensure the sentence length is within a reasonable range
    int sentenceLen = strlen(sentence);
    char* values[12];
    int valueIndex = 0;

    if (sentenceLen >= 50 && sentenceLen <= 80) {
      char* sentenceCopy = strdup(sentence); // Create a copy to avoid modifying the original string
      char* token = strtok(sentenceCopy, ",");
      while (token != NULL && valueIndex < 12) {
        values[valueIndex++] = token;
        token = strtok(NULL, ",");
      }
      free(sentenceCopy); // Free the copied string when done

      if (valueIndex >= 7) {
        char* latitudeValue = values[3];
        char* latitudeDirection = values[4];
        char* longitudeValue = values[5];
        char* longitudeDirection = values[6];
        char* speedValue = values[7]; // Speed over ground in knots

        // Convert latitude and longitude to decimal degrees based on direction
         latitude = atof(latitudeValue) /100;
         longitude = atof(longitudeValue)/100; 
        
        // Adjust latitude and longitude based on direction
        if (latitudeDirection[0] == 'S') {
          latitude = -latitude;
        }
        if (longitudeDirection[0] == 'W') {
          longitude = -longitude;
        }

        // Extract and validate course
        if (valueIndex >= 8) {
          char* courseValue = values[8];
          course = atof(courseValue);

          // Ensure course is within the valid range of 0 to 360 degrees
          while (course < 0.0) {
            course += 360.0;
          }
          while (course >= 360.0) {
            course -= 360.0;
          }

          // Extract and print speed
           speed = atof(speedValue);

        }
        
        
        // Extract and print date and time if available
        if (valueIndex >= 10) {
        String dateValue = values[8]; // Date in DDMMYY format
        char* time_c = values[1];
        char time[7];
        
        memcpy(time, time_c ,6);
        time[6] = '\0';
        String timeValue ; 
        for(int i=0;i<7;i++)
        {
            timeValue.concat(time[i]);
        }

          if (dateValue.length()== 6 && timeValue.length()>=7  ) {
            
            day = (dateValue.substring(0, 2)).toInt();
            month = (dateValue.substring(2, 4)).toInt();
            year = (dateValue.substring(4, 6)).toInt();
            hours=0;
            if(timeValue.substring(0,2).toInt() == 0){
              hours = (timeValue.substring(1, 2)).toInt();
            }
            else{
              hours = (timeValue.substring(0, 2)).toInt();
            }
            
            min = (timeValue.substring(2, 4)).toInt();
            sec = (timeValue.substring(4, 6)).toInt();
          
          } 
          else {
            Serial.println("GPS Invalid date or time format.");
          }
        } 
        else {
          Serial.println("GPS Date and time information missing.");
        }
      }
    }
  } 
  else if (strncmp(sentence, "$GPGGA", 6) == 0) {
    // Parse "$GPGGA" sentence for altitude
    int sentenceLen = strlen(sentence);
    char* values[15];
    int valueIndex = 0;

    if (sentenceLen >= 50 && sentenceLen <= 80) {
      char* sentenceCopy = strdup(sentence); // Create a copy to avoid modifying the original string
      char* token = strtok(sentenceCopy, ",");
      while (token != NULL && valueIndex < 15) {
        values[valueIndex++] = token;
        token = strtok(NULL, ",");
      }
      free(sentenceCopy); // Free the copied string when done

      if (valueIndex >= 15) {
        char* altitudeValue = values[9]; // Altitude above sea level (in meters)
        altitude = atof(altitudeValue);
      }
    }
  }
  String gpsDataString = "Lat: " + String(latitude)  ;
  gpsDataString += ",Long: " + String(longitude);
  gpsDataString += ",Course(degrees): " + String(course, 2) ;
  gpsDataString += ",Speed(knots): " + String(speed, 2) ;
  gpsDataString += ",Altitude(meters): " + String(altitude, 2)  ;
  gpsDataString += ",Date(DD/MM/YYYY): " + String(day) + "/" + String(month) + "/" + String(year) ;
  gpsDataString += ",Time(HH:MM:SS): " + String(hours) + ":" + String(min) + ":" + String(sec) ;
  gps gpsval={0};
  gpsval.altitude=altitude;
  gpsval.longitude=longitude;
  gpsval.course=course;
  gpsval.latitude = latitude;
  gpsval.speed = speed;

  return gpsDataString;
}
