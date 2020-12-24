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

uint8_t value = 0;

int main(void)
{
	//msg_t new;
    ComMessage testing;
    uint8_t length;

    halInit();
    chSysInit();
    mpu_init();

    com_init();

    /** Inits the Inter Process Communication bus. */
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	uint8_t data[10] = {0,1,2,3,4,5,6,7,8,9 };
	testing.data = data;
	testing.timestamp = 0x1234;

	for(uint8_t i = 0; i<10; i++)
	{
	    testing.contentId = i;
	    testing.length = i;
		value = com_output_buffer_write_message(testing);
	}
	/*value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_write_data(testing);
	value = com_output_buffer_write_header(testing);
	value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_write_header(testing);
	value = com_output_buffer_write_message(testing);
	value = com_output_buffer_write_data(testing);

	value = com_output_buffer_msg_free(testing);

	value = com_output_buffer_get_next_length();

	for(uint8_t i = 0; i<10; i++)
		data[i] = 0;

	//com_output_buffer_burst_content_msg(data);
	for(uint8_t i = 0; i<10; i++)
	    value = com_output_buffer_read_message(&testing);

	value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_read_data(&testing);
    length = com_output_buffer_read_header(&testing);
    value = com_output_buffer_read_header(&testing);
    value = com_output_buffer_read_message(&testing);
    for(uint8_t i = 0; i<10; i++)
        data[i] = 0;
    testing.length = length;
    value = com_output_buffer_read_data(&testing);

    testing.contentId = 0x15;
    testing.timestamp = 0x1234;
    testing.length = 9;
    testing.data = data;
    value = com_output_buffer_msg_available();
    value = com_output_buffer_write_header(testing);
    value = com_output_buffer_msg_available();
    value = com_output_buffer_write_data(testing);

    value = com_output_buffer_msg_available();
    value = com_output_buffer_read_header(&testing);
    testing.length = 9;
    value = com_output_buffer_msg_available();
    value = com_output_buffer_read_data(&testing);*/

    uint8_t test_msg = 0xBE;
    testing.contentId = 0x15;
    testing.timestamp = 0x1234;
    testing.length = 1;
    testing.data = &test_msg;
    value = com_send_data(testing);

    while (1) {

        palTogglePad(GPIOD, GPIOD_LED_FRONT);
        chThdSleepMilliseconds(1000);
    }
    return value;
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
