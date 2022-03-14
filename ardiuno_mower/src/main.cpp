#include <Arduino.h>
#include <SoftwareSerial.h>
#include "main.h"
#include "sbus.h"

SoftwareSerial mySerial(2, 3); // RX, TX

#define REMOTE_CONNECTION_CHANNEL 8

/* SbusRx object on Serial1 */
bfs::SbusRx sbus_rx(&Serial);
/* SbusTx object on Serial1 */
//bfs::SbusTx sbus_tx(PIN1);

/* Array for storing SBUS data */
std::array<int16_t, bfs::SbusRx::NUM_CH()> sbus_data;

// the setup function runs once when you press reset or power the board
void setup() {
    // set the data rate for the SoftwareSerial port
    mySerial.begin(38400);
    mySerial.println("UART Init completed");
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    mySerial.println("LED Output mode set");
    //100000
    Serial.begin(100000);
    mySerial.println("Serial begin(100000)");
    while (!Serial) {}

    mySerial.println("Serial Ready, starting SBUS RX");

    /* Begin the SBUS communication */
    sbus_rx.Begin();
    mySerial.println("SBUS RX Ready");
}

/**
 * We care about 9 channels for our R81 they are as follows (mapped from remote):
 * 0 --> Ail (right stick Left/Right)
 * 1 --> Elevation (right stick up/down)
 * 2 --> Throttle (left stick up/down)
 * 3 --> Rudder  (left stick left/right)
 * 4 --> Switch (SA)  190 == off; 992 == mid; 1720 == 100%
 * 5 --> Switch (SB)  190 == off; 992 == mid; 1720 == 100%
 * 6 --> Switch (SC)  190 == off; 992 == mid; 1720 == 100%
 * 7 --> Switch (SD)  190 == off; 992 == mid; 1720 == 100%
 * 8 --> Signal Strength --> < 1000 disconnected; >= 1001 connected at various levels
 */
bool REMOTE_CONNECTED = false;
float steering = 50.0;

void loop() {
    if (sbus_rx.Read()) {
        sbus_data = sbus_rx.ch();
        REMOTE_CONNECTED = validateRemoteConnection(sbus_data[REMOTE_CONNECTION_CHANNEL], REMOTE_CONNECTED);

        for (int8_t i = 0; i <= 8; i++) {
            mySerial.print("Channel");
            mySerial.print(i);
            mySerial.print(":");
            mySerial.print(sbus_data[i]);
            mySerial.print("\t");
        }
        mySerial.print("\n");
    }
}


/**
 * Check the incoming signal strength; channel index 8 by default.
 *
 * @param signalStrength
 * @param remoteConnected
 * @return
 */
boolean validateRemoteConnection(int signalStrength, boolean remoteConnected) {
    //mySerial.println(signalStrength);
    //Connected and still connected; nothing to see here
    if(remoteConnected && signalStrength > 1000) return true;

    if(!remoteConnected && signalStrength > 1000) {
        mySerial.println("Remote is reconnected");
        return true;
    } else if(remoteConnected && signalStrength <= 1000) {
        mySerial.println("Remote disconnected");
        return false;
    }

    //no output; this will be hit in cases where the remote is NOT connected, and the signal str is too low to flag as connected.
    return false;
}