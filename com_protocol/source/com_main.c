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

#ifndef CORE
bool burstRequested = false;
#endif

void setup_CAN_id(void);
void input_msg_callback(uint8 msgId, MyMessage msg);


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
        {
            if(!com_input_buffer_get_burst_request(BURST_BUFFER))
            {
                burstActive = false;
                //printf("burst_terminated\n");
                com_osal_thread_sleep_ms(93);
                currentBurst++;
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
                    com_input_buffer_burst_requested(BURST_BUFFER);
                    com_input_buffer_burst_terminated(currentBurst);
                    com_input_buffer_unblock_buffer(BURST_BUFFER);

                    //printf("burst activated\n");

                    burstActive = true;
                    tempMsg.length = 0;
                    tempMsg.id = com_input_buffer_get_origin(currentBurst);
//                    com_CAN_output_send_msg(CORE_BURST_ACCEPT,tempMsg);
                    break;
                }
                currentBurst=(currentBurst+1)%NB_MODULES;
            }
            if(burstActive)
                continue;
            com_osal_thread_sleep_ms(100);
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
        if(com_output_buffer_half_full()&&(com_output_buffer_get_burst_request()==false))
        {
            tempMsg.length = 0;
            //request burst
//            if(com_CAN_output_send_msg(MOD_BURST_REQ,tempMsg))
//                com_output_buffer_burst_requested();
        }
        com_osal_thread_sleep_ms(100);
    }
}
#endif

void com_main_init(void)
{
    com_osal_init();

    OSAL_CREATE_THREAD(BurstHandler, NULL, OSAL_MEDIUM_PRIO);

    com_CAN_output_init();
	com_CAN_input_init();

	com_CAN_input_set_msg_callback(&input_msg_callback);
    setup_CAN_id();
}

void com_main_end(void)
{
    com_osal_end();
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
    switch(msgId)
    {
#ifdef CORE
    case MOD_BURST_REQ:
        for(uint8 i=0; i<NB_MODULES; i++)
            if((com_input_buffer_get_origin(i)&0x3FF)==(msg.id&0x3FF))
            {
                com_input_buffer_burst_requested(i);
                break;
            }
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
        NB_MSG_P_S = msg.data8[0];
#endif
    default: break;
    }
}
