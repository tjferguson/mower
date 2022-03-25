#include <Arduino.h>
#include <SoftwareSerial.h>
#include "main.h"
#include "sbus.h"

//Define a log statement to be using our special serial interface
#define LOG(x) (mySerial.println(x))


//If we passed in DEBUG to the compiler, we want to include the debug output statements to serial.
#ifdef DEBUG
#define TRACE(x) (mySerial.print(x))
#else
#define TRACE(x)
#endif

#ifdef INFO
#define TRACE2(x) (mySerial.print(x))
#else
#define TRACE2(x)
#endif

SoftwareSerial mySerial(2, 3); // RX, TX

#define REMOTE_CONNECTION_CHANNEL 8
#define SWITCH_ON 1720
#define SWITCH_MID 990
#define SWITCH_OFF 190
#define SWITCH_JITTER 20


//Receive channel for the sbus data
bfs::SbusRx sbus_rx(&Serial);

//Transmit channel for outbound sbus data
bfs::SbusTx sbus_tx(&Serial);

//Buffer/Array to read in int values from the sbus channels
std::array<int16_t, bfs::SbusRx::NUM_CH()> sbus_data;


/**
 * Setup the board for communication via the remote transmitter
 *
 * This is handled via 2 major configurations.
 * 1) We configure the serial output (and technically input) to be via a UART connection using SoftwareSerial at 38400 (on pin 2,3 as above)
 *
 * 2) We have the TX SBUS connected to the Serial RX (Pin 0) on the board; this is set to a baud rate of 100000 (validated via oscilliscope as 10us long pulses)
 */
void setup() {
    // set the data rate for the SoftwareSerial port
    mySerial.begin(38400);
    LOG("UART Init completed");

    //Configure the real Serial port to use 10 microsecond pulse width; or 100000 baud.
    Serial.begin(100000);

    LOG("Serial begin(100000)");
    while (!Serial) {}

    LOG("Serial Ready, starting SBUS RX");

    /* Begin the SBUS communication */
    sbus_rx.Begin();
    LOG("SBUS RX Ready");
}

/**
 * We care about 9 channels for our R81 they are as follows (mapped from my remote):
 * These were assigned in the DEFINES above as:
 * SWITCH_ON ==> 1720
 * SWITCH_MID ==> 990
 * SWITCH_OFF ==> 190
 * SWITCH_JITTER ==> 20
 *
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

//Our general rule here is that 0 is the 'middle' condition (e.g. no throttle, no left/right motion)
signed int steering = 0;
signed int throttle = 0;

void loop() {
    if (sbus_rx.Read()) {
        sbus_data = sbus_rx.ch();
        REMOTE_CONNECTED = validateRemoteConnection(sbus_data[REMOTE_CONNECTION_CHANNEL], REMOTE_CONNECTED);
        if(REMOTE_CONNECTED) {
            throttle = sbusValueToPercent(sbus_data[2]);
            steering = sbusValueToPercent(sbus_data[0]);

            TRACE2("Throttle/Steering Vector:");
            TRACE2(throttle); TRACE2("/"); TRACE2(steering);
            TRACE2("\n");
        } else if(!REMOTE_CONNECTED && (steering != 0 || throttle != 0) ){
            //Disable throttle because we shouldn't be here; this effectively shuts down the motion of the mower
            //If we add a relay switch for the spark plug; we would activate that here as well.
            throttle = 0;
            steering = 0;
            LOG("Disabled power to motors because the remote is disconnected");
        }

#ifdef DEBUG
        for (int8_t i = 0; i <= 8; i++) {
            TRACE("Channel");
            TRACE(i);
            TRACE(":");
            TRACE(sbus_data[i]);
            TRACE("\t");
        }
        /* Display lost frames and failsafe data */
        TRACE(sbus_rx.lost_frame());
        TRACE("\t");
        TRACE(sbus_rx.failsafe());

        TRACE("\n");
#endif
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
    if(remoteConnected && signalStrength > (SWITCH_MID+SWITCH_JITTER) ) return true;

    if(!remoteConnected && signalStrength > (SWITCH_MID+SWITCH_JITTER) ) {
        LOG("Remote is reconnected");
        return true;
    } else if(remoteConnected && signalStrength <= (SWITCH_MID+SWITCH_JITTER)) {
        LOG("Remote disconnected");
        return false;
    }

    //no output; this will be hit in cases where the remote is NOT connected, and the signal str is too low to flag as connected.
    return false;
}


/**
 * Calculate the sbus value on a range of -100, 0, 100; where -100 is 100% backwards throttle; 0 is stopped; and 100 is 100% forward motion.
 *
 * Values from receiver appear to be on scale of: 190 == off; 992 == mid; 1720 == 100%
 *
 * @param sbusValue
 * @return
 */
int sbusValueToPercent(int sbusValue) {
    //Give ourselves a few degrees on the stick of 'zero' space, since we don't want the mower to jump when we start it up.
    if(sbusValue > (SWITCH_MID-SWITCH_JITTER) && sbusValue < (SWITCH_MID+SWITCH_JITTER)) {
        return 0;
    }
    if(sbusValue < (SWITCH_OFF+SWITCH_JITTER)) return -100;
    if(sbusValue > (SWITCH_ON-SWITCH_JITTER)) return 100;

    if(sbusValue <= SWITCH_MID) {
        //Calculate as a whole % of the delta of the range 990-190  (800 values)
        int ret = (int)( (100 - ((sbusValue - SWITCH_OFF)/8.2f))*-1);
        if(ret > 0) return 0;
        if(ret < -90) return -100;
        return ret;
    }
    if(sbusValue >= SWITCH_MID) {
        int ret = (int)((sbusValue - SWITCH_MID)/7.2f);
        if(ret > 100) return 100;
        if(ret < 0) return 0;

        return ret;
    }

    return 0;
}