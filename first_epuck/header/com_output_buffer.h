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

void com_output_buffer_init(void);

uint8_t com_output_buffer_write_data(uint8_t contentId, uint16_t timestamp, uint8_t length, uint8_t* data);

uint8_t com_output_buffer_read_data(uint8_t* contentId, uint16_t* timestamp, uint8_t* data);

uint8_t com_output_buffer_get_next_length(void);

#endif /* HEADER_COM_OUTPUT_BUFFER_H_ */
