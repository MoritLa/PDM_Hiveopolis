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

uint8 queue_read(queue_t *queue, uint8 nbBytes, uint8* data);
uint8 queue_scan(queue_t * queue, uint8 nbBytes, uint8 offset, uint8* data);
uint8 queue_write(queue_t *queue, uint8 nbBytes, uint8* data);

uint8 reset_write(queue_t *queue, uint8 nbBytes);
uint8 reset_read(queue_t *queue, uint8 nbBytes);

void com_buffer_tools_init_queue(queue_t* queue, size_t bufSize, uint8* buffer)
{
    queue->head = 0;
    queue->tail = 0 ;
    queue->filledBytes = 0;
    queue->size = bufSize;
    queue->data = buffer;

    queue->dataLeft[IN] = 0;
    queue->dataLeft[OUT] = 0;

    queue->lastOpsType[IN] = NO_OPS;
    queue->lastOpsType[OUT] = NO_OPS;
    queue->lastOpsLength[IN] = 0;
    queue->lastOpsLength[OUT] = 0;

    queue->origin = 0x7FF;
    queue->blocked = true;
    queue->burstPending = false;
}

void com_buffer_tools_clear_buffer(queue_t* queue)
{
    uint8 head;
    head = queue->head;
    com_buffer_tools_init_queue(queue, queue->size, queue->data);
    queue->head = head;
    queue->tail = head;
}

uint8 com_buffer_tool_write_message(queue_t * queue, ComMessage message)
{
    //*lastWrite=0;
    queue->lastOpsLength[IN] = 0;
    queue->lastOpsType[IN] = MESSAGE_OPS;

    if(message.length == BUFFER_ERROR)
        return BUFFER_ERROR;

    if( queue->blocked == true ||
        queue->dataLeft[IN] != 0 ||/*dataLeft[IN] != 0 ||*/
        (queue->size-queue->filledBytes < (uint8)(BUF_HEAD_LEN+message.length)) ||
        !queue_write(queue, sizeof(message.contentId), &message.contentId))
        return BUFFER_ERROR;
    else if(!queue_write(queue, sizeof(message.timestamp), (uint8*) &message.timestamp))
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
    else if(!queue_write(queue, sizeof(message.destination), (uint8*) &message.destination))
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
    {
        //*lastWrite = message.length+BUF_HEAD_LEN;
        queue->lastOpsLength[IN] = message.length+BUF_HEAD_LEN;
        return message.length;
    }
}

