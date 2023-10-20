/*
 * ELT S300 I2C library
 * s300.h
 */

#ifndef ELT_S300_I2C_HOLLY
#define ELT_S300_I2C_HOLLY

#include "Arduino.h"
#include "Wire.h"

#define S300I2C_ADDR      0x31

class S300I2C {
 public:
  S300I2C(TwoWire &w);
  boolean begin(uint8_t i2caddr);
  void sleep(void);              // Sleep command
  void wakeup(void);             // Wake up command
  void clear_recalib(void);      // Clear Recalibtation Factor Command
  void start_mcdl(void);         // Start Manual Calibration
  void end_mcdl(void);           // End Manual Calibration
  void start_acdl(void);         // Start Auto-Calibration
  void end_acdl(void);           // End Auto Calibration
  unsigned int getCO2ppm(void);  // get CO2 value

 private:
  TwoWire *wire;
  unsigned int co2i;
  uint8_t _i2caddr;
  uint8_t _tmpBuf[7];

  /**
   * Internal function to perform and I2C write.
   *
   * @param cmd   The 8-bit command ID to send.
   */
  void     writeCommand(uint8_t cmd);


};
#endif
