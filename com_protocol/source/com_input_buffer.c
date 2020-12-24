/*
 * com_input_buffer.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_input_buffer.h"
#include "com_buffer_tool.h"

#include "configuration.h"

#define IN_BUF_SIZE        INPUT_BUF_LENGTH

static uint8_t inBuffer[NB_MODULES][IN_BUF_SIZE];

static uint8_t burstBuffer[2*BURST_LENGTH];

static queue_t inputQueue[NB_MODULES+1];

static uint8_t dataLeftIn[NB_MODULES+1][IN_OUT_SIZE];

void com_input_buffer_init(void)
{
    for(uint8_t i=0; i<NB_MODULES;i++)
    {
        dataLeftIn[i][IN] = 0;
        dataLeftIn[i][OUT] = 0;
        inputQueue[i].head = 0;
        inputQueue[i].tail = 0 ;
        inputQueue[i].filledBytes = 0;
        inputQueue[i].size = IN_BUF_SIZE;
        inputQueue[i].data = inBuffer[i];
    }
    dataLeftIn[NB_MODULES][IN]= 0;
    dataLeftIn[NB_MODULES][OUT]= 0;

    inputQueue[NB_MODULES].head = 0;
    inputQueue[NB_MODULES].tail = 0;
    inputQueue[NB_MODULES].filledBytes = 0;
    inputQueue[NB_MODULES].size = 2*BURST_LENGTH;
    inputQueue[NB_MODULES].data = burstBuffer;
}

uint8_t com_input_buffer_write_message(uint8_t bufferNb, ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data)
{
    return com_buffer_tool_write_message(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//contentId, timestamp, length, data);
}

uint8_t com_input_buffer_write_header(uint8_t bufferNb, ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length)
{
    return com_buffer_tool_write_header(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//contentId, timestamp, length);
}

uint8_t com_input_buffer_write_data(uint8_t bufferNb, ComMessage message)//uint8_t length, uint8_t* data)
{
    return com_buffer_tool_write_data(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//length, data);
}

uint8_t com_input_buffer_read_message(uint8_t bufferNb, ComMessage* message)//uint8_t* contentId, uint16_t* timestamp, uint8_t* data)
{
    return com_buffer_tool_read_message(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//contentId, timestamp, data);
}

uint8_t com_input_buffer_get_next_length(uint8_t bufferNb)
{
    return com_buffer_tool_get_next_length(&inputQueue[bufferNb], dataLeftIn[bufferNb]);
}

uint8_t com_input_buffer_read_header(uint8_t bufferNb, ComMessage* message)//uint8_t* contentId, uint16_t* timestamp)
{
    return com_buffer_tool_read_header(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//contentId, timestamp);
}

uint8_t com_input_buffer_read_data(uint8_t bufferNb, ComMessage* message)//uint8_t length, uint8_t* data)
{
    return com_buffer_tool_read_data(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);//length, data);
}

uint8_t com_input_buffer_get_left_write(uint8_t bufferNb)
{
    return dataLeftIn[bufferNb][IN];
}

uint8_t com_input_buffer_get_left_read(uint8_t bufferNb)
{
    return dataLeftIn[bufferNb][OUT];
}

bool com_input_buffer_empty(uint8_t bufferNb)
{
    return inputQueue[bufferNb].filledBytes == 0?true:false;
}

bool com_input_buffer_msg_available(uint8_t bufferNb)
{
    return com_buffer_tool_msg_available(&inputQueue[bufferNb], dataLeftIn[bufferNb]);
}

bool com_input_buffer_msg_free(uint8_t bufferNb, ComMessage message)
{
    return com_buffer_tool_msg_free(&inputQueue[bufferNb], dataLeftIn[bufferNb], message);
}

uint8_t com_input_buffer_burst_content_msg(uint8_t bufferNb, uint8_t* data)
{
    inputQueue[bufferNb].tail=inputQueue[bufferNb].head-10;
    inputQueue[bufferNb].filledBytes = 10;
    return 0;
}
