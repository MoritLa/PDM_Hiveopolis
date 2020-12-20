/*
 * com_output_buffer.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_output_buffer.h"
#include "com_buffer_tool.h"

#include "configuration.h"

#define OUT_BUF_SIZE		2*BURST_LENGTH

static uint8_t buffer[OUT_BUF_SIZE];

static queue_t outputQueue = {.size = OUT_BUF_SIZE};

static uint8_t dataLeft = 0;

void com_output_buffer_init(void)
{
	dataLeft = 0;

	outputQueue.head = 0;
	outputQueue.tail = 0 ;
	outputQueue.filledBytes = 0;

	outputQueue.data = buffer;
}

uint8_t com_output_buffer_write_data(uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data)
{
	if(	(outputQueue.size-outputQueue.filledBytes < (uint8_t)(4+length)) ||
		!queue_write(&outputQueue, 1, &contentId))
	    return false;
	else if(!queue_write(&outputQueue, 2, (uint8_t*) &timestamp))
	{
	    reset_write(&outputQueue, 1);
	    return false;
	}
	else if(!queue_write(&outputQueue, 1, &length))
	{
        reset_write(&outputQueue, 3);
	    return false;
	}
	else if (!queue_write(&outputQueue, length, data))
	{
        reset_write(&outputQueue, 4);
	    return false;
	}
	else
		return true;
}

uint8_t com_output_buffer_read_message(uint8_t* contentId, uint16_t* timestamp, uint8_t* data)
{
	uint8_t length = com_output_buffer_get_next_length();

	if(	dataLeft != 0 ||
		(outputQueue.filledBytes < (uint8_t)(4+length)) ||
		!queue_read(&outputQueue, 1, contentId))
	    return 0;
	else if(!queue_read(&outputQueue, 2, (uint8_t*) timestamp))
    {
        reset_read(&outputQueue, 1);
        return 0;
    }
	else if(!queue_read(&outputQueue, 1, &length))
    {
        reset_read(&outputQueue, 3);
        return 0;
    }
	else if (!queue_read(&outputQueue, length, data))
    {
        reset_read(&outputQueue, 4);
        return 0;
    }
	else
		return length;
}

uint8_t com_output_buffer_get_next_length(void)
{
	uint8_t length;
	if( dataLeft != 0 ||
		outputQueue.filledBytes<4)
	    return 0;
	queue_scan(&outputQueue, 1, 3, &length);

	return length;
}

uint8_t com_output_buffer_read_header(uint8_t* contentId, uint16_t* timestamp)
{
	uint8_t length = com_output_buffer_get_next_length();

	if(	dataLeft != 0 ||
		(outputQueue.filledBytes < 4) ||
		!queue_read(&outputQueue, 1, contentId))
	    return 0;
	else if(!queue_read(&outputQueue, 2, (uint8_t*) timestamp))
	{
	    reset_read(&outputQueue, 1);
	    return 0;
	}
	else if(!queue_read(&outputQueue, 1, &length))
	{
        reset_read(&outputQueue, 3);
	    return 0 ;
	}
	else
	{
		dataLeft = length;
		return length;
	}

}

uint8_t com_output_buffer_read_data(uint8_t length, uint8_t* data)
{
	if( dataLeft == 0)
		return 0;
	else
	{
		if(length > dataLeft)
			length = dataLeft;

		if( (outputQueue.filledBytes < length) ||
			!queue_read(&outputQueue, length, data))
			return 0;
		else
		{
			dataLeft -= length;
			return length;
		}
	}
}

uint8_t com_output_buffer_get_left(void)
{
	return dataLeft;
}

bool com_output_buffer_empty(void)
{
    return outputQueue.filledBytes ==0?true:false;
}

uint8_t com_output_buffer_burst_content_msg(uint8_t* data)
{
	outputQueue.tail=outputQueue.head-10;
	outputQueue.filledBytes = 10;
    return 0;
}
