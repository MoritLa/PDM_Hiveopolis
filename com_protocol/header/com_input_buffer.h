/*
 * com_input_buffer.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_INPUT_BUFFER_H_
#define HEADER_COM_INPUT_BUFFER_H_
#include <stdlib.h>

#include "com_osal.h"
#include "com_utilities.h"

void com_input_buffer_init(void);

uint8 com_input_buffer_write_message(uint8 bufferNb, ComMessage message);

void com_input_buffer_clear_buffer(uint8 bufferNb);

uint8 com_input_buffer_write_header(uint8 bufferNb, ComMessage message);

uint8 com_input_buffer_write_data(uint8 bufferNb, ComMessage message);

uint8 com_input_buffer_read_message(uint8 bufferNb, ComMessage* message);

uint8 com_input_buffer_get_next_length(uint8 bufferNb);

uint8 com_input_buffer_read_header(uint8 bufferNb, ComMessage* message);

// reads parts of a datafield.
//  if length is bigger than the dataLeft, only dataLeft bytes are read and this
//  value is returned
uint8 com_input_buffer_read_data(uint8 bufferNb, ComMessage* message);


uint8 com_input_buffer_get_left_write(uint8 bufferNb);

uint8 com_input_buffer_get_left_read(uint8 bufferNb);

bool com_input_buffer_empty(uint8 bufferNb);

bool com_input_buffer_msg_available(uint8 bufferNb);

bool com_input_buffer_msg_free(uint8 bufferNb, ComMessage message);

bool com_input_buffer_half_full(uint8 bufferNb);

bool com_input_buffer_undo_read(uint8 bufferNb);

bool com_input_buffer_undo_write(uint8 bufferNb);

uint8 com_input_buffer_is_blocked(uint8 bufferNb);

void com_input_buffer_block_buffer(uint8 bufferNb);

void com_input_buffer_unblock_buffer(uint8 bufferNb);

uint8 com_input_buffer_set_origin(uint8 bufferNb, uint16 origin);

uint16 com_input_buffer_get_origin(uint8 bufferNb);

uint8 com_input_buffer_burst_content_msg(uint8 bufferNb, uint8* data);

#endif /* HEADER_COM_INPUT_BUFFER_H_ */
