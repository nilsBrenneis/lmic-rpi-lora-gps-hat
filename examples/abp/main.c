/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lmic.h"
#include "debug.h"

// sensor functions
extern void initsensor(void);
extern u2_t readsensor(void);

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////

static const u1_t NWKSKEY[16] = { 0x08, 0x1E, 0x80, 0xCF, 0xB4, 0x3D, 0x96, 0x36, 0xF3, 0x2B, 0x80, 0xF2, 0xD3, 0xEC, 0x31, 0xC6 };

static const u1_t APPSKEY[16] = { 0x07, 0xF1, 0x69, 0xD6, 0xE5, 0x32, 0xF7, 0x8A, 0xC3, 0xA9, 0x91, 0x00, 0x76, 0x3C, 0x54, 0xE3 };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x260130C9 ; // <-- Change this address for every node!

static void do_send(osjob_t*);
static char mydata[32] = "Hello, world!";
static osjob_t sendjob;

//////////////////////////////////////////////////
// APPLICATION CALLBACKS
// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
//////////////////////////////////////////////////
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// initial job
static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();

    // start joining
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}


// application entry point
int main () {
    osjob_t initjob;

    // initialize runtime env
    os_init();
    // initialize debug library
    debug_init();
    // setup initial job
    os_setCallback(&initjob, initfunc);
    // execute scheduled jobs and events
    os_runloop();
    // (not reached)
    return 0;
}


//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////


void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        debug_str("OP_TXRXPEND, not sending\r\n");
    } else {
        // Prepare upstream data transmission at the next possible time.
        u2_t val = readsensor();
        debug_val("val = ", val);
        // prepare and schedule data for transmission
        LMIC.frame[0] = val >> 8;
        LMIC.frame[1] = val;
        LMIC_setTxData2(1, LMIC.frame, 2, 0); // (port 1, 2 bytes, unconfirmed)
        debug_str("Packet queued\r\n");
    }

    os_setTimedCallback(j, os_getTime()+sec2osticks(10), do_send);
}

//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {

      case EV_TXCOMPLETE:
          debug_str("EV_TXCOMPLETE (includes waiting for RX windows)\r\n");
          if (LMIC.txrxFlags & TXRX_ACK)
            debug_str("Received ack\r\n");
          if (LMIC.dataLen) {
            debug_str("Received ");
            debug_val("len=", LMIC.dataLen);
            debug_str(" bytes of payload\r\n");
          }
          break;
      case EV_LOST_TSYNC:
          debug_str("EV_LOST_TSYNC\r\n");
          break;
      case EV_RESET:
          debug_str("EV_RESET\r\n");
          break;
      case EV_RXCOMPLETE:
          // data received in ping slot
          debug_str("EV_RXCOMPLETE\r\n");
          break;
      default: 
          debug_str("EV...\r\n");
	  break;
    }
}

