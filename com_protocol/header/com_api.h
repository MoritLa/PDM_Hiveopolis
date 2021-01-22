/*
 * com_api.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_API_H_
#define HEADER_COM_API_H_

#include "configuration.h"
#include "com_osal.h"
#include "com_utilities.h"

void com_init(void);
void com_end(void);

// Sets a function of type ComMessageCb as emergency callback function
//  It is called when an emergency message is received
void com_set_emergency_cb(ComMessageCb callback);

// Puts data in the output buffer
//  A message contains a timestamp, a content ID, Data length and the actual data
uint8 com_send_data(ComMessage message);

// Returns the state of each mailbox
//  for each mailbox a bit is set if contains an unread message
uint64 com_poll_mailbox(void);

// Returns the length of the datafield of the next message in the mailbox
uint8 com_get_message_length(uint8 mailbox);

// Returns the next message in the mailbox. The data is copied in the array
//  which is defined by the pointer data.
ComMessage com_read_mailbox(uint8 mailbox, uint8* data);

// Returns the CAN ID of the Mailbox.
uint16 com_get_origin(uint8 mailbox);

// Indicates, whether a burst has been requested by the module which is written
//  to a certain mailbox
bool com_get_burst_pending(uint8 mailbox);

// Opens a mailbox for a certain CAN ID (origin)  and returns a Mailbox number.
//  If the origin is already writen to a Mailbox, this one is returned.
uint8 com_open_mailbox(uint16 origin);


#endif /* HEADER_COM_API_H_ */
