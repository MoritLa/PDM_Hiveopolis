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

static uint8 inBuffer[NB_MODULES][IN_BUF_SIZE];

static uint8 burstBuffer[2*BURST_LENGTH];

static queue_t inputQueue[NB_MODULES+1];

void com_input_buffer_init(void)
{
    for(uint8 i=0; i<NB_MODULES;i++)
        com_buffer_tools_init_queue(&inputQueue[i],IN_BUF_SIZE, inBuffer[i]);

    com_buffer_tools_init_queue(&inputQueue[NB_MODULES], 2*BURST_LENGTH, burstBuffer);
}

void com_input_buffer_clear_buffer(uint8 bufferNb)
{
    com_buffer_tools_clear_buffer(&inputQueue[bufferNb]);
}

uint8 com_input_buffer_write_message(uint8 bufferNb, ComMessage message)
{
    return com_buffer_tool_write_message(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_write_header(uint8 bufferNb, ComMessage message)
{
    return com_buffer_tool_write_header(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_write_data(uint8 bufferNb, ComMessage message)
{
    return com_buffer_tool_write_data(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_read_message(uint8 bufferNb, ComMessage* message)
{
    return com_buffer_tool_read_message(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_get_next_length(uint8 bufferNb)
{
    return com_buffer_tool_get_next_length(&inputQueue[bufferNb]);
}

uint8 com_input_buffer_read_header(uint8 bufferNb, ComMessage* message)
{
    return com_buffer_tool_read_header(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_read_data(uint8 bufferNb, ComMessage* message)
{
    return com_buffer_tool_read_data(&inputQueue[bufferNb], message);
}

uint8 com_input_buffer_get_left_write(uint8 bufferNb)
{
    return com_buffer_tool_get_left(&inputQueue[bufferNb], IN);
    return inputQueue[bufferNb].dataLeft[IN];
}

uint8 com_input_buffer_get_left_read(uint8 bufferNb)
{
    return com_buffer_tool_get_left(&inputQueue[bufferNb], OUT);
    return inputQueue[bufferNb].dataLeft[OUT];
}

bool com_input_buffer_empty(uint8 bufferNb)
{
    return com_buffer_tool_empty(&inputQueue[bufferNb]);
    return inputQueue[bufferNb].filledBytes == 0?true:false;
}

bool com_input_buffer_msg_available(uint8 bufferNb)
{
    return com_buffer_tool_msg_available(&inputQueue[bufferNb]);
}

bool com_input_buffer_msg_free(uint8 bufferNb, ComMessage message)
{
    return com_buffer_tool_msg_free(&inputQueue[bufferNb], message);
}

bool com_input_buffer_half_full(uint8 bufferNb)
{
    return com_buffer_tool_half_full(&inputQueue[bufferNb]);
}


bool com_input_buffer_undo_read(uint8 bufferNb)
{
    return com_buffer_tool_undo_read(&inputQueue[bufferNb]);
}

bool com_input_buffer_undo_write(uint8 bufferNb)
{
    return com_buffer_tool_undo_write(&inputQueue[bufferNb]);
}

uint8 com_input_buffer_is_blocked(uint8 bufferNb)
{
    return com_buffer_tool_is_blocked(&inputQueue[bufferNb]);
}

void com_input_buffer_block_buffer(uint8 bufferNb)
{
    com_buffer_tool_block_buffer(&inputQueue[bufferNb]);
}

void com_input_buffer_unblock_buffer(uint8 bufferNb)
{
    com_buffer_tool_unblock_buffer(&inputQueue[bufferNb]);
}

uint8 com_input_buffer_set_origin(uint8 bufferNb, uint16 origin)
{
    return com_buffer_tool_set_origin(&inputQueue[bufferNb], origin);
}

uint16 com_input_buffer_get_origin(uint8 bufferNb)
{
    return com_buffer_tool_get_origin(&inputQueue[bufferNb]);
}

uint8 com_input_buffer_burst_content_msg(uint8 bufferNb, uint8* data)
{
    //inputQueue[bufferNb].tail=inputQueue[bufferNb].head-10;
    //inputQueue[bufferNb].filledBytes = 10;
    return 0;
}
