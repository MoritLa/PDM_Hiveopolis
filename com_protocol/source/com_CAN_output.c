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

#ifdef CORE
static uint32 messagesSend[NB_MSG_P_S];
static bool slotFree[NB_MSG_P_S];
static uint8 currentSlot = 0;
#else
static unsigned char NB_MSG_P_S = 32;
static uint32 messagesSend[100];//NB_MSG_P_S];
static bool slotFree[100];//NB_MSG_P_S];
static uint8 currentSlot = 0;
#endif

static uint16 CANId = 0xFFFF;
static bool burstActive = false;
static bool burstAccepted = false;

void update_free_slots(void);
bool fill_slot(void);
uint8 write_burst(uint8 frameCount);


uint8 value1;

OSAL_DEFINE_THREAD(CANSend, 256, arg) {

    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void)arg;

    uint8 downsample = 0;
    uint8 frameCount = 0;
#ifdef CORE
    for(uint8 i=0; i<NB_MSG_P_S; i++)
#else
    for(uint8 i=0; i<100/*NB_MSG_P_S*/; i++)
#endif
    {
        messagesSend[i] = com_osal_get_systime_ms()-1000;
        slotFree[i] = true;
    }
    currentSlot = 0;

    while(1){
        if(currentSlot>=NB_MSG_P_S)
            currentSlot = 0;

        if(burstActive == true)
        {
            write_burst(frameCount);
            burstActive = false;
            burstAccepted = false;

            com_osal_thread_sleep_ms(1);
        }
        else
        {
            downsample++;
            if(downsample == 10)
            {
                update_free_slots();
                downsample = 0;
            }

            if((NB_MSG_P_S && slotFree[currentSlot]) ||
                (burstAccepted)) //prevent long
            {
                if(fill_slot())
                {
                    if(burstAccepted)
                        frameCount++;
                    else
                    {
                        slotFree[currentSlot]=false;
                        messagesSend[currentSlot] = com_osal_get_systime_ms();
                        currentSlot = (currentSlot+1)%NB_MSG_P_S;
                        frameCount = 0;
                    }
                }
            }
            if(burstAccepted==true && (com_output_buffer_get_left_read()==0))
                burstActive = true;
            else
                com_osal_thread_sleep_ms(1);
        }

    }
}

void com_CAN_output_init(void)
{
    CANId = 0xFFFF;
    burstActive = false;
    burstAccepted = false;

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

    if(CANId==0xFFFF)
        return false;
    tempMsg.id=(1<<10)|CANId;

    switch(msgId)
    {
#ifdef CORE
    case CORE_HEAD:
    case CORE_CONT:
    case CORE_BURST_ACCEPT:
    case CORE_HS:
        if(msg.length > 8-2)
            return false;
        tempMsg.length = msg.length+2;

        tempMsg.data8[0] =(msgId<<3) | (((uint8)(msg.id>>8))&0x3);
        tempMsg.data8[1] = (uint8)(msg.id&0xFF);
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+2] = msg.data8[i];

        com_osal_can_lock();
        canTransmit = com_osal_send_CAN(tempMsg);
        com_osal_can_unlock();
        break;
    case CORE_REG:
    case CORE_CHOKE:
        if(msg.length > 8-1)
            return false;
        tempMsg.length = msg.length+1;

        tempMsg.data8[0] = msgId<<3;
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+1] = msg.data8[i];

        com_osal_can_lock();
        canTransmit = com_osal_send_CAN(tempMsg);
        com_osal_can_unlock();
        break;
#else
    case MOD_HEAD:
    case MOD_CONT:
    case MOD_BURST_REQ:
    case MOD_BURST_CONT:
    case MOD_FILE_HEAD:
    case MOD_FILE_CONT:
    case MOD_REG:
    case MOD_HS:
        if(msg.length > 8-1)
            return false;
        tempMsg.length = msg.length+1;

        tempMsg.data8[0] = msgId<<3;
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+1] = msg.data8[i];
        com_osal_can_lock();
        canTransmit = com_osal_send_CAN(tempMsg);
        com_osal_can_unlock();
        break;
#endif
    default: break;
    }
    return canTransmit;
}

bool com_CAN_output_send_emergency_msg(uint8 msgId, MyMessage msg)
{
    bool canTransmit = false;
    MyMessage tempMsg;

    if(CANId==0xFFFF || msgId == HEARTBEAT)
        return false;
    tempMsg.id=CANId & (~(1<<10));

#ifdef CORE

        if(msg.length > 8-3)
            return false;
        tempMsg.length = msg.length+2;

        tempMsg.data8[0] = msgId;
        tempMsg.data8[1] = (uint8)((msg.id>>8)&0xFF);
        tempMsg.data8[2] = (uint8)(msg.id&0xFF);
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+3] = msg.data8[i];

        com_osal_can_lock();
        while(!com_osal_send_CAN(tempMsg))
            com_osal_thread_sleep_us(50);
//        canTransmit = com_osal_send_CAN(tempMsg);
        com_osal_can_unlock();

#else

        if(msg.length > 8-1)
            return false;
        tempMsg.length = msg.length+1;

        tempMsg.data8[0] = msgId;
        for(uint8 i=0; i<msg.length;i++)
            tempMsg.data8[i+1] = msg.data8[i];
        com_osal_can_lock();
        while(!com_osal_send_CAN(tempMsg))
            com_osal_thread_sleep_us(50);
        com_osal_can_unlock();
        //canTransmit = com_osal_send_CAN(tempMsg);
#endif

    return canTransmit;
}

