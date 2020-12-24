/*
 * com_CAN_input.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_CAN_input.h"
#include "com_input_buffer.h"
#include "configuration.h"

static uint16_t CANId = 0x7FF;

static uint8_t bufferMap[NB_MODULES][2];

#define BUF_MAP_ID      0
#define BUF_MAP_BUF     1

uint8_t write_to_buffer(MyMessage inputMessage);


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
    for(uint8_t i=0; i<NB_MODULES;i++)
    {
        bufferMap[i][BUF_MAP_ID] = 0x000;
        bufferMap[i][BUF_MAP_BUF] = 0;
    }
    OSAL_CREATE_THREAD(CANReceive,NULL, OSAL_MEDIUM_PRIO);

}

void com_CAN_input_set_module_id(uint8_t mod_id)
{
    CANId = 0x0000;
    CANId =  (MODULE_TYPE)<<6 | mod_id;
}


uint8_t write_to_buffer(MyMessage message)
{
    uint8_t inputBufferId = 0xFF;
    uint8_t bytesWritten = 0;

    ComMessage bufMessage;

    for(uint8_t i=0; i<NB_MODULES; i++)
        if(bufferMap[i][BUF_MAP_ID]==message.id)
        {
            inputBufferId = bufferMap[i][BUF_MAP_BUF];
            break;
        }
    if(inputBufferId==0xFF)
        return 1;

    if(com_input_buffer_get_left_write(inputBufferId)!=0 && message.data8[0] == MOD_CONT)
    {
        bufMessage.length = message.length-1;
        bufMessage.data = message.data8;

        bytesWritten = com_input_buffer_write_data(inputBufferId, bufMessage);
        if(bytesWritten != message.length-1)
            return 0;
    }
    else if(com_input_buffer_get_left_write(inputBufferId)==0 && message.data8[0] == MOD_HEAD)
    {
        bufMessage.contentId = message.data8[MOD_HEAD_CONT];
        bufMessage.timestamp = (uint16_t) message.data8[MOD_HEAD_TIME];
        bufMessage.length = message.data8[MOD_HEAD_LENGTH];
        bufMessage.data = NULL;

        bytesWritten = com_input_buffer_write_header(inputBufferId, bufMessage);
        if(bytesWritten == MOD_HEAD_DLC-1)
            return 0;
    }
    else
    {
        return 0;
    }

    return 1;
}
