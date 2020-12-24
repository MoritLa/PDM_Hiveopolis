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


bool com_init(void)
{
	return com_main_init();
}

void com_end(void)
{
    com_main_end();
}

uint8_t com_send_data(ComMessage message)
{
    return com_output_buffer_write_message(message);
}

uint64_t com_poll_mailbox(void)
{
    uint64_t mailboxes = 0;
    for(uint8_t i=0; i<NB_MODULES+1;i++)
        if(com_input_buffer_msg_available(i))
            mailboxes |= (0x1<<i);

    return mailboxes;
}
