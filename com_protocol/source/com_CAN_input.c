/*
 * com_CAN_input.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_CAN_input.h"
#include "com_CAN_output.h"
#include "com_input_buffer.h"
#include "configuration.h"

#ifdef CHIBIOS
#include "leds.h"
#endif

// Decides to which input buffer the inputMessage must be written
uint8 write_to_buffer(MyMessage inputMessage);

// Decodes a CAN message with a high priority ID, it calls the emergencyCb function
uint8 treat_emergency_message(MyMessage message);

// Decodes non data CAN messages, it calls the protocolCb function
uint8 treat_protocol_message(MyMessage message);

// Decodes a burst message and writes the content to the right buffer
uint8 treate_burst_message(MyMessage message);

// Callback function, they can be set by the user, default function does nothing
ComMessageCb protocolCb = &com_generic_message_cb;
ComMessageCb emergencyCb = &com_generic_message_cb;


static uint16 CANId = ID_NOT_SET;

// This thead collects all CAN frames and writes it to a buffer
OSAL_DEFINE_THREAD(CANReceive, 512, arg) {

    OSAL_SET_CHANNEL_NAME(__FUNCTION__) ;
    (void)arg;
    MyMessage inputMessage;

    while(1){
        com_osal_thread_sleep_us(30); // Approximate duration of one CAN frame

        //com_osal_can_lock();
        inputMessage = com_osal_poll_CAN();
        //com_osal_can_unlock();

        if(inputMessage.id == CAN_REC_ERROR) //time out or wrong message type
        {
            continue;
        }
        // debugging message
        if(write_to_buffer(inputMessage)!=1)
            com_CAN_output_send_emergency_msg(0x11,inputMessage);
    }
}

void com_CAN_input_init(void)
{
    CANId = ID_NOT_SET;

    com_input_buffer_init();

#ifndef CORE
    // CORE buffer is allways active
    com_input_buffer_set_origin(CORE_BUFFER, 0x000);
    com_input_buffer_unblock_buffer(CORE_BUFFER);
#endif

    OSAL_CREATE_THREAD(CANReceive,NULL, OSAL_HIGH_PRIO);

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

uint16 com_CAN_input_get_CAN_id(void)
{
    return CANId;
}


uint8 write_to_buffer(MyMessage message)
{
    uint8 inputBufferId = 0xFF;
    uint8 bytesWritten = 0;

    ComMessage bufMessage;

    for(uint8 i=0; i<NB_MODULES; i++)
        if((com_input_buffer_get_origin(i)&0x3FF)==(message.id&0x3FF))
        {
            inputBufferId = i;
            break;
        }
    if(inputBufferId==0xFF)// Id has no buffer (ignore frame)
        return 1;

    if(CANId == ID_NOT_SET)
        return 0;

    if((0x400&message.id)==0) //High priority frame
        return treat_emergency_message(message);
    else if((message.data8[0]&(0x1F<<3)) == (MOD_BURST_CONT<<3))// burst frame
//        return 1;
        return treate_burst_message(message);
#ifndef CORE
    else if(inputBufferId == CORE_BUFFER && // content frame from core
            com_input_buffer_get_left_write(inputBufferId)!=0 &&
            (((message.data8[0]&0x3)<<8)|message.data8[1])==CANId &&
            (message.data8[0]&(0x1F<<3)) == (CORE_CONT<<3))
    {
        bufMessage.length = message.length-2;
        bufMessage.data = message.data8+2;

        bytesWritten = com_input_buffer_write_data(inputBufferId, bufMessage);
        if(bytesWritten != message.length-1)
            return 0;
    }
    else if(inputBufferId == CORE_BUFFER && // header message from core
            com_input_buffer_get_left_write(inputBufferId)==0 &&
            (((message.data8[0]&0x3)<<8)|message.data8[1])==CANId &&
            (message.data8[0]&(0x1F<<3)) == (CORE_HEAD<<3))
    {
        bufMessage.contentId = message.data8[CORE_HEAD_CONT];
        bufMessage.timestamp = message.data8[CORE_HEAD_TIME]+ (((uint16)message.data8[CORE_HEAD_TIME+1])<<8);
        bufMessage.length = message.data8[CORE_HEAD_LENGTH];
        bufMessage.data = NULL;

        bytesWritten = com_input_buffer_write_header(inputBufferId, bufMessage);
        if(bytesWritten == MOD_HEAD_DLC-1)
            return 0;
    }
#endif
    else if(com_input_buffer_get_left_write(inputBufferId)!=0 && // content frame from other module
            ((message.data8[0]&(0x1F<<3)) == (MOD_CONT<<3)))
    {
        bufMessage.length = message.length-1;
        bufMessage.data = message.data8+1;

        bytesWritten = com_input_buffer_write_data(inputBufferId, bufMessage);
        if(bytesWritten != message.length-1)
            return 0;
    }
    else if(com_input_buffer_get_left_write(inputBufferId)==0 && // header frame from other module
            ((message.data8[0]&(0x1F<<3)) == (MOD_HEAD<<3)))
    {
        bufMessage.contentId = message.data8[MOD_HEAD_CONT];
        bufMessage.timestamp = message.data8[MOD_HEAD_TIME]+ (((uint16)message.data8[MOD_HEAD_TIME+1])<<8);
        bufMessage.length = message.data8[MOD_HEAD_LENGTH];
        bufMessage.data = NULL;
#ifdef CORE
        bufMessage.destination = 0x0000;
#endif
        bytesWritten = com_input_buffer_write_header(inputBufferId, bufMessage);
        if(bytesWritten == MOD_HEAD_DLC-1)
            return 0;
    }

    else
        return treat_protocol_message(message);
    return 1;
}

uint8 treat_emergency_message(MyMessage message)
{
    MyMessage tempMsg;

    if(CANId == ID_NOT_SET)
        return 0;

    if(message.data8[0] == HEARTBEAT)
    {
        if(message.length<1)
            return 2;
        tempMsg.id=message.id;
        tempMsg.length = message.length-1;
        for(uint8 i=0; i<message.length-1;i++)
            tempMsg.data8[i] = message.data8[i+1];
        protocolCb((message.data8[0]),tempMsg);
    }
    else if(message.id==CORE_CAN_ID)
    {
        if(!((((message.data8[0]&0x3)<<8)|message.data8[1])==CANId))
            return 1;
        if(message.length<3)
            return 2;
        tempMsg.id=message.id;
        tempMsg.length = message.length-3;
        for(uint8 i=0; i<message.length-3;i++)
            tempMsg.data8[i] = message.data8[i+3];
        emergencyCb((message.data8[0]),tempMsg);
    }
    else
    {
        if(message.length<1)
            return 2;
        tempMsg.id=message.id;
        tempMsg.length = message.length-1;
        for(uint8 i=0; i<message.length-1;i++)
            tempMsg.data8[i] = message.data8[i+1];
        emergencyCb((message.data8[0]),tempMsg);
    }
    return 1;
}

uint8 treat_protocol_message(MyMessage message)
{
    MyMessage tempMsg;

    if(CANId == ID_NOT_SET)
        return 0;

    switch(message.data8[0]>>3)
    {
    case CORE_HEAD:
    case CORE_CONT:
    case CORE_HS:
        // filter frames who have destination
        if(!((((message.data8[0]&0x3)<<8)|message.data8[1])==CANId))
            return 1;
    case CORE_BURST_ACCEPT:
        // don't filter frame, but store destination
        tempMsg.id=(((uint16)message.data8[0]&0x3)<<8)|message.data8[1];
        tempMsg.length = message.length-2;
        for(uint8 i=0; i<message.length-2;i++)
            tempMsg.data8[i] = message.data8[i+2];
        protocolCb((message.data8[0]>>3),tempMsg);
        break;
    case CORE_REG:
    case CORE_CHOKE:
    case MOD_HEAD:
    case MOD_CONT:
    case MOD_BURST_REQ:
    case MOD_BURST_CONT:
    case MOD_FILE_HEAD:
    case MOD_FILE_CONT:
    case MOD_REG:
    case MOD_HS:
        // treat message with no destination
        tempMsg.id=message.id;
        tempMsg.length = message.length-1;
        for(uint8 i=0; i<message.length-1;i++)
            tempMsg.data8[i] = message.data8[i+1];
        protocolCb((message.data8[0]>>3),tempMsg);
        break;
    default: break;
    }
    return 1;
}

uint8 treate_burst_message(MyMessage message)
{
    static ComMessage msgContent;
    uint8 framePointer = 0;
    static uint8 msgHeadPointer = 0;
    uint8 length;
    uint8 tempData;

    static uint8 msgCount = 0;

    //burst does not come from a module of interest -> burst buffer not set up
    if(com_input_buffer_get_origin(BURST_BUFFER) != (message.id&0x3FF))
        return 1;
    //write problem with burst buffer
    if(com_input_buffer_is_blocked(BURST_BUFFER))
        return 2;

    if(message.length==1)
    {
        //end burst
        msgContent.length = 1;
        msgContent.data = &tempData;

#ifdef CORE
        if(com_input_buffer_get_left_write(BURST_BUFFER))
        {
#ifdef CHIBIOS
            palTogglePad(GPIOD, GPIOD_LED_FRONT);
#endif
#ifdef LINUX
            printf("Frame incomplete\n");
#endif
        }
        while(com_input_buffer_get_left_write(BURST_BUFFER))
            com_input_buffer_write_data(BURST_BUFFER, msgContent);
#endif
        com_input_buffer_block_buffer(BURST_BUFFER);
        com_input_buffer_burst_terminated(BURST_BUFFER);

        msgCount = 0;
        return 1;
    }

    for(framePointer=1;framePointer<message.length;)
    {
        if(com_input_buffer_get_left_write(BURST_BUFFER) != 0) // There is data left to be written
        {
            msgContent.length = message.length-framePointer;
            msgContent.data = message.data8+framePointer;
            length = com_input_buffer_write_data(BURST_BUFFER, msgContent);
            if(length == BUFFER_ERROR)
                return 2;
            framePointer += length;
        }
        else if(msgHeadPointer == BUS_HEAD_LEN) // All parts of the header are received
        {
            if(com_input_buffer_write_header(BURST_BUFFER,msgContent) == BUFFER_ERROR)
                return 2;
            msgHeadPointer = 0;
        }
        else
        {
            // header is incomplete and header must be compleated
            switch(msgHeadPointer)
            {
            case MOD_BURST_TIME_1:
                msgContent.timestamp = (uint16)message.data8[framePointer];
                break;
            case MOD_BURST_TIME_2:
                msgContent.timestamp = (((uint16)message.data8[framePointer])<<8)|msgContent.timestamp;
                break;
            case MOD_BURST_CONT_ID:
                msgContent.contentId = message.data8[framePointer];
                break;
            case MOD_BURST_LENGTH:
                msgContent.length = message.data8[framePointer];
                break;
            default: break;
            }
            msgHeadPointer++;
            framePointer++;
        }
    }
    msgCount++;
    return 1;
}
