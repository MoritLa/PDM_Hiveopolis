/*
 * com_buffer_tool.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_BUFFER_TOOL_H_
#define HEADER_COM_BUFFER_TOOL_H_

#include <stdlib.h>

typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    uint8_t filledBytes;
    uint8_t* data;
} queue_t;


uint8_t queue_read(queue_t *queue, uint8_t nbBytes, uint8_t* data);
uint8_t queue_scan(queue_t * queue, uint8_t nbBytes, uint8_t offset, uint8_t* data);
uint8_t queue_write(queue_t *queue, uint8_t nbBytes, uint8_t* data);

#endif /* HEADER_COM_BUFFER_TOOL_H_ */
