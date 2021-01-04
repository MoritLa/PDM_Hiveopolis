/*
 * com_output_buffer.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_OUTPUT_BUFFER_H_
#define HEADER_COM_OUTPUT_BUFFER_H_

#include <stdlib.h>

#include "com_osal.h"
#include "com_utilities.h"

void com_output_buffer_init(void);

void com_output_buffer_clear_buffer(void);

uint8 com_output_buffer_write_message(ComMessage message);

uint8 com_output_buffer_write_header(ComMessage message);

uint8 com_output_buffer_write_data(ComMessage message);

uint8 com_output_buffer_read_message(ComMessage* message);

uint8 com_output_buffer_get_next_length(void);

uint8 com_output_buffer_read_header(ComMessage* message);

// reads parts of a datafield.
//	if length is bigger than the dataLeft, only dataLeft bytes are read and this
//	value is returned
uint8 com_output_buffer_read_data(ComMessage* message);


uint8 com_output_buffer_get_left_write(void);

uint8 com_output_buffer_get_left_read(void);

bool com_output_buffer_empty(void);

bool com_output_buffer_msg_available(void);

bool com_output_buffer_msg_free(ComMessage message);

bool com_output_buffer_half_full(void);

bool com_output_buffer_undo_write(void);

bool com_output_buffer_undo_read(void);

void com_output_buffer_burst_requested(void);

void com_output_buffer_burst_terminated(void);

bool com_output_buffer_get_burst_request(void);

#endif /* HEADER_COM_OUTPUT_BUFFER_H_ */
