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

#include "com_api.h"
#include "com_output_buffer.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void)
{
	msg_t new;

    halInit();
    chSysInit();
    mpu_init();

    com_init();

    /** Inits the Inter Process Communication bus. */
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	uint8_t data[10] = {0,1,2,3,4,5,6,7,8,9 };

	for(uint8_t i = 0; i<10; i++)
	{
		com_output_buffer_write_data(i,0x1234,i, &data);
		//data += i;
	}

	uint8_t msg_id;
	uint16_t timestamp;
	uint8_t length;

	length = com_output_buffer_get_next_length();

	for(uint8_t i = 0; i<10; i++)
		data[i] = 0;

	for(uint8_t i = 0; i<10; i++)
	{

		length = com_output_buffer_read_data(&msg_id, &timestamp, &data);
		com_output_buffer_write_data(msg_id,timestamp, length, &data);
	}


    while (1) {

        palTogglePad(GPIOD, GPIOD_LED_FRONT);
        chThdSleepMilliseconds(1000);
    }
    return 0;
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
