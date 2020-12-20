/*
 * com_CAN_output.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */
#include <stdlib.h>
#include <stdbool.h>

#include "com_CAN_output.h"
#include "configuration.h"
#include "com_output_buffer.h"
#include "definition.h"

static uint32_t messagesSend[NB_MSG_P_S];
static bool slotFree[NB_MSG_P_S];
static uint8_t currentSlot = 0;

void update_free_slots(void);
bool fill_slot(void);


OSAL_DEFINE_THREAD(CANSend, 256, arg) {

    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void)arg;

    uint8_t downsample = 0;

    for(uint8_t i=0; i<NB_MSG_P_S; i++)
    {
        messagesSend[i] = com_osal_get_systime_ms()-1000;
        slotFree[i] = true;
    }
    currentSlot = 0;
    while(1){
        downsample++;
        if(downsample == 10)
        {
            update_free_slots();
            downsample = 0;
        }

        if(slotFree[currentSlot])
        {
            if(fill_slot())
            {
                slotFree[currentSlot]=false;
                messagesSend[currentSlot] = com_osal_get_systime_ms();
                currentSlot = (currentSlot+1)%NB_MSG_P_S;
            }
        }
        //com_osal_thread_sleep_ms(10);
    }
}

void com_CAN_output_init(void)
{
    OSAL_CREATE_THREAD(CANSend,NULL, OSAL_MEDIUM_PRIO);

}


void update_free_slots(void)
{
    uint32_t currentTime = com_osal_get_systime_ms();

    for(uint8_t i=0; i<NB_MSG_P_S; i++)
    {
        if(slotFree[i])
            continue;
        else
        {
            if( (currentTime>=1000 && messagesSend[i]<=currentTime-1000) ||
                (currentTime< 1000 && messagesSend[i]> currentTime
                                   && messagesSend[i]< COM_OSAL_MAX_TIME-1000+currentTime))
                slotFree[i] = true;
        }
    }
}

// What happens when write fails
bool fill_slot(void)
{
    MyMessage message;
    if(!com_output_buffer_empty())
    {
        if(com_output_buffer_get_left()!=0)
        {
            message.data8[0] = MOD_CONT;
            message.length = com_output_buffer_read_data(7, message.data8+MOD_CONT_DATA)+1;
        }
        else
        {
            message.data8[0] = MOD_CONT;
            message.data8[MOD_HEAD_LENGTH] = com_output_buffer_read_header(message.data8+MOD_HEAD_CONT, (uint16_t*) (message.data8+MOD_HEAD_TIME));
            message.length = MOD_HEAD_DLC;
        }
        message.id = CAN_ID;

        return com_osal_send_CAN(message);
    }
    else
        return false;
}
