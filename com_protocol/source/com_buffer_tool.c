/*
 * com_buffer_tool.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 *
 *  Addapted from:
 *  https://gist.github.com/ryankurte/61f95dc71133561ed055ff62b33585f8
 */

#include "com_buffer_tool.h"

uint8_t queue_read(queue_t *queue, uint8_t nbBytes, uint8_t* data)
{
	if (nbBytes > queue->filledBytes)
		return false;

    for(uint8_t i = 0; i<nbBytes; i++)
    {
    	data[i] = queue->data[queue->tail];

    	queue->tail = (queue->tail + 1) % queue->size;
    }
    queue->filledBytes = queue->head>=queue->tail?
							queue->head-queue->tail:
							queue->head-queue->tail+queue->size;

    return true;
}

uint8_t queue_scan(queue_t * queue, uint8_t nbBytes, uint8_t offset, uint8_t* data)
{
	if (nbBytes + offset > queue->filledBytes) {
		return false;
	}

	for(uint8_t i = 0; i<nbBytes; i++)
		data[i] = queue->data[(queue->tail+offset+i)%queue->size];

	return true;
}

uint8_t queue_write(queue_t *queue, uint8_t nbBytes, uint8_t* data)
{
    if (nbBytes > queue->size-queue->filledBytes) {
        return false;
    }
    for(uint8_t i = 0; i<nbBytes; i++)
    {
    	queue->data[queue->head] = data[i];
    	queue->head = (queue->head + 1) % queue->size;
    }

    queue->filledBytes = queue->head>queue->tail?
    						queue->head-queue->tail:
    						queue->head-queue->tail+queue->size;

    return true;
}

//removes the last nbBytes written to the buffer from the fifo
//	returns false and empties the buffer if not enought bytes are available
uint8_t reset_write(queue_t *queue, uint8_t nbBytes)
{
	if (nbBytes > queue->filledBytes) {
		queue->head = queue->tail;
		queue->filledBytes = 0;
		return false;
	}
	else
	{
		if(queue->head >= nbBytes)
			queue->head -= nbBytes;
		else
			queue->head = queue->size-nbBytes+queue->head;

		queue->filledBytes -= nbBytes;

		return true;
	}
	return false;
}

//returns the last nbBytes bytes that have been read to the buffer
// returns false and fills the buffer if the buffer would overrun
uint8_t reset_read(queue_t *queue, uint8_t nbBytes)
{
    if (nbBytes > queue->size-queue->filledBytes)
    {
		queue->tail = queue->head;
		queue->filledBytes = queue->size;
		return false;
	}
    else
    {
    	if(queue->tail >= nbBytes)
				queue->tail -= nbBytes;
			else
				queue->tail = queue->size-nbBytes+queue->tail;

			queue->filledBytes += nbBytes;

			return true;
    }
    return false;
}
