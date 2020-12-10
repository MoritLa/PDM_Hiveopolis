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
    if (queue->tail+nbBytes-1 >= queue->head) {
        return false;
    }

    for(uint8_T i = 0; i<nbBytes; i++)
    {
    	data[i] = queue->data[queue->tail];
    	queue->data[queue->tail] = 0x00;

    	queue->tail = (queue->tail + 1) % queue->size;
    }
    queue->filledBytes = queue->head>=queue->tail?
							queue->head-queue->tail:
							queue->head-queue->head+queue->size;

    return true;
}

uint8_t queue_scan(queue_t * queue, uint8_t nbBytes, uint8_t offset, uint8_t* data)
{
	if (queue->tail+nbBytes+offset-1 >= queue->head) {
		return false;
	}

	for(uint8_T i = 0; i<nbBytes; i++)
		data[i] = queue->data[queue->tail+offset+i];

	return true;
}

uint8_t queue_write(queue_t *queue, uint8_t nbBytes, uint8_t* data)
{
    if ( (nbBytes > queue->head - queue->tail && queue->head >= queue->tail) ||
    	 (nbBytes > queue->head - queue->tail + queue->size && queue->head < queue->tail)) {
        return false;
    }
    for(uint8_t i = 0; i<nbBytes; i++)
    {
    	queue->data[queue->head] = data[i];
    	queue->head = (queue->head + 1) % queue->size;
    }

    queue->filledBytes = queue->head>=queue->tail?
    						queue->head-queue->tail:
    						queue->head-queue->head+queue->size;

    return true;
}
