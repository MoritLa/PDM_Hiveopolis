/*
 * com_main.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */
#include "com_main.h"
#include "com_CAN_output.h"
#include "com_CAN_input.h"
#include "com_output_buffer.h"
#include "com_input_buffer.h"
#include "com_osal.h"

//#ifndef CORE
//bool burstRequested = false;
//#endif

// Reads address pins to define Module Id, for core default is 0
void setup_CAN_id(void);

// Callback function for protocol messages
void input_msg_callback(uint8 msgId, MyMessage msg);

#ifdef CORE
enum states {NO_BURST, BURST_ACTIVE};
static uint8 state = NO_BURST;
#endif

#ifdef CORE
OSAL_DEFINE_THREAD(BurstHandler, 256, arg) {
    (void) arg;
    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;

    uint8 currentBurst=0;
    uint8 burstActive = false;
    MyMessage tempMsg;

    while(true)
    {
        if(burstActive)
        //if(state == BURST_ACTIVE)
        {
            if(!com_input_buffer_get_burst_request(BURST_BUFFER))
            {
                burstActive = false;
                // state = NO_BURST;
                //currentBurst=(currentBurst+1)%NB_MODULES;
            }
            else
                com_osal_thread_sleep_ms(1);
        }
        else
        {
            for(uint8 i=0; i<NB_MODULES;i++)
            {
                if(com_input_buffer_get_burst_request(currentBurst))
                {
                    if(!com_input_buffer_set_origin(BURST_BUFFER,com_input_buffer_get_origin(currentBurst)))
                        break; //don't start burst if buffer is not ready

                    tempMsg.length = 0;
                    tempMsg.id = com_input_buffer_get_origin(currentBurst);

                    if(!com_CAN_output_send_msg(CORE_BURST_ACCEPT,tempMsg))
                        break;

                    burstActive = true;
                    //state = BURST_ACTIVE;
                    com_input_buffer_burst_requested(BURST_BUFFER);
                    com_input_buffer_burst_terminated(currentBurst);
                    com_input_buffer_unblock_buffer(BURST_BUFFER);
                    break;
                }
                //currentBurst=(currentBurst+1)%NB_MODULES;
            }
            if(burstActive)
            //if(state == BURST_ACTIVE)
                com_osal_thread_sleep_ms(BURST_PERIOD);
            else
                com_osal_thread_sleep_ms(10);
        }
    }
}
#else

OSAL_DEFINE_THREAD(BurstHandler, 256, arg) {

    (void) arg;
    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;

    MyMessage tempMsg;

    while(true)
    {
        if(com_output_buffer_half_full() &&
            (com_output_buffer_get_burst_request()==false))
        {
            tempMsg.length = 0;
            //request burst
            if(com_CAN_output_send_msg(MOD_BURST_REQ,tempMsg))
                com_output_buffer_burst_requested();
        }
        com_osal_thread_sleep_ms(10);
    }
}
#endif

void com_main_init(void)
{
    MyMessage tempMsg;

    com_osal_init();

    com_CAN_output_init();
	com_CAN_input_init();

	com_CAN_input_set_msg_callback(&input_msg_callback);
    setup_CAN_id();

    tempMsg.length = 0;


#ifdef CORE
    state = NO_BURST;
    while(!com_CAN_output_send_msg(CORE_REG,tempMsg))
        com_osal_thread_sleep_ms(1);
    com_output_buffer_unblock_buffer();
#else
    while(!com_CAN_output_send_msg(MOD_REG,tempMsg))
        com_osal_thread_sleep_ms(1);
#endif

    OSAL_CREATE_THREAD(BurstHandler, NULL, OSAL_MEDIUM_PRIO);
}

void com_main_end(void)
{
    com_osal_end();
}

uint8 com_main_register_module(uint16 origin)
{
    uint8 mailbox = NO_MAILBOX;

     for(uint8 i=0; i<NB_MODULES; i++)
         if(com_input_buffer_get_origin(i)==(origin&0x3FF) &&
             com_input_buffer_get_origin(i) != ID_NOT_SET)
             return i;

     //select available mailbox
     for(uint8 i=0; i<NB_MODULES; i++)
         if(com_input_buffer_is_blocked(i)&&
             com_input_buffer_empty(i))
         {
             mailbox = i;
             break;
         }
     // if there is no Mailbox available or allocation fails
     if(mailbox != NO_MAILBOX &&
         com_input_buffer_set_origin(mailbox, origin))
     {
         com_input_buffer_unblock_buffer(mailbox);
         return mailbox;
     }
     else
         return NO_MAILBOX;
     return NO_MAILBOX;
}

