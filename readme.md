<!-- PROJECT LOGO -->
<div align="center">
  <h2 align="center">Hannigan Labs</h2>

  <h3 align="center">
    Xpod for Environmental Sensing 
    <br />
  </h3>
</div>

***
## About The Project

### Built With
The code is developed and compiled on Arduino IDE. Make sure you have the latest version of [Arduino IDE](https://www.arduino.cc/en/software) installed.

<!-- GETTING STARTED -->
## Getting Started
* To get a local copy of the code, run the below command or use the [link](https://github.com/coffeye/xpod/archive/refs/heads/main.zip) to download the zip and extract.
   ```sh
   git clone https://github.com/coffeye/xpod.git
   ```
* Navigate to the project directory. Double click on xpod_node.ino file to open the code in Arduino IDE.

### Prerequisites

The code uses various modules and to interface with them specific Arduino libraries are required. Install the following library packages within the Arduino IDE. If you do not know how to install a library in Arduino IDE, then follow this [link](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library).

* Adafruit ADS1X15
* Adafruit BME680
* ELT S300
* SdFat Adafruit Fork
* TinyGPSPlus

### Modifications
To enable SD card functionality, following modifications are required
* Navigate to the folder where the Arduino libraries are installed. Usually this will be under Documents/Arduino/libraries on Windows OS.
* Inside this directory, open 'SdFat_-_Adafruit_Fork\src\SdFatConfig.h' file and change '#define SPI_DRIVER_SELECT 0' to '#define SPI_DRIVER_SELECT 2'
* Save and close the file.

Now you should be able to compile the code with SD card logging enabled.

<!-- ROADMAP -->
## Status

Features Implemented
- Reading from BME Sensor
- Reading from FIGARO Sensor
- Reading from CO2 and CO sensor
- Serial Port logging

Features in Progress
- [X] SD Card logging
- [ ] Time stamping the logs
- [ ] Time syncing when GPS signal is available
- [ ] Time syncing when GPS signal not available (TBD)