uint8 com_buffer_tool_write_header(queue_t * queue, ComMessage message)
{
    //*lastWrite = 0;
    queue->lastOpsLength[IN] = 0;
    queue->lastOpsType[IN] = HEAD_OPS;

    if( queue->blocked == true ||
        queue->dataLeft[IN] != 0 ||/*dataLeft[IN] != 0 ||*/
        (queue->size-queue->filledBytes < (uint8)(BUF_HEAD_LEN)) ||
        !queue_write(queue, sizeof(message.contentId), &message.contentId))
        return BUFFER_ERROR;
    else if(!queue_write(queue, sizeof(message.timestamp), (uint8*) &message.timestamp))
    {
        reset_write(queue, sizeof(message.contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_write(queue, sizeof(message.length), &message.length))
    {
        reset_write(queue, sizeof(message.contentId)+sizeof(message.timestamp));
        return BUFFER_ERROR ;
    }
#ifdef CORE
    else if(!queue_write(queue, sizeof(message.destination), (uint8*) &message.destination))
    {
        reset_write(queue, sizeof(message.contentId)+sizeof(message.timestamp)+sizeof(message.length));
        return BUFFER_ERROR;
    }
#endif
    else
    {
        //dataLeft[IN] = message.length;
        //*lastWrite = BUF_HEAD_LEN;
        queue->dataLeft[IN] = message.length;
        queue->lastOpsLength[IN] = BUF_HEAD_LEN;

        return message.length;
    }
}

uint8 com_buffer_tool_write_data(queue_t * queue, ComMessage message)
{
    uint8 length = message.length;

    queue->lastOpsLength[IN] = 0;
    queue->lastOpsType[IN] = DATA_OPS;

    if( queue->dataLeft[IN] == 0/*dataLeft[IN] == 0*/)
        return BUFFER_ERROR;
    else
    {
        //if(length > dataLeft[IN])
        //    length = dataLeft[IN];

        if(length > queue->dataLeft[IN])
            length = queue->dataLeft[IN];

        if( (queue->size-queue->filledBytes < length) ||
            !queue_write(queue, length, message.data))
            return BUFFER_ERROR;
        else
        {
            //dataLeft[IN] -= length;
            //*lastWrite = length;
            queue->dataLeft[IN] -= length;
            queue->lastOpsLength[IN] = length;
            if(queue->dataLeft[IN]>=20)
                return BUFFER_ERROR;
            return length;
        }
    }
}

uint8 com_buffer_tool_read_message(queue_t * queue, ComMessage* message)
{
    uint8 length = com_buffer_tool_get_next_length(queue);
    message->length = BUFFER_ERROR;

    //*lastRead = 0;
    queue->lastOpsLength[OUT] = 0;
    queue->lastOpsType[OUT] = MESSAGE_OPS;

    if( queue->dataLeft[OUT] != 0 ||//dataLeft[OUT] != 0 ||
        (queue->filledBytes < (uint8)(BUF_HEAD_LEN+length)) ||
        !queue_read(queue, sizeof(message->contentId), &message->contentId))
        return BUFFER_ERROR;
    else if(!queue_read(queue, sizeof(message->timestamp), (uint8*) &message->timestamp))
    {
        reset_read(queue, sizeof(message->contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_read(queue, sizeof(length), &length))
    {
        reset_read(queue, sizeof(message->contentId)+message->timestamp);
        return BUFFER_ERROR;
    }
#ifdef CORE
    else if(!queue_read(queue, sizeof(message->destination), (uint8*) &message->destination))
    {
        reset_read(queue, sizeof(message->contentId)+sizeof(message->timestamp)+sizeof(message->length));
        return BUFFER_ERROR;
    }
#endif
    else if (!queue_read(queue, length, message->data))
    {
        reset_read(queue, BUF_HEAD_LEN);
        return BUFFER_ERROR;
    }
    else
    {
        message->length = length;
        //*lastRead = length+BUF_HEAD_LEN;
        queue->lastOpsLength[OUT] = length+BUF_HEAD_LEN;
        return length;
    }
}

uint8 com_buffer_tool_get_next_length(queue_t * queue)
{
    uint8 length;
    if( queue->dataLeft[OUT] != 0 || //dataLeft[OUT] != 0 ||
            queue->filledBytes<BUF_HEAD_LEN)
        return BUFFER_ERROR;
    queue_scan(queue, sizeof(length), sizeof(((ComMessage*)NULL)->contentId)+sizeof(((ComMessage*)NULL)->timestamp), &length);

    return length;
}

uint8 com_buffer_tool_read_header(queue_t * queue, ComMessage* message)
{
    uint8 length = com_buffer_tool_get_next_length(queue);
    message->length = BUFFER_ERROR;

    //*lastRead = 0;
    queue->lastOpsLength[OUT] = 0;
    queue->lastOpsType[OUT] = HEAD_OPS;

    if( queue->dataLeft[OUT] != 0 ||//dataLeft[OUT] != 0 ||
        (queue->filledBytes < BUF_HEAD_LEN) ||
        !queue_read(queue, sizeof(message->contentId), &message->contentId))
        return BUFFER_ERROR;
    else if(!queue_read(queue, sizeof(message->timestamp), (uint8*) &message->timestamp))
    {
        reset_read(queue, sizeof(message->contentId));
        return BUFFER_ERROR;
    }
    else if(!queue_read(queue, sizeof(length), &length))
    {
        reset_read(queue, sizeof(message->contentId)+sizeof(message->timestamp));
        return BUFFER_ERROR ;
    }
#ifdef CORE
    else if(!queue_read(queue, sizeof(message->destination), (uint8*) &message->destination))
    {
        reset_read(queue, sizeof(message->contentId)+sizeof(message->timestamp)+sizeof(message->length));
        return BUFFER_ERROR;
    }
#endif
    else
    {
        message->length = length;

        //dataLeft[OUT] = length;
        //*lastRead = BUF_HEAD_LEN;
        queue->dataLeft[OUT] = length;
        queue->lastOpsLength[OUT] = BUF_HEAD_LEN;

        return length;
    }
}

uint8 com_buffer_tool_read_data(queue_t * queue, ComMessage* message)
{
    uint8 length = message->length;

    //*lastRead = 0;
    queue->lastOpsLength[OUT] = 0;
    queue->lastOpsType[OUT] = DATA_OPS;

    if( queue->dataLeft[OUT] == 0/*dataLeft[OUT] == 0*/)
        return BUFFER_ERROR;
    else
    {
        //if(length > dataLeft[OUT])
        //    length = dataLeft[OUT];
        if(length > queue->dataLeft[OUT])
            length = queue->dataLeft[OUT];

        if( (queue->filledBytes < length) ||
            !queue_read(queue, length, message->data))
            return BUFFER_ERROR;
        else
        {
            //dataLeft[OUT] -= length;
            //*lastRead = length;
            queue->dataLeft[OUT] -= length;
            queue->lastOpsLength[OUT] = length;

            return length;
        }
    }
}

uint8 com_buffer_tool_get_left(queue_t* queue, uint8 direction)
{
    return queue->dataLeft[direction];
}

bool com_buffer_tool_empty(queue_t* queue)
{
    return (queue->filledBytes==0 && queue->dataLeft[IN]==0)?true:false;
}

bool com_buffer_tool_msg_available(queue_t* queue)
{
    if(queue->dataLeft[OUT]==0/*dataLeft[OUT]==0*/ &&
        queue->filledBytes >= BUF_HEAD_LEN+com_buffer_tool_get_next_length(queue))
        return true;
    else
        return false;
}

bool com_buffer_tool_msg_free(queue_t* queue, ComMessage message)
{
    if(queue->blocked == false &&
        queue->dataLeft[IN]==0 /*dataLeft[IN]==0*/ &&
        queue->size-queue->filledBytes >= message.length+BUF_HEAD_LEN)
        return true;
    else
        return false;
}

bool com_buffer_tool_half_full(queue_t* queue)
{
    if((queue->size>>1) < queue->filledBytes)
        return true;
    else
        return false;
}

bool com_buffer_tool_undo_read(queue_t * queue)
{
    if(queue->lastOpsLength[OUT]>queue->size-queue->filledBytes)
    {
        queue->lastOpsLength[OUT] = 0;
        queue->lastOpsType[OUT] = NO_OPS;
        return false;
    }
    else
    {
        switch(queue->lastOpsType[OUT])
        {
        case NO_OPS: return true; break;
        case MESSAGE_OPS:
            reset_read(queue, queue->lastOpsLength[OUT]); break;
        case HEAD_OPS:
            reset_read(queue, queue->lastOpsLength[OUT]);
            queue->dataLeft[OUT] = 0; break;
        case DATA_OPS:
            reset_read(queue, queue->lastOpsLength[OUT]);
            queue->dataLeft[OUT] += queue->lastOpsLength[OUT]; break;
        default:
            queue->lastOpsLength[OUT] = 0;
            queue->lastOpsType[OUT] = NO_OPS;
            return false; break;
        }
    }

    queue->lastOpsLength[OUT] = 0;
    queue->lastOpsType[OUT] = NO_OPS;

    return true;
}

bool com_buffer_tool_undo_write(queue_t * queue)
{
    if(queue->blocked == true ||
        queue->lastOpsLength[IN]>queue->filledBytes)
    {
        queue->lastOpsLength[IN] = 0;
        queue->lastOpsType[IN] = NO_OPS;
        return false;
    }
    else
    {
        switch(queue->lastOpsType[IN])
        {
        case NO_OPS: return true; break;
        case MESSAGE_OPS:
            reset_write(queue, queue->lastOpsLength[IN]); break;
        case HEAD_OPS:
            reset_write(queue, queue->lastOpsLength[IN]);
            queue->dataLeft[IN] = 0; break;
        case DATA_OPS:
            reset_write(queue, queue->lastOpsLength[IN]);
            queue->dataLeft[IN] += queue->lastOpsLength[IN]; break;
        default:
            queue->lastOpsLength[IN] = 0;
            queue->lastOpsType[IN] = NO_OPS;
            return false; break;
        }
    }

    queue->lastOpsLength[IN] = 0;
    queue->lastOpsType[IN] = NO_OPS;

    return true;
}

uint8 com_buffer_tool_is_blocked(queue_t *queue)
{
    return queue->blocked;
}

void com_buffer_tool_block_buffer(queue_t *queue)
{
    queue->blocked = true;
}

void com_buffer_tool_unblock_buffer(queue_t *queue)
{
    queue->blocked = false;
}

uint8 com_buffer_tool_set_origin(queue_t *queue, uint16 origin)
{
    if(queue->blocked == true ||
        queue->dataLeft[IN] == 0 ||
        queue->filledBytes == 0)
    {
        queue->origin = origin&0x3FF;
        return true;
    }
    return false;
}

uint16 com_buffer_tool_get_origin(queue_t *queue)
{
    return queue->origin;
}

void com_buffer_tool_burst_requested(queue_t *queue)
{
    queue->burstPending = true;
}

void com_buffer_tool_burst_terminated(queue_t *queue)
{
    queue->burstPending = false;
}

bool com_buffer_tool_get_burst_request(queue_t *queue)
{
    return queue->burstPending;
}

uint8 queue_read(queue_t *queue, uint8 nbBytes, uint8* data)
{
	if (nbBytes > queue->filledBytes)
		return false;

    for(uint8 i = 0; i<nbBytes; i++)
    {
    	data[i] = queue->data[queue->tail];

    	queue->tail = (queue->tail + 1) % queue->size;
    }
    queue->filledBytes = queue->head>=queue->tail?
							queue->head-queue->tail:
							queue->head-queue->tail+queue->size;

    return true;
}

uint8 queue_scan(queue_t * queue, uint8 nbBytes, uint8 offset, uint8* data)
{
	if (nbBytes + offset > queue->filledBytes) {
		return false;
	}

	for(uint8 i = 0; i<nbBytes; i++)
		data[i] = queue->data[(queue->tail+offset+i)%queue->size];

	return true;
}

uint8 queue_write(queue_t *queue, uint8 nbBytes, uint8* data)
{
    if (nbBytes > queue->size-queue->filledBytes) {
        return false;
    }
    for(uint8 i = 0; i<nbBytes; i++)
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
uint8 reset_write(queue_t *queue, uint8 nbBytes)
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
uint8 reset_read(queue_t *queue, uint8 nbBytes)
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
