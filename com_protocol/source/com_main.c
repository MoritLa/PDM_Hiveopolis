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

void setup_CAN_id(void);
void input_msg_callback(uint8 msgId, MyMessage msg);

OSAL_DEFINE_THREAD(FSM, 256, arg) {

    (void) arg;
    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;


    while(true)
    {
        if(com_output_buffer_half_full())
        {
            //request burst
        }
        com_osal_thread_sleep_ms(100);
    }
}


bool com_main_init(void)
{
    OSAL_CREATE_THREAD(FSM, NULL, OSAL_MEDIUM_PRIO);
    setup_CAN_id();

	//com_CAN_output_init();
	com_CAN_input_init();
	com_CAN_input_set_msg_callback(&input_msg_callback);

	com_osal_init();

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
    default: break;
    }
}
