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
The code is developed and compiled on Arduino IDE. Make sure you use have the latest version of [Arduino IDE](https://www.arduino.cc/en/software) installed.

<!-- GETTING STARTED -->
## Getting Started
To get a local copy up and running follow these steps
* Git clone the this git repository
   ```sh
   git clone https://github.com/coffeye/xpod.git
   ```
* Navigate to the project directory. Double click on .ino file or open it through Arduino IDE.

### Prerequisites

The project uses various modules to get data from sensors. Install the following library packages within the Arduino IDE. If you don't know how to install a library in Arduino IDE, then follow this [link](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library).

* Adafruit ADS1X15
* Adafruit BME680
* ELT S300
* TinyGPSPlus

<!-- ROADMAP -->
## Status

Currently following features are implemented
- Reading from BME Sensor
- Reading from FIGARO Sensor
- Reading from CO2 and CO sensor
- Serial Port logging

Following features are work in progress
- [ ] SD Card logging
- [ ] Time stamping the logs
- [ ] Time syncing when GPS signal is available
- [ ] Time syncing when GPS signal not available (TBD)