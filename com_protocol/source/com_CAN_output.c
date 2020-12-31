/*
 * com_CAN_output.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */
#include <stdlib.h>
#include <stdbool.h>

#include "com_CAN_output.h"

#include "../header/com_utilities.h"
#include "configuration.h"
#include "com_output_buffer.h"

static uint32 messagesSend[NB_MSG_P_S];
static bool slotFree[NB_MSG_P_S];
static uint8 currentSlot = 0;

static uint16 CANId = 0xFFFF;

void update_free_slots(void);
bool fill_slot(void);


uint8 value1;

OSAL_DEFINE_THREAD(CANSend, 256, arg) {

    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void)arg;

    uint8 downsample = 0;

    for(uint8 i=0; i<NB_MSG_P_S; i++)
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
        com_osal_thread_sleep_ms(10);
    }
}

void com_CAN_output_init(void)
{
    com_output_buffer_init();

    OSAL_CREATE_THREAD(CANSend,NULL, OSAL_MEDIUM_PRIO);

}

void com_CAN_output_set_module_id(uint8 mod_id)
{
    CANId = 0x0000;
    CANId =  (MODULE_TYPE)<<6 | mod_id;
}

bool com_CAN_output_send_msg(uint8 msgId, MyMessage msg)
{
    bool canTransmit = false;
    MyMessage tempMsg;

    if(CanId==0xFFFF)
        return false;

    switch(msgId)
    {
    case CORE_HEAD:
    case CORE_CONT:
    case CORE_BURST_ACCEPT:
    case CORE_HS:
        tempMsg.id=CANId;
        tempMsg.length = msg.length+2;


        tempMsg.data8[0] =(MOD_CONT<<3) | (((uint8)(msg.id>>8))&0x7);
        tempMsg.data8[1] = (uint8)(msg.id&0xFF);
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+2] = msg.data8[i];

        canTransmit = com_osal_send_CAN(tempMsg);
        break;
    case CORE_REG:
    case MOD_HEAD:
    case MOD_CONT:
    case MOD_BURST_REQ:
    case MOD_BURST_CONT:
    case MOD_FILE_HEAD:
    case MOD_FILE_CONT:
    case MOD_REG:
    case MOD_HS:
        tempMsg.id=CANId;
        tempMsg.length = msg.length+1;

        tempMsg.data8[0] = msgId<<3;
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+1] = msg.data8[i];

        canTransmit = com_osal_send_CAN(tempMsg);
        break;
    default: break;
    }

    return canTransmit;
}

void update_free_slots(void)
{
    uint32 currentTime = com_osal_get_systime_ms();

    for(uint8 i=0; i<NB_MSG_P_S; i++)
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
    ComMessage bufMessage;
    bool canTransmit = false;
#ifdef CORE
    static uint16 lastDestination=0;
#endif
    if(CANId==0xFFFF)
        return false;
    if(!com_output_buffer_empty())
    {
        message.id = CANId | (1<<10);   //Low priority identifier
        if(com_output_buffer_get_left_read()!=0)
        {

#ifdef CORE
            if(lastDestination==0)
                return false;
            message.data8[0] =(MOD_CONT<<3) | (((uint8)(lastDestination>>8))&0x7);
            message.data8[1] = (uint8)(lastDestination&0xFF);
#else
            message.data8[0] =(MOD_CONT<<3);
#endif
            bufMessage.data = message.data8+MOD_CONT_DATA;
            bufMessage.length = 8-MOD_CONT_DATA;
            message.length = com_output_buffer_read_data(&bufMessage)+1;
            if(message.length == BUFFER_ERROR)
                return false;

            canTransmit = com_osal_send_CAN(message);
        }
        else
        {
            message.data8[MOD_HEAD_LENGTH] = com_output_buffer_read_header(&bufMessage);
            if(message.data8[MOD_HEAD_LENGTH] == BUFFER_ERROR)
                return false;
#ifdef CORE
            lastDestination = bufMessage.destination;
            message.data8[0] =(MOD_CONT<<3) | (((uint8)(lastDestination>>8))&0x7);
            message.data8[1] = (uint8)(lastDestination&0xFF);
#else
            message.data8[0] = (MOD_HEAD<<3);
#endif
            message.data8[MOD_HEAD_CONT] = bufMessage.contentId;
            message.data8[MOD_HEAD_TIME] = (uint8) bufMessage.timestamp;
            message.data8[MOD_HEAD_TIME+1] = (uint8) (bufMessage.timestamp>>8);
            message.length = MOD_HEAD_DLC;

            canTransmit = com_osal_send_CAN(message);
        }
        if(!canTransmit)
            com_output_buffer_undo_read();

        return canTransmit;
    }
    else
        return false;
}
