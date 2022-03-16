Use of SoftwareSerial is absolutely required, this should be ran through something like a CP2102 UART dongle
so that data can be collected from Serial without conflicting with the SBUS serial data stream.


To configure and make work on CLION, install platformio, install platformio plugins.

Setup CLI Tools (tried using export of path, but that failed for some reason inside CLOIN worked fine in CLI directly):
* sudo ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio
* sudo ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio
* sudo ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb
