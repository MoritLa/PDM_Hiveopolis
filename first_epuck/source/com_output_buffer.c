/*
 * com_output_buffer.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_output_buffer.h"
#include "com_buffer_tool.h"

#include "configuration.h"

static uint8_t buffer[2*BURST_LENGTH];

static queue_t outputQueue;


void com_output_buffer_init(void)
{
	outputQueue.data = buffer;
	outputQueue.size = 2*BURST_LENGTH ;

	outputQueue.head = 0;
	outputQueue.tail = 0 ;
	outputQueue.filledBytes = 0;
}

uint8_t com_output_buffer_write_data(uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data)
{
	if(	!queue_write(&outputQueue, 1, &contentId) ||
		!queue_write(&outputQueue, 2, &timestamp) ||
		!queue_write(&outputQueue, 1, &length) ||
		!queue_write(&outputQueue, length, data) )
		return false;
	else
		return true;
}

uint8_t com_output_buffer_read_data(uint8_t* contentId, uint16_t* timestamp, uint8_t* data)
{
	uint8_t length;
	if(	!queue_read(&outputQueue, 1, &contentId) ||
		!queue_read(&outputQueue, 2, &timestamp) ||
		!queue_read(&outputQueue, 1, &length) ||
		!queue_read(&outputQueue, length, data))
		return 0 ;
	else
		return length;
}

uint8_t com_output_buffer_get_next_length(void)
{
	uint8_t length;

	queue_scan(&outputQueue, 1, 3, &length);

	return length;
}
