/*
 * com_buffer_tool.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_BUFFER_TOOL_H_
#define HEADER_COM_BUFFER_TOOL_H_

#include <stdlib.h>
#include "com_osal.h"
#include "com_utilities.h"

typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    size_t filledBytes;
    uint8* data;

    uint8 dataLeft[IN_OUT_SIZE];
    uint8 lastOpsLength[IN_OUT_SIZE];
    uint8 lastOpsType[IN_OUT_SIZE];

    uint16 origin; // can only be set if buffer is blocked and empty and no data is left to be written
    uint8 blocked; // when buffer is blocked, no write action can be executed, unless data is left to be written
} queue_t;

enum lastOpsTypeDef {
        NO_OPS, MESSAGE_OPS, HEAD_OPS, DATA_OPS
};

void com_buffer_tools_init_queue(queue_t* queue, size_t bufSize, uint8* buffer);

void com_buffer_tools_clear_buffer(queue_t* queue);

uint8 com_buffer_tool_write_message(queue_t * queue, ComMessage message);

uint8 com_buffer_tool_write_header(queue_t * queue, ComMessage message);

uint8 com_buffer_tool_write_data(queue_t * queue, ComMessage message);

uint8 com_buffer_tool_read_message(queue_t * queue, ComMessage* message);

uint8 com_buffer_tool_get_next_length(queue_t * queue);

uint8 com_buffer_tool_read_header(queue_t * queue, ComMessage* message);

// reads parts of a datafield.
//  if length is bigger than the dataLeft, only dataLeft bytes are read and this
//  value is returned
uint8 com_buffer_tool_read_data(queue_t * queue, ComMessage* message);

uint8 com_buffer_tool_get_left(queue_t* queue, uint8 direction);

bool com_buffer_tool_empty(queue_t* queue);

bool com_buffer_tool_msg_available(queue_t* queue);

bool com_buffer_tool_msg_free(queue_t* queue, ComMessage message);

bool com_buffer_tool_half_full(queue_t* queue);

bool com_buffer_tool_undo_read(queue_t * queue);

bool com_buffer_tool_undo_write(queue_t * queue);

uint8 com_buffer_tool_is_blocked(queue_t *queue);

void com_buffer_tool_block_buffer(queue_t *queue);

void com_buffer_tool_unblock_buffer(queue_t *queue);

uint8 com_buffer_tool_set_origin(queue_t *queue, uint16 origin);

uint16 com_buffer_tool_get_origin(queue_t *queue);

#endif /* HEADER_COM_BUFFER_TOOL_H_ */
