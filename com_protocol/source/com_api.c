/*
 * com_api.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_api.h"
#include "com_main.h"
#include "com_output_buffer.h"

bool com_init(void)
{
	return com_main_init();
}

void com_end(void)
{
    com_main_end();
}

uint8_t send_data(uint8_t msg_id, uint16_t timestamp, uint8_t length, uint8_t* data)
{
    return com_output_buffer_write_data(msg_id, timestamp, length, data);
}
