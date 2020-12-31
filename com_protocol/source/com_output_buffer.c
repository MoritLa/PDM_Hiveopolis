/*
 * com_output_buffer.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_output_buffer.h"
#include "com_buffer_tool.h"

#include "configuration.h"

#define OUT_BUF_SIZE		100//2*BURST_LENGTH//tests with 73 and 83 and 100

static uint8 outBuffer[OUT_BUF_SIZE];

static queue_t outputQueue ;

void com_output_buffer_init(void)
{
    com_buffer_tools_init_queue(&outputQueue, OUT_BUF_SIZE,outBuffer);
    com_buffer_tool_unblock_buffer(&outputQueue);
}

void com_output_buffer_clear_buffer(void)
{
    com_buffer_tools_clear_buffer(&outputQueue);
}

uint8 com_output_buffer_write_message(ComMessage message)
{
	return com_buffer_tool_write_message(&outputQueue, message);
}

uint8 com_output_buffer_write_header(ComMessage message)
{
    return com_buffer_tool_write_header(&outputQueue, message);
}

uint8 com_output_buffer_write_data(ComMessage message)
{
    return com_buffer_tool_write_data(&outputQueue, message);
}

uint8 com_output_buffer_read_message(ComMessage* message)
{
	return com_buffer_tool_read_message(&outputQueue, message);
}

uint8 com_output_buffer_get_next_length(void)
{
	return com_buffer_tool_get_next_length(&outputQueue);
}

uint8 com_output_buffer_read_header(ComMessage* message)
{
    return com_buffer_tool_read_header(&outputQueue, message);
}

uint8 com_output_buffer_read_data(ComMessage* message)
{
    return com_buffer_tool_read_data(&outputQueue, message);
}

uint8 com_output_buffer_get_left_write(void)
{
    return com_buffer_tool_get_left(&outputQueue, IN);
}

uint8 com_output_buffer_get_left_read(void)
{
    return com_buffer_tool_get_left(&outputQueue, OUT);
}

bool com_output_buffer_empty(void)
{
    return com_buffer_tool_empty(&outputQueue);
    return outputQueue.filledBytes == 0?true:false;
}

bool com_output_buffer_msg_available(void)
{
    return com_buffer_tool_msg_available(&outputQueue);
}

bool com_output_buffer_msg_free(ComMessage message)
{
    return com_buffer_tool_msg_free(&outputQueue, message);
}

bool com_output_buffer_half_full(void)
{
    return com_buffer_tool_half_full(&outputQueue);
}

bool com_output_buffer_undo_write(void)
{
    return com_buffer_tool_undo_write(&outputQueue);
}

bool com_output_buffer_undo_read(void)
{
    return com_buffer_tool_undo_read(&outputQueue);
}

uint8 com_output_buffer_burst_content_msg(uint8* data)
{
    (void) data;
	//outputQueue.tail=outputQueue.head-10;
	//outputQueue.filledBytes = 10;
    return 0;
}
