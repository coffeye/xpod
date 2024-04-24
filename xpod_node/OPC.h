/*
OPC.h - library for operating optical particle counter OPC-R2 from Alphasense using an Arduino Mega.
 
 Written by Joseph Habeck (habec021@umn.edu) on 6/24/18. (https://github.com/JHabeck/Alphasense-OPC-N2/tree/master)
 Edited using Marcel Oliveira's code from github (https://github.com/shyney7/OPC-R2_ESP32/tree/main) 
 Put together by Aidan Mobley
*/
// White = MISO, purple = CLK, blue = MOSI, green = D49
// Do not plug green(CS) into the CS pin on rev4

#ifndef OPC_h
#define OPC_h

// include Arduino SPI library
#include <SPI.h>

#define PRINT_BINS  0 // Prints the amount of particles in each of the 16 bins
#define PM_COUNT    0 // Returns the PM measurements in particle count rather than ug/m3
#define CONVERT     0 // Returns the bin measurements in particles/ml rather than particle count


const byte OPC_ready = 0xF3;
const byte OPC_busy = 0x31;

// particle data structure
struct particleData{
  int bin[16];    // bin 0-2: pm 1.0, bin 0-5/6: pm 2.5 bin 0-11: pm 10.0     bin 5: 1.6-2.1, bin 6: 2.1-3.0
  float sp;
  float sfr;
  float PM10;
  float PM25;
  float PM100;
};

// define class
class OPC{
  public:
    OPC();
    bool begin();
    bool on();
    bool off();
    particleData getData();
    String read4sd(particleData data);
    String read4print(particleData data);
  
  private:
    uint16_t twoBytes2int(byte LSB, byte MSB);
    float fourBytes2float(byte val0, byte val1, byte val2, byte val3);
    bool getReady(const byte command);
    int CSpin;
};


#endif /* OPC_h */
