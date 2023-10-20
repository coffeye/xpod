// -*- C++ -*-
// Demo2
// Ver:1.00
//
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "s300i2c.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ELT-S300を使用するための型宣言
// S300I2Cは予約。s3()のs3はプログラマが自由につけられる。
// (Wire)は約束。
S300I2C s3(Wire);

void setup() {
  Serial.begin(115200);
  // Wire.begin()は約束。
  Wire.begin();
  // s3.begin(S300I2C_ADDR)は約束。
  // s3は上で宣言した値を使う。
  s3.begin(S300I2C_ADDR);
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT35");
    while (1) delay(1);
  }
  delay(10000); // 10sec wait.
  Serial.println("START S300I2C with SHT31");
}

void loop() {
  unsigned int co2;
  float t,h;
  String outbuf;
  // getCO2ppm()メソッドでCO2を読み込むが、
  // 桁などは有っていないのでセンサ固有の桁合わせを
  // 忘れずに行うように。
  co2 = s3.getCO2ppm();
  t = sht31.readTemperature();
  h = sht31.readHumidity();
  outbuf = "CO2/TEMP/HUM="+String(co2)+"/"+String(t)+"/"+String(h);
  Serial.println(outbuf);
  delay(10000); // 10sec wait
}
