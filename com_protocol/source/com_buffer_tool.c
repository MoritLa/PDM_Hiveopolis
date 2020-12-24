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

#ifdef CORE
#define BUF_HEAD_LEN    (sizeof(((ComMessage*)NULL)->contentId)+\
                         sizeof(((ComMessage*)NULL)->timestamp)+\
                         sizeof(((ComMessage*)NULL)->length)+\
                         sizeof(((ComMessage*)NULL)->destination))
#else
#define BUF_HEAD_LEN    (sizeof(((ComMessage*)NULL)->contentId)+\
                         sizeof(((ComMessage*)NULL)->timestamp)+\
                         sizeof(((ComMessage*)NULL)->length))
#endif

uint8_t queue_read(queue_t *queue, uint8_t nbBytes, uint8_t* data);
uint8_t queue_scan(queue_t * queue, uint8_t nbBytes, uint8_t offset, uint8_t* data);
uint8_t queue_write(queue_t *queue, uint8_t nbBytes, uint8_t* data);

uint8_t reset_write(queue_t *queue, uint8_t nbBytes);
uint8_t reset_read(queue_t *queue, uint8_t nbBytes);

uint8_t com_buffer_tool_write_message(queue_t * queue, uint8_t* dataLeft, ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data)
{
    if( dataLeft[IN] != 0 ||
        (queue->size-queue->filledBytes < (uint8_t)(BUF_HEAD_LEN+message.length)) ||
        !queue_write(queue, sizeof(message.contentId), &message.contentId))
        return BUFFER_ERROR;
    else if(!queue_write(queue, sizeof(message.timestamp), (uint8_t*) &message.timestamp))
    {
        reset_write(queue, sizeof(message.contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_write(queue, sizeof(message.length), &message.length))
    {
        reset_write(queue, sizeof(message.contentId)+sizeof(message.timestamp));
        return BUFFER_ERROR;
    }
#ifdef CORE
    else if(!queue_write(queue, sizeof(message.destination), &message.destination))
    {
        reset_write(queue, sizeof(message.contentId)+sizeof(message.timestamp)+sizeof(message.length));
        return BUFFER_ERROR;
    }
#endif
    else if (!queue_write(queue, message.length, message.data))
    {
        reset_write(queue, BUF_HEAD_LEN);
        return BUFFER_ERROR;
    }
    else
        return message.length;
}

uint8_t com_buffer_tool_write_header(queue_t * queue, uint8_t* dataLeft, ComMessage message)//uint8_t contentId, uint16_t timestamp, uint8_t length)
{
    if( dataLeft[IN] != 0 ||
        (queue->size-queue->filledBytes < (uint8_t)(BUF_HEAD_LEN)) ||
        !queue_write(queue, sizeof(message.contentId), &message.contentId))
        return BUFFER_ERROR;
    else if(!queue_write(queue, sizeof(message.timestamp), (uint8_t*) &message.timestamp))
    {
        reset_write(queue, sizeof(message.contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_write(queue, sizeof(message.length), &message.length))
    {
        reset_write(queue, sizeof(message.contentId)+sizeof(message.timestamp));
        return BUFFER_ERROR ;
    }
    else
    {
        dataLeft[IN] = message.length;
        return message.length;
    }
}

uint8_t com_buffer_tool_write_data(queue_t * queue, uint8_t* dataLeft, ComMessage message)//uint8_t length, uint8_t* data)
{
    uint8_t length = message.length;

    if( dataLeft[IN] == 0)
        return BUFFER_ERROR;
    else
    {
        if(length > dataLeft[IN])
            length = dataLeft[IN];

        if( (queue->size-queue->filledBytes < length) ||
            !queue_write(queue, length, message.data))
            return BUFFER_ERROR;
        else
        {
            dataLeft[IN] -= length;
            return length;
        }
    }
}

uint8_t com_buffer_tool_read_message(queue_t * queue, uint8_t* dataLeft, ComMessage* message)//uint8_t* contentId, uint16_t* timestamp, uint8_t* data)
{
    uint8_t length = com_buffer_tool_get_next_length(queue, dataLeft);
    message->length = 0;

    if( dataLeft[OUT] != 0 ||
        (queue->filledBytes < (uint8_t)(BUF_HEAD_LEN+length)) ||
        !queue_read(queue, sizeof(message->contentId), &message->contentId))
        return BUFFER_ERROR;
    else if(!queue_read(queue, sizeof(message->timestamp), (uint8_t*) &message->timestamp))
    {
        reset_read(queue, sizeof(message->contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_read(queue, sizeof(length), &length))
    {
        reset_read(queue, sizeof(message->contentId)+message->timestamp);
        return BUFFER_ERROR;
    }
    else if (!queue_read(queue, length, message->data))
    {
        reset_read(queue, BUF_HEAD_LEN);
        return BUFFER_ERROR;
    }
    else
    {
        message->length = length;
        return length;
    }
}

uint8_t com_buffer_tool_get_next_length(queue_t * queue, uint8_t* dataLeft)
{
    uint8_t length;
    if( dataLeft[OUT] != 0 ||
            queue->filledBytes<BUF_HEAD_LEN)
        return BUFFER_ERROR;
    queue_scan(queue, sizeof(length), sizeof(((ComMessage*)NULL)->contentId)+sizeof(((ComMessage*)NULL)->timestamp), &length);

    return length;
}

uint8_t com_buffer_tool_read_header(queue_t * queue, uint8_t* dataLeft, ComMessage* message)//uint8_t* contentId, uint16_t* timestamp)
{
    uint8_t length = com_buffer_tool_get_next_length(queue, dataLeft);
    message->length = 0;

    if( dataLeft[OUT] != 0 ||
        (queue->filledBytes < BUF_HEAD_LEN) ||
        !queue_read(queue, sizeof(message->contentId), &message->contentId))
        return BUFFER_ERROR;
    else if(!queue_read(queue, sizeof(message->timestamp), (uint8_t*) &message->timestamp))
    {
        reset_read(queue, sizeof(message->contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_read(queue, sizeof(length), &length))
    {
        reset_read(queue, sizeof(message->contentId)+sizeof(message->timestamp));
        return BUFFER_ERROR ;
    }
    else
    {
        dataLeft[OUT] = length;
        message->length = length;
        return length;
    }

}


uint8_t com_buffer_tool_read_data(queue_t * queue, uint8_t* dataLeft, ComMessage* message)//uint8_t length, uint8_t* data)
{
    uint8_t length = message->length;

    if( dataLeft[OUT] == 0)
        return BUFFER_ERROR;
    else
    {
        if(length > dataLeft[OUT])
            length = dataLeft[OUT];

        if( (queue->filledBytes < length) ||
            !queue_read(queue, length, message->data))
            return BUFFER_ERROR;
        else
        {
            dataLeft[OUT] -= length;
            return length;
        }
    }
}

bool com_buffer_tool_msg_available(queue_t* queue, uint8_t* dataLeft)
{
    if(dataLeft[OUT]==0 && queue->filledBytes >= BUF_HEAD_LEN+com_buffer_tool_get_next_length(queue, dataLeft))
        return true;
    else
        return false;
}

bool com_buffer_tool_msg_free(queue_t* queue, uint8_t* dataLeft, ComMessage message)
{
    if(dataLeft[IN]==0 && queue->size-queue->filledBytes >= message.length+BUF_HEAD_LEN)
        return true;
    else
        return false;
}

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
