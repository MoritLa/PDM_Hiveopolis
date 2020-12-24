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
#include "definition.h"

typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    size_t filledBytes;
    uint8_t* data;
} queue_t;

uint8_t com_buffer_tool_write_message(queue_t * queue, uint8_t* dataLeft, ComMessage message);//uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data);

uint8_t com_buffer_tool_write_header(queue_t * queue, uint8_t* dataLeft, ComMessage message);//uint8_t contentId, uint16_t timestamp, uint8_t length);

uint8_t com_buffer_tool_write_data(queue_t * queue, uint8_t* dataLeft, ComMessage message);//uint8_t length, uint8_t* data);

uint8_t com_buffer_tool_get_next_length(queue_t * queue, uint8_t* dataLeft);

uint8_t com_buffer_tool_read_message(queue_t * queue, uint8_t* dataLeft, ComMessage* message);//uint8_t* contentId, uint16_t* timestamp, uint8_t* data);

uint8_t com_buffer_tool_get_next_length(queue_t * queue, uint8_t* dataLeft);

uint8_t com_buffer_tool_read_header(queue_t * queue, uint8_t* dataLeft, ComMessage* message);//uint8_t* contentId, uint16_t* timestamp);

// reads parts of a datafield.
//  if length is bigger than the dataLeft, only dataLeft bytes are read and this
//  value is returned
uint8_t com_buffer_tool_read_data(queue_t * queue, uint8_t* dataLeft, ComMessage* message);//uint8_t length, uint8_t* data);

bool com_buffer_tool_msg_available(queue_t* queue, uint8_t* dataLeft);

bool com_buffer_tool_msg_free(queue_t* queue, uint8_t* dataLeft, ComMessage message);

#endif /* HEADER_COM_BUFFER_TOOL_H_ */
