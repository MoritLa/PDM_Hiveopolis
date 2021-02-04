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


#define MSG_LENGTH      16
#define MSG_PERIOD      1000//[ms]

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static uint8 value = 0;

static uint8 dataLength = 50;

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

OSAL_DEFINE_THREAD(read_core, 512, arg){
    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void) arg;
    uint8 inMailbox;
    ComMessage inMessage;

    uint8 data[253];

    inMailbox = com_open_mailbox(0x400);

    while(true)
    {
        while(com_poll_mailbox() & (0x1<<inMailbox))
        {
            inMessage = com_read_mailbox(inMailbox, data);

            if(inMessage.length==1 || inMessage.contentId == 0x10)
            {
                if(inMessage.data[0] & (1<<0))
                    palClearPad(GPIOD, GPIOD_LED1);
                else
                    palSetPad(GPIOD, GPIOD_LED1);

                if(inMessage.data[0] & (1<<1))
                    palClearPad(GPIOD, GPIOD_LED3);
                else
                    palSetPad(GPIOD, GPIOD_LED3);

                if(inMessage.data[0] & (1<<2))
                    palClearPad(GPIOD, GPIOD_LED5);
                else
                    palSetPad(GPIOD, GPIOD_LED5);

                if(inMessage.data[0] & (1<<3))
                    palClearPad(GPIOD, GPIOD_LED7);
                else
                    palSetPad(GPIOD, GPIOD_LED7);
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

    // Inits the Inter Process Communication bus.
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    com_init();

    //OSAL_CREATE_THREAD(empty_buffer, NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_core, NULL, OSAL_MEDIUM_PRIO);

	uint8 data[MSG_LENGTH];

    testing.contentId = 0x15;
    testing.length = MSG_LENGTH;
    testing.data = data;


    while (1) {
        for(uint8_t i = 0; i<testing.length; i++)
            data[i] = (uint8) rand()&0xFF;
        testing.timestamp = (uint16) (com_osal_get_systime_ms()&0xFFFF);

        value = com_send_data(testing);

        //palTogglePad(GPIOD, GPIOD_LED_FRONT);

        com_osal_thread_sleep_ms(MSG_PERIOD);
    }
    return value;
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
