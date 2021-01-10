/*
 * com_api.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_api.h"
#include "com_main.h"
#include "com_output_buffer.h"
#include "com_input_buffer.h"
#include "com_CAN_input.h"


void com_init(void)
{
	com_main_init();
}

void com_end(void)
{
    com_main_end();
}

void com_set_emergency_cb(ComMessageCb callback)
{
    com_CAN_input_set_emergency_cb(callback);
}

uint8 com_send_data(ComMessage message)
{
    return com_output_buffer_write_message(message);
}

uint8 com_output_queue_empty(void)
{
    return com_output_buffer_empty();
}

void com_clear_output_queue(void)
{
    com_output_buffer_clear_buffer();
    //send reset message on CAN
}

uint64 com_poll_mailbox(void)
{
    uint64 mailboxes = 0;
    for(uint8 i=0; i<=NB_MODULES;i++)
        if(com_input_buffer_msg_available(i))
            mailboxes = mailboxes|((uint64)0x1<<i);

    return mailboxes;
}

uint8 com_get_message_length(uint8 mailbox)
{
    return com_input_buffer_get_next_length(mailbox);
}

ComMessage com_read_mailbox(uint8 mailbox, uint8* data)
{
    ComMessage message;
    message.data = data;

    com_input_buffer_read_message(mailbox, &message);

    return message;
}

uint16 com_get_origin(uint8 mailbox)
{
    return com_input_buffer_get_origin(mailbox);
}

bool com_get_burst_pending(uint8 mailbox)
{
    return com_input_buffer_get_burst_request(mailbox);
}

uint8 com_open_mailbox(uint16 origin)
{
    uint8 mailbox = NO_MAILBOX;

    for(uint8 i=0; i<NB_MODULES; i++)
        if(com_input_buffer_get_origin(i)==origin)
            return i;

    //select available mailbox
    for(uint8 i=0; i<NB_MODULES; i++)
        if(com_input_buffer_is_blocked(i)&&
            com_input_buffer_empty(i))
        {
            mailbox = i;
            break;
        }
    // if there is no Mailbox available or allocation fails
    if(mailbox != NO_MAILBOX &&
        com_input_buffer_set_origin(mailbox, origin))
    {
        com_input_buffer_unblock_buffer(mailbox);
        return mailbox;
    }
    else
        return NO_MAILBOX;
    return NO_MAILBOX;
}

void com_close_mailbox(uint8 mailbox)
{
    com_input_buffer_block_buffer(mailbox);
}

void com_clear_mailbox(uint8 mailbox)
{
    com_input_buffer_clear_buffer(mailbox);
}
