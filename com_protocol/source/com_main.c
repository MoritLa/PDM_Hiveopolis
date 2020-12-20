/*
 * com_main.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */
#include "com_main.h"
#include "com_CAN_output.h"
#include "com_output_buffer.h"
#include "com_osal.h"

bool com_main_init(void)
{
	com_output_buffer_init();
	com_CAN_output_init();
	return com_osal_init();
}

void com_main_end(void)
{
    com_osal_end();
}
