/*
 * com_CAN_output.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_CAN_OUTPUT_H_
#define HEADER_COM_CAN_OUTPUT_H_

#include "com_osal.h"
#include "com_utilities.h"

// Initializes the CAN input module (buffer, input loop)
void com_CAN_output_init(void);

// Sets the module ID of the module, this ID together with the module type
//  define the CAN ID
void com_CAN_output_set_module_id(uint8 mod_id);

// Sends a protocol message with the msgId and the content of msg
//  For the core the destination of the message is defined by
//  msg.id
bool com_CAN_output_send_msg(uint8 msgId, MyMessage msg);

// Sends a message with the high priority ID with the msgId
//  For the core the destination of the message is defined by
//  msg.id
bool com_CAN_output_send_emergency_msg(uint8 msgId, MyMessage msg);

// Indicates to the output loop, that a burst has been accepted
void com_CAN_output_burst_accepted(void);

// Returns whether a burst has been accepted
//  This flag is reset as soon as the burst has been terminated.
bool com_CAN_output_get_burst_accepted(void);

#ifndef CORE
// Sets the chokes frames per second rate
void com_CAN_output_set_fps(uint8 fps);
#endif

#endif /* HEADER_COM_CAN_OUTPUT_H_ */
