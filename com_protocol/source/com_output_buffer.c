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

static uint8_t outBuffer[OUT_BUF_SIZE];

static queue_t outputQueue = {.size = OUT_BUF_SIZE};

static uint8_t dataLeftOut[IN_OUT_SIZE] = {0,0};

void com_output_buffer_init(void)
{
	dataLeftOut[IN] = 0;
	dataLeftOut[OUT] = 0;

	outputQueue.head = 0;
	outputQueue.tail = 0 ;
	outputQueue.filledBytes = 0;

	outputQueue.data = outBuffer;
}

uint8_t com_output_buffer_write_message(ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data)
{
	return com_buffer_tool_write_message(&outputQueue, dataLeftOut, message);//contentId, timestamp, length, data);
}

uint8_t com_output_buffer_write_header(ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length)
{
    return com_buffer_tool_write_header(&outputQueue, dataLeftOut, message);//contentId, timestamp, length);
}

uint8_t com_output_buffer_write_data(ComMessage message)//uint8_t length, uint8_t* data)
{
    return com_buffer_tool_write_data(&outputQueue, dataLeftOut, message);//length, data);
}

uint8_t com_output_buffer_read_message(ComMessage* message)//uint8_t* contentId, uint16_t* timestamp, uint8_t* data)
{
	return com_buffer_tool_read_message(&outputQueue, dataLeftOut, message);//contentId, timestamp, data);
}

uint8_t com_output_buffer_get_next_length(void)
{
	return com_buffer_tool_get_next_length(&outputQueue, dataLeftOut);
}

uint8_t com_output_buffer_read_header(ComMessage* message)//uint8_t* contentId, uint16_t* timestamp)
{
    return com_buffer_tool_read_header(&outputQueue, dataLeftOut, message);//contentId, timestamp);
}

uint8_t com_output_buffer_read_data(ComMessage* message)//uint8_t length, uint8_t* data)
{
    return com_buffer_tool_read_data(&outputQueue, dataLeftOut, message);//length, data);
}

uint8_t com_output_buffer_get_left_write(void)
{
	return dataLeftOut[IN];
}

uint8_t com_output_buffer_get_left_read(void)
{
    return dataLeftOut[OUT];
}

bool com_output_buffer_empty(void)
{
    return outputQueue.filledBytes == 0?true:false;
}

bool com_output_buffer_msg_available(void)
{
    return com_buffer_tool_msg_available(&outputQueue, dataLeftOut);
}

bool com_output_buffer_msg_free(ComMessage message)
{
    return com_buffer_tool_msg_free(&outputQueue, dataLeftOut, message);
}

uint8_t com_output_buffer_burst_content_msg(uint8_t* data)
{
    (void) data;
	//outputQueue.tail=outputQueue.head-10;
	//outputQueue.filledBytes = 10;
    return 0;
}
