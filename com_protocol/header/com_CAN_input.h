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

// Initializes the CAN input module (buffers, input loop)
void com_CAN_input_init(void);

// Sets a function of type ComMessageCb as protocol call back
//  This function is called, when a message is received that does not contain
//  data messages (ex. burst requests)
void com_CAN_input_set_msg_callback(ComMessageCb protocolMsg);

// Sets a function of type ComMessageCb as emergency call back
//  This function is called, when a message with high priority ID is received.
void com_CAN_input_set_emergency_cb(ComMessageCb emergencyMsg);

// Sets the module ID of the module, this ID together with the module type
//  define the CAN ID
void com_CAN_input_set_module_id(uint8 mod_id);

// Returns the CAN ID fof the module
uint16 com_CAN_input_get_CAN_id(void);

#endif /* COM_CAN_INPUT_H_ */