void com_CAN_output_burst_accepted(void)
{
    burstAccepted = true;
}

bool com_CAN_output_get_burst_accepted(void)
{
    return burstAccepted;
}
#ifndef CORE
void com_CAN_output_set_fps(uint8 fps)
{
    NB_MSG_P_S = fps;
}
#endif

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
    if(!com_output_buffer_empty() || !com_output_buffer_is_blocked())
    {
        message.id = CANId | (1<<10);   //Low priority identifier
        if(com_output_buffer_get_left_read()!=0)
        {

#ifdef CORE
            if(lastDestination==0)
                return false;
            message.data8[0] =(CORE_CONT<<3) | (((uint8)(lastDestination>>8))&0x3);
            message.data8[1] = (uint8)(lastDestination&0xFF);
            bufMessage.data = message.data8+CORE_CONT_DATA;
            bufMessage.length = 8-CORE_CONT_DATA;
#else
            message.data8[0] =(MOD_CONT<<3);
            bufMessage.data = message.data8+MOD_CONT_DATA;
            bufMessage.length = 8-MOD_CONT_DATA;
#endif
            com_osal_can_lock();
            message.length = com_output_buffer_read_data(&bufMessage)+1;
            com_osal_can_unlock();
            if(message.length == BUFFER_ERROR)
                return false;

            com_osal_can_lock();
            canTransmit = com_osal_send_CAN(message);
            com_osal_can_unlock();
        }
        else
        {
            message.data8[MOD_HEAD_LENGTH] = com_output_buffer_read_header(&bufMessage);
            if(message.data8[MOD_HEAD_LENGTH] == BUFFER_ERROR)
                return false;
#ifdef CORE
            lastDestination = bufMessage.destination;
            message.data8[0] =(CORE_HEAD<<3) | (((uint8)(lastDestination>>8))&0x3);
            message.data8[1] = (uint8)(lastDestination&0xFF);
            message.data8[CORE_HEAD_CONT] = bufMessage.contentId;
            message.data8[CORE_HEAD_TIME] = (uint8) bufMessage.timestamp;
            message.data8[CORE_HEAD_TIME+1] = (uint8) (bufMessage.timestamp>>8);
            message.length = CORE_HEAD_DLC;
#else
            message.data8[0] = (MOD_HEAD<<3);
            message.data8[MOD_HEAD_CONT] = bufMessage.contentId;
            message.data8[MOD_HEAD_TIME] = (uint8) bufMessage.timestamp;
            message.data8[MOD_HEAD_TIME+1] = (uint8) (bufMessage.timestamp>>8);
            message.length = MOD_HEAD_DLC;
#endif
            com_osal_can_lock();
            canTransmit = com_osal_send_CAN(message);
            com_osal_can_unlock();
        }
        if(!canTransmit)
            com_output_buffer_undo_read();

        return canTransmit;
    }
    else
        return false;
}

uint8 write_burst(uint8 frameCount)
{
    MyMessage tempMsg;
    ComMessage msgContent;
    //uint16 frameCount = 0;
    uint8 framePointer = 0;
    uint8 msgHeadPointer = BUS_HEAD_LEN;
    uint8 endCode = 0;

    tempMsg.id = CANId | (1<<10);
    tempMsg.length = 8;
    tempMsg.data8[0] = MOD_BURST_CONT<<3;

    while(true)
    {
        for(framePointer=1;framePointer<8;)
        {
            if(msgHeadPointer < BUS_HEAD_LEN)
            {
                switch(msgHeadPointer)
                {
                case MOD_BURST_TIME_1:
                    tempMsg.data8[framePointer] = (uint8)msgContent.timestamp&0xFF;
                    break;
                case MOD_BURST_TIME_2:
                    tempMsg.data8[framePointer] = (uint8)((msgContent.timestamp>>8)&0xFF);
                    break;
                case MOD_BURST_CONT_ID:
                    tempMsg.data8[framePointer] = msgContent.contentId;
                    break;
                case MOD_BURST_LENGTH:
                    tempMsg.data8[framePointer] = msgContent.length;
                    break;
                default: break;
                }
                msgHeadPointer++;
                framePointer++;
            }
            else if(com_output_buffer_get_left_read()!=0)
            {
                msgContent.length = 8-framePointer;
                msgContent.data = tempMsg.data8+framePointer;
                com_osal_can_lock();
                framePointer += com_output_buffer_read_data(&msgContent);
                com_osal_can_unlock();
            }
            else
            {
                if(frameCount+(framePointer-1+com_output_buffer_get_next_length()+BUF_HEAD_LEN)/7>BURST_LENGTH/7 ||
                        com_output_buffer_empty())
                {
                    endCode = 2;
                    break;
                }
                com_output_buffer_read_header(&msgContent);
                if(msgContent.length == BUFFER_ERROR)
                {
                    endCode = 1;
                    break;
                }
                msgHeadPointer = 0;
            }
        }
        if(msgHeadPointer!=0)
        {
            tempMsg.length = framePointer;
//            com_osal_send_CAN(tempMsg);
            com_osal_can_lock();
            while(!com_osal_send_CAN(tempMsg))
                com_osal_thread_sleep_us(50);
            com_osal_can_unlock();
            frameCount++;

        }
        if(endCode)
            break;
    }

    tempMsg.length = 1;

    com_osal_can_lock();
    while(!com_osal_send_CAN(tempMsg))
        com_osal_thread_sleep_us(50);
    com_osal_can_unlock();

    com_osal_thread_sleep_ms(1);

    com_output_buffer_burst_terminated();

    return endCode-1;
}
