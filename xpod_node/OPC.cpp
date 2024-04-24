/*
OPC.cpp

Written by Joseph Habeck (habec021@umn.edu) on 6/24/18. (https://github.com/JHabeck/Alphasense-OPC-N2/tree/master)
Edited using Marcel Oliveira's code from github (https://github.com/shyney7/OPC-R2_ESP32/tree/main)
Put together by Aidan Mobley

 */
#include <stdio.h>
#include "OPC.h"

// Combine two bytes into a 16-bit unsigned int
uint16_t OPC::twoBytes2int(byte LSB, byte MSB){
  uint16_t int_val = ((MSB << 8) | LSB);
  return int_val;
}

// Return an IEEE754 float from an array of 4 bytes
float OPC::fourBytes2float(byte val0, byte val1, byte val2, byte val3) {
  uint8_t bytes[4] = {uint8_t(val0), uint8_t(val1), uint8_t(val2), uint8_t(val3)};
  float result;
  memcpy(&result, bytes, 4);
  return result;
}

// Gets OPC ready to do something
bool OPC::getReady(const byte command){
  byte inData;
  SPI.beginTransaction(SPISettings(300000, MSBFIRST, SPI_MODE1));
  int tries = 0;
  int total_tries = 0;
  while(inData != OPC_ready & total_tries++ < 20){
    for(int i = 0; i < 10; i++){
      inData = SPI.transfer(0x01);    // Try reading some bytes here to clear out anything remnant of other SPI activity
      delayMicroseconds(10);
    }
    delay(10);
    digitalWrite(CSpin, LOW);
    while(inData != OPC_ready & tries++ < 20)
    {
      inData = SPI.transfer(command);
      delay(5);
    }
    if(inData != OPC_ready){
      if(inData == OPC_busy){         // waiting 2 seconds because opc is busy
        digitalWrite(CSpin, HIGH);
        delay(2000);
      }
      else{                           // resetting spi because different byte is returned
        digitalWrite(CSpin, HIGH);
        SPI.endTransaction();
        delay(6000);
        SPI.beginTransaction(SPISettings(300000, MSBFIRST, SPI_MODE1));
      }
    }
  }
  delay(10);
  if(inData == OPC_ready)
    return true;
  else
    return false;
}

OPC::OPC(){
  CSpin = 49;
}

bool OPC::begin(){
  SPI.begin();
  pinMode(CSpin, OUTPUT);
  digitalWrite(CSpin, HIGH);
  delay(1000);
  return on();
}

bool OPC::on(){
  bool on = getReady(0x03);
  SPI.transfer(0x03);
  digitalWrite(CSpin, HIGH);
  SPI.endTransaction();
  delay(2000);
  return on;
}

bool OPC::off(){
  bool off = getReady(0x03);
  SPI.transfer(0x00);
  digitalWrite(CSpin, HIGH);
  SPI.endTransaction();
  return off;
}

particleData OPC::getData(){
  particleData data;
  double conv;
  byte vals[64];
  byte command[] = {0x30, 0x01};       // command bytes to request histogram
  
  // SPI transaction
  getReady(command[0]);
  delay(100);
  
  // read all bits available
  for (int i=0; i<64; ++i){
    vals[i] = SPI.transfer(command[1]);
    delayMicroseconds(10);
  }
  
  digitalWrite(CSpin, HIGH);
  
  // sample period [s]
  float sp = fourBytes2float(vals[44], vals[45], vals[46], vals[47]);
  // sample flow rate [ml/s]
  float sfr = fourBytes2float(vals[36], vals[37], vals[38], vals[39]);
  
  // conversion to concentration [particles/ml]
  if (CONVERT){
    conv = sp * sfr;
  }
  else{     // keep as particle-count/sec
    conv = 1;
  }
  for(int i = 0; i < 16; i++){
    data.bin[i] = twoBytes2int(vals[i*2], vals[(i*2)+1]) / conv;
  }

  #if PM_COUNT
    // The below code just gives the raw counts for pm1.0, pm2.5, and pm10.0
    float PM10 = data.bin[0] + data.bin[1] + data.bin[2];
    float PM25 = 0;
    for(int i = 0; i <= 5; i++){
      PM25 += data.bin[i];
    }
    PM25 += data.bin[6] / 2;
    float PM100 = 0;
    for(int i = 0; i <= 11; i++){
      PM100 += data.bin[i];
    }
  #else
    float PM10 = fourBytes2float(vals[50], vals[51], vals[52], vals[53]);
    float PM25 = fourBytes2float(vals[54], vals[55], vals[56], vals[57]);
    float PM100 = fourBytes2float(vals[58], vals[59], vals[60], vals[61]);
  #endif
  
  data.sp = sp;     // This seems to always read 5.20
  data.sfr = sfr;
  data.PM10 = PM10;
  data.PM25 = PM25;
  data.PM100 = PM100;
  
  return data;
}

String OPC::read4sd(particleData data){
  String out_str = "";
  #if PRINT_BINS
    for(int i = 0; i < 16; i++){
      out_str += String(data.bin[i]) + ",";
    }
  #else
    //out_str = ",,,,,,,,,,,,,,,,";
  #endif
  out_str += String(data.sp) + ",";
  out_str += String(data.sfr) + ",";
  out_str += String(data.PM10) + ",";
  out_str += String(data.PM25) + ",";
  out_str += String(data.PM100) + ",";
  
  return out_str;
}
String OPC::read4print(particleData data){
  String out_str = "";
  #if PRINT_BINS
    for(int i = 0; i < 16; i++){
      out_str += "Bin " + String(i) + ": " + String(data.bin[i]) + ",";
    }
  #endif
  out_str += "Sample Period: " + String(data.sp) + ",";
  out_str += "Sample Flow Rate: " + String(data.sfr) + ",";
  out_str += "PM1.0: " + String(data.PM10) + ",";
  out_str += "PM2.5: " + String(data.PM25) + ",";
  out_str += "PM10.0: " + String(data.PM100) + ",";

  return out_str;
}
