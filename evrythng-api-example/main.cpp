/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 *
 * --- DISCLAIMER ---
 *
 * EVRYTHNG provides this source code "as is" and without warranty of any kind,
 * and hereby disclaims all express or implied warranties, including without
 * limitation warranties of merchantability, fitness for a particular purpose,
 * performance, accuracy, reliability, and non-infringement.
 *
 *
 * --- READ ME ---
 *
 * This is a demo application that uses EVRYTHNG's mbed wrapper to read and
 * write value to EVRYTHNG's engine. Please refer to the online documentation
 * available at http://dev.evrythng.com/mbed.
 *
 *
 * Author: Michel Yerly
 *
 */

#include "mbed.h"
#include "EthernetInterface.h"
#include <time.h>
#include "EvrythngApi.h"
#include "util.h"
#include "eventqueue.h"

/*
 * Configuration
 */
 
// Your EVRYTHNG api key.
const char* API_KEY = "";

// Your thng id, the one that represents your mbed device.
const char* THNG_ID = "";

// The property you want values to be pushed to.
const char* THNG_PROP_SET = "prop3";

// The property you want values to be read from.
const char* THNG_PROP_GET = "prop3";



DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

static char displayVal = 0;

/*
 * Displays a value between 0 and 15 as binary on the 4 LEDs.
 */
void display(char x)
{
    led1 = x & (char)8 ? 1 : 0;
    led2 = x & (char)4 ? 1 : 0;
    led3 = x & (char)2 ? 1 : 0;
    led4 = x & (char)1 ? 1 : 0;
    displayVal = x;
}

/*
 * Flashes the 4 LEDs, and restores the state of the last display().
 */
void flash()
{
    char s = displayVal;
    for (int i = 0; i < 2; ++i) {
        display(0x0F);
        wait(0.1);
        display(0x00);
        wait(0.1);
    }
    display(s);
}

/* 
 * Function to get the number of milliseconds elapsed since the system startup.
 * It is imprecise and if called repetitively and quickly, it may always return
 * the same result. It MUST be called at least every 30 minutes.
 * NOTE: I'm not proud of it.
 */
int64_t getMillis()
{
    static Timer tmr;
    static int64_t clk = 0;
    
    tmr.stop();
    clk += tmr.read_ms();
    tmr.reset();
    tmr.start();
    
    char str[21];
    char* end;
    sprinti64(str, clk, &end);
    *end = '\0';
    dbg.printf("%s ", str);
    
    return clk;
}

/*
 * Entry point.
 *
 * This demo application uses an event loop to set and get property values
 * on EVRYTHNG engine at given intervals.
 */
int main()
{
    dbg.printf("Initializing ethernet interface\r\n");
    EthernetInterface eth;
    eth.init(); //Use DHCP
    eth.connect();
    dbg.printf("IP address: %s\r\n", eth.getIPAddress());

    EvrythngApi api(API_KEY);

    // intervals in ms
    const int SET_INTERVAL = 5000;
    const int GET_INTERVAL = 2000;

    int curVal = 0;
    display(curVal);

    int putVal = 0;
    char strPutVal[3];  
    int64_t timestamp = 1345736694000LL; // 2012-08-23 10:44:54am EST

    EventQueue eventQueue;
    eventQueue.put(getMillis()+GET_INTERVAL, EVT_GET_PROP);
    eventQueue.put(getMillis()+SET_INTERVAL, EVT_SET_PROP);

    while (!eventQueue.empty()) {
        EventType event = eventQueue.waitNext(getMillis());

        switch (event) {

            case EVT_SET_PROP:

                eventQueue.put(SET_INTERVAL + getMillis(), EVT_SET_PROP);

                putVal = (putVal + 1) & 0xF;
                sprintf(strPutVal, "%d", putVal);
                timestamp++;
                api.setThngPropertyValue(THNG_ID, THNG_PROP_SET, string(strPutVal), timestamp);

                break;

            case EVT_GET_PROP:

                eventQueue.put(GET_INTERVAL + getMillis(), EVT_GET_PROP);

                string v;
                int r = api.getThngPropertyValue(THNG_ID, THNG_PROP_GET, v);
                if (r == 0) {
                    flash();
                    int newVal = atoi(v.c_str());
                    if (newVal != curVal) {
                        curVal = newVal;
                        display(curVal);
                    }
                }
                break;
        }
    }

    dbg.printf("Disconnecting ethernet interface\r\n");
    eth.disconnect();

    dbg.printf("Done.\r\n");
    while(1) {}
}