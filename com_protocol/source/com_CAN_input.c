/*
 * com_CAN_input.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_CAN_input.h"
#include "com_input_buffer.h"
#include "configuration.h"

static uint16 CANId = 0x7FF;

uint8 write_to_buffer(MyMessage inputMessage);

ComMessageCb protocolCb = &com_generic_message_cb;
ComMessageCb emergencyCb = &com_generic_message_cb;

OSAL_DEFINE_THREAD(CANReceive, 256, arg) {

    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void)arg;
    MyMessage inputMessage;

    while(1){
        com_osal_thread_sleep_us(1000);

        inputMessage = com_osal_poll_CAN();
        if(inputMessage.id == CAN_REC_ERROR) //time out or wrong message type
            continue;
        write_to_buffer(inputMessage);
    }
}

void com_CAN_input_init(void)
{
    com_input_buffer_init();

#ifndef CORE
    com_input_buffer_set_origin(CORE_BUFFER, 0x000);
    com_input_buffer_unblock_buffer(CORE_BUFFER);
#endif
    OSAL_CREATE_THREAD(CANReceive,NULL, OSAL_MEDIUM_PRIO);

}

void com_CAN_input_set_msg_callback(ComMessageCb protocolMsg)
{
    protocolCb = protocolMsg;
}

void com_CAN_input_set_emergency_cb(ComMessageCb emergencyMsg)
{
    emergencyCb = emergencyMsg;
}

void com_CAN_input_set_module_id(uint8 mod_id)
{
    CANId = 0x0000;
    CANId =  (MODULE_TYPE)<<6 | mod_id;
}


uint8 write_to_buffer(MyMessage message)
{
    uint8 inputBufferId = 0xFF;
    uint8 bytesWritten = 0;

    ComMessage bufMessage;
    MyMessage tempMsg;

    for(uint8 i=0; i<NB_MODULES; i++)
        if((com_input_buffer_get_origin(i)&0x3FF)==(message.id&0x3FF))
        {
            inputBufferId = i;
            break;
        }
    if(inputBufferId==0xFF)
        return 1;
    if((0x400&message.id)==0)
    {
        if(message.data8[0] == HEARTBEAT)
        {
            tempMsg.id=message.id;
            tempMsg.length = message.length-1;
            for(uint8 i=0; i<message.length-1;i++)
                tempMsg.data8[i] = message.data8[i+1];
            protocolCb((message.data8[0]),tempMsg);
        }
        else if(message.id==CORE_CAN_ID)//treat emergency
        {
            tempMsg.id=message.id;
            tempMsg.length = message.length-2;
            for(uint8 i=0; i<message.length-2;i++)
                tempMsg.data8[i] = message.data8[i+2];
            emergencyCb((message.data8[0]),tempMsg);
        }
        else
        {
            tempMsg.id=message.id;
            tempMsg.length = message.length-1;
            for(uint8 i=0; i<message.length-1;i++)
                tempMsg.data8[i] = message.data8[i+1];
            emergencyCb((message.data8[0]),tempMsg);
        }
    }
    else if(com_input_buffer_get_left_write(inputBufferId)!=0 &&
            ((message.data8[0]&(0x1F<<3)) == (MOD_CONT<<3)
#ifndef CORE
             ||((inputBufferId == CORE_BUFFER &&
                 (message.data8[0]&0x7) == (((uint8)(CANId>>8))) &&
                 message.data8[1] == (uint8)(CANId&0xFF) &&
                (message.data8[0]&(0x1F<<3)) == (CORE_CONT<<3)))
#endif
            ))
    {
        bufMessage.length = message.length-1;
        bufMessage.data = message.data8+1;

        bytesWritten = com_input_buffer_write_data(inputBufferId, bufMessage);
        if(bytesWritten != message.length-1)
            return 0;
    }
    else if(com_input_buffer_get_left_write(inputBufferId)==0 &&
            ((message.data8[0]&(0x1F<<3)) == (MOD_HEAD<<3)
#ifndef CORE
             ||(inputBufferId == CORE_BUFFER &&
                 (message.data8[0]&0x7) == ((uint8)(CANId>>8)) &&
                 message.data8[1] == (uint8)(CANId&0xFF) &&
                (message.data8[0]&(0x1F<<3)) == (CORE_HEAD<<3))
#endif
            ))
    {
        bufMessage.contentId = message.data8[MOD_HEAD_CONT];
        bufMessage.timestamp = message.data8[MOD_HEAD_TIME]+ (((uint16)message.data8[MOD_HEAD_TIME+1])<<8);
        bufMessage.length = message.data8[MOD_HEAD_LENGTH];
        bufMessage.data = NULL;

        bytesWritten = com_input_buffer_write_header(inputBufferId, bufMessage);
        if(bytesWritten == MOD_HEAD_DLC-1)
            return 0;
    }
    else
    {
        switch(message.data8[0]>>3)
        {

        case CORE_HEAD:
        case CORE_CONT:
        case CORE_BURST_ACCEPT:
        case CORE_HS:
            tempMsg.id=message.id;
            tempMsg.length = message.length-2;
            for(uint8 i=0; i<message.length-2;i++)
                tempMsg.data8[i] = message.data8[i+2];
            protocolCb((message.data8[0]>>3),tempMsg);
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
            tempMsg.id=message.id;
            tempMsg.length = message.length-1;
            for(uint8 i=0; i<message.length-1;i++)
                tempMsg.data8[i] = message.data8[i+1];
            protocolCb((message.data8[0]>>3),tempMsg);
            break;
        default: break;
        }
    }
    return 1;
}
