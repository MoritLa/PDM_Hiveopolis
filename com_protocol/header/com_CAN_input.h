/*
 * com_CAN_input.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef COM_CAN_INPUT_H_
#define COM_CAN_INPUT_H_


#include "com_osal.h"
#include "com_utilities.h"

void com_CAN_input_init(void);

void com_CAN_input_set_msg_callback(ComMessageCb protocolMsg);
void com_CAN_input_set_emergency_cb(ComMessageCb emergencyMsg);

void com_CAN_input_set_module_id(uint8 mod_id);

#endif /* COM_CAN_INPUT_H_ */
