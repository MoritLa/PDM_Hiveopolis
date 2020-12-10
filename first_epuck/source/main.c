/*
 * main.c
 * Initialisation of the threads
 *
 *  Created on: 30. oct. 2018
 *      Author: Moritz Laim
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hal.h"
#include "ch.h"
#include "memory_protection.h"

#include "usbcfg.h"

#include "leds.h"

#include "main.h"

//#include "com_api.h"


messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void)
{
	msg_t new;

    halInit();
    chSysInit();
    mpu_init();

    /** Inits the Inter Process Communication bus. */
	messagebus_init(&bus, &bus_lock, &bus_condvar);

    while (1) {

        palTogglePad(GPIOD, GPIOD_LED_FRONT);
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
