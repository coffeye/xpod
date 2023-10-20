/* -*- C++ -*-
 *
 * Example program for ELT S300 I2C
 * Version: 1.00
 *
 */


#include "s300i2c.h"
#include <Wire.h>

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
  delay(10000); // 10sec wait.
  Serial.println("START S300I2C");
}

void loop() {
  unsigned int co2;
  // getCO2ppm()メソッドでCO2を読み込むが、
  // 桁などは有っていないのでセンサ固有の桁合わせを
  // 忘れずに行うように。
  co2 = s3.getCO2ppm();
  Serial.print("CO2=");
  Serial.println(co2);
  delay(30000); // 30sec wait
}

