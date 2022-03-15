# General Information


Use of SoftwareSerial is absolutely required, this should be ran through something like a CP2102 UART dongle
so that data can be collected from Serial without conflicting with the SBUS serial data stream.


To configure and make work on CLION, install platformio, install platformio plugins.

Setup CLI Tools (tried using export of path, but that failed for some reason inside CLOIN worked fine in CLI directly):
* sudo ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio
* sudo ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio
* sudo ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb




## Parts List

### Pre-Built
* Arduino Uno
* 2x MegaMoto Motor Control (H-Bridge)
* RS81 FrSky receiver
* Foxeer 1200TVL Razer Nano FPV Camera
* Mateksys VTX-HV video transmitter

### Single Components

#### Inverter
* 2x 10k Resisters
* 1x 2N3904 Transitor
* PCB

#### Power Rail
* 4x 12V 7.5AH F2 SLA Batteries 

#### Drive Configuration
* 2x 24V 350W MY1016 Electric Motor with 11 Tooth #25 Chain Sprocket
* 