void setup_CAN_id(void)
{
#ifdef CORE
    com_CAN_output_set_module_id(MODULE_ID);
    com_CAN_input_set_module_id(MODULE_ID);
#else
    uint8 id = 0;
    com_osal_setup_GPIO(ADRESS_PIN0, IN);
    com_osal_setup_GPIO(ADRESS_PIN1, IN);
    com_osal_setup_GPIO(ADRESS_PIN2, IN);
    com_osal_setup_GPIO(ADRESS_PIN3, IN);

    id = com_osal_get_GPIO(ADRESS_PIN0) |
        (com_osal_get_GPIO(ADRESS_PIN1)<<1) |
        (com_osal_get_GPIO(ADRESS_PIN2)<<2) |
        (com_osal_get_GPIO(ADRESS_PIN3)<<3);

    com_CAN_output_set_module_id(id);
    com_CAN_input_set_module_id(id);
#endif
}


void input_msg_callback(uint8 msgId, MyMessage msg)
{
    MyMessage tempMsg;
#ifdef CORE
    uint8 i = 0;
#endif

    switch(msgId)
    {
#ifdef CORE
    case MOD_BURST_REQ:
        for(i=0; i<NB_MODULES; i++)
            if((com_input_buffer_get_origin(i)&0x3FF)==(msg.id&0x3FF))
            {
                com_input_buffer_burst_requested(i);
                break;
            }
        break;
    case MOD_REG:
        //Module Handshake
        tempMsg.length = 0;
        tempMsg.id = msg.id;

        //clear burst flags
        for(i=0; i<=NB_MODULES; i++)
            if((com_input_buffer_get_origin(i)&0x3FF)==(msg.id&0x3FF))
            {
                com_input_buffer_clear_buffer(i);
                if((com_input_buffer_get_origin(BURST_BUFFER)&0x3FF)==(msg.id&0x3FF))
                    com_input_buffer_clear_buffer(BURST_BUFFER);
                com_input_buffer_set_origin(i,msg.id);
                com_input_buffer_unblock_buffer(i);
                break;
            }
        if(i == NB_MODULES)
        {
            //register module
            com_main_register_module(msg.id);
        }
        //reset input buffer
        //if current burst from this ID, reset burst buffer
        // stop active burst
        while(!com_CAN_output_send_msg(CORE_HS,tempMsg))
            com_osal_thread_sleep_ms(1);

        break;
    case MOD_HS:
        break;
#else
    case CORE_BURST_ACCEPT:
        if(msg.id == com_CAN_input_get_CAN_id())
            com_CAN_output_burst_accepted();
        else
        {
            if(!com_input_buffer_set_origin(BURST_BUFFER, msg.id))
                break; //don't start burst if buffer is not ready
            com_input_buffer_unblock_buffer(BURST_BUFFER);
        }
        break;
    case CORE_CHOKE:
        com_CAN_output_set_fps(msg.data8[0]);
        //NB_MSG_P_S = msg.data8[0];
        break;
    case CORE_REG:
        tempMsg.length = 0;
        while(!com_CAN_output_send_msg(MOD_REG,tempMsg))
            com_osal_thread_sleep_ms(1);
        if(!com_CAN_output_get_burst_accepted())
            com_output_buffer_burst_terminated();
        break;
    case CORE_HS:
        tempMsg.length = 4;
        tempMsg.data8[0] = (uint8) SERIAL_NUMBER;
        tempMsg.data8[1] = (uint8) (SERIAL_NUMBER>>8);
        tempMsg.data8[2] = (uint8) HEARTBEAT_PERIOD;
        tempMsg.data8[3] = (uint8) (HEARTBEAT_PERIOD>>8);
        while(!com_CAN_output_send_msg(MOD_HS, tempMsg))
            com_osal_thread_sleep_ms(1);
        //Start transmitting
        com_output_buffer_unblock_buffer();
        break;
#endif
    default: break;
    }
}
