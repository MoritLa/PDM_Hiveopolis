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
#include "definition.h"

void com_input_buffer_init(void);

uint8_t com_input_buffer_write_message(uint8_t bufferNb, ComMessage message);//uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data);

uint8_t com_input_buffer_write_header(uint8_t bufferNb, ComMessage message);//uint8_t contentId, uint16_t timestamp, uint8_t length);

uint8_t com_input_buffer_write_data(uint8_t bufferNb, ComMessage message);//uint8_t length, uint8_t* data);

uint8_t com_input_buffer_read_message(uint8_t bufferNb, ComMessage* message);//uint8_t* contentId, uint16_t* timestamp, uint8_t* data);

uint8_t com_input_buffer_get_next_length(uint8_t bufferNb);

uint8_t com_input_buffer_read_header(uint8_t bufferNb, ComMessage* message);//uint8_t* contentId, uint16_t* timestamp);

// reads parts of a datafield.
//  if length is bigger than the dataLeft, only dataLeft bytes are read and this
//  value is returned
uint8_t com_input_buffer_read_data(uint8_t bufferNb, ComMessage* message);//uint8_t length, uint8_t* data);


uint8_t com_input_buffer_get_left_write(uint8_t bufferNb);

uint8_t com_input_buffer_get_left_read(uint8_t bufferNb);

bool com_input_buffer_empty(uint8_t bufferNb);

bool com_input_buffer_msg_available(uint8_t bufferNb);

bool com_input_buffer_msg_free(uint8_t bufferNb, ComMessage message);

uint8_t com_input_buffer_burst_content_msg(uint8_t bufferNb, uint8_t* data);

#endif /* HEADER_COM_INPUT_BUFFER_H_ */
