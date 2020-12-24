/*
 * com_CAN_output.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_CAN_OUTPUT_H_
#define HEADER_COM_CAN_OUTPUT_H_

#include "com_osal.h"
#include "definition.h"

void com_CAN_output_init(void);

void com_CAN_output_set_module_id(uint8_t mod_id);

#endif /* HEADER_COM_CAN_OUTPUT_H_ */
