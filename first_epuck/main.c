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

#include <sensors/proximity.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

uint8 value = 0;

OSAL_DEFINE_THREAD(empty_buffer, 256, arg){
    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void) arg;
    uint8 inMailbox;
    uint16 proxVal[8];
    ComMessage inMessage;

    inMailbox = com_open_mailbox(0x441);

    while(true)
    {
        while(com_poll_mailbox() & (0x1<<inMailbox))
        {
            if(com_get_message_length(inMailbox)!=16)
            {
                inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                continue;
            }
            inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
            if(inMessage.length == BUFFER_ERROR)
                continue;
            for(uint8 i=0; i<8; i+=2)
            {
                if(proxVal[i]>0xFF)
                    set_led(i/2,0);
                else
                    set_led(i/2,1);
                if(proxVal[i+1]>0xFF)
                    set_rgb_led(i+1,0,0,0);
                else
                    set_rgb_led(i+1,0xFF,0,0);
            }
        }

        com_osal_thread_sleep_ms(10);
    }
}

int main(void)
{
	//msg_t new;
    ComMessage testing;
    uint8 length;

    halInit();
    chSysInit();
    mpu_init();

    proximity_start();
    com_init();

    //OSAL_CREATE_THREAD(empty_buffer, NULL, OSAL_MEDIUM_PRIO);

    /** Inits the Inter Process Communication bus. */
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	uint8 data[10] = {0,1,2,3,4,5,6,7,8,9 };
	testing.data = data;
	testing.timestamp = 0x1234;

	for(uint8 i = 0; i<10; i++)
	{
	    testing.contentId = i;
	    testing.length = i;
		value = com_output_buffer_write_message(testing);
	}
/*
	value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_write_data(testing);
	value = com_output_buffer_write_header(testing);
	value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_write_header(testing);
	value = com_output_buffer_write_message(testing);
	value = com_output_buffer_write_data(testing);

	value = com_output_buffer_msg_free(testing);

	value = com_output_buffer_get_next_length();

	for(uint8 i = 0; i<10; i++)
		data[i] = 0;

	//com_output_buffer_burst_content_msg(data);
	for(uint8 i = 0; i<10; i++)
	    value = com_output_buffer_read_message(&testing);

	value = com_output_buffer_msg_free(testing);
	value = com_output_buffer_read_data(&testing);
    length = com_output_buffer_read_header(&testing);
    value = com_output_buffer_read_header(&testing);
    value = com_output_buffer_read_message(&testing);
    for(uint8 i = 0; i<10; i++)
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
    value = com_output_buffer_read_data(&testing);
*/
/*
	testing.contentId = 0x15;
    testing.timestamp = 0x1234;
    testing.length = 9;
    testing.data = data;

    value = com_output_buffer_write_message(testing);
    value = com_output_buffer_undo_write();
    value = com_output_buffer_write_message(testing);
    value = com_output_buffer_write_header(testing);
    value = com_output_buffer_undo_write();
    value = com_output_buffer_write_header(testing);
    value = com_output_buffer_write_data(testing);
    value = com_output_buffer_undo_write();
    value = com_output_buffer_write_data(testing);

    value = com_output_buffer_read_message(&testing);
    value = com_output_buffer_undo_read();
    value = com_output_buffer_read_message(&testing);
    value = com_output_buffer_read_header(&testing);
    value = com_output_buffer_undo_read();
    value = com_output_buffer_read_header(&testing);
    value = com_output_buffer_read_data(&testing);
    value = com_output_buffer_undo_read();
    value = com_output_buffer_read_data(&testing);

    value = com_output_buffer_write_message(testing);
    value = com_output_buffer_read_message(&testing);
    value = com_output_buffer_undo_write();

    value = com_output_buffer_write_message(testing);
    value = com_output_buffer_read_message(&testing);
    value = com_output_buffer_write_message(testing);
    value = com_output_buffer_undo_read();
*/
    uint8 test_msg = 0xBE;
    testing.contentId = 0x15;
    testing.timestamp = 0x1234;
    testing.length = 1;
    testing.data = data;
    value = com_send_data(testing);

//    testing.length = 16;
//    uint16 prox_val[8];
//    testing.data = (uint8*) prox_val;

    while (1) {
//        for(uint8 i=0; i<8; i++)
//            prox_val[i] = get_prox(i);

        value = com_send_data(testing);
        palTogglePad(GPIOD, GPIOD_LED_FRONT);
        chThdSleepMilliseconds(1);
    }
    return value;
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
