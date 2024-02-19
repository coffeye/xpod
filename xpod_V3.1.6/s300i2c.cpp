#include "s300i2c.h"
#include "Arduino.h"

S300I2C::S300I2C(TwoWire &w) {
  wire = &w;
  co2i = 0;
}

boolean S300I2C::begin(uint8_t i2caddr = S300I2C_ADDR) {
  _i2caddr = i2caddr;
  return true;
}

void S300I2C::writeCommand(uint8_t cmd) {
  wire->beginTransmission(_i2caddr);
  wire->write(cmd);
  wire->endTransmission();
}

unsigned int S300I2C::getCO2ppm(void) {
  writeCommand('R');
  wire->requestFrom((int)_i2caddr,(int)7);
  for (int i=0; wire->available(); i++) {
    _tmpBuf[i] = wire->read();
    delay(10);
  }
  if (_tmpBuf[0] != 0x08 ||
      _tmpBuf[3] == 0xff ||
      _tmpBuf[4] == 0xff ||
      _tmpBuf[5] == 0xff ||
      _tmpBuf[6] == 0xff) {
    return 0;
  }
  return (_tmpBuf[1] << 8) | _tmpBuf[2];
}

void S300I2C::sleep(void) {
  writeCommand('S');
  delay(4000);
}

void S300I2C::wakeup(void) {
  writeCommand('W');
  delay(6000);
}
  
void S300I2C::clear_recalib(void) {
  writeCommand('C');
  delay(6000);
}

void S300I2C::start_mcdl(void) {
  writeCommand('M');
  delay(2000);
}

void S300I2C::end_mcdl(void) {
  writeCommand('E');
  delay(2000);
}

void S300I2C::start_acdl(void) {
  writeCommand('A');
  delay(2000);
}

void S300I2C::end_acdl(void) {
  writeCommand('E');
  delay(2000);
}
