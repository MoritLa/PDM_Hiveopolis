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

void com_set_emergency_cb(ComMessageCb callback);

uint8 com_send_data(ComMessage message);

uint64 com_poll_mailbox(void);

uint8 com_get_message_length(uint8 mailbox);

ComMessage com_read_mailbox(uint8 mailbox, uint8* data);

uint16 com_get_origin(uint8 mailbox);

uint8 com_open_mailbox(uint16 origin);


#endif /* HEADER_COM_API_H_ */
