/*
 * main.c
 *
 *  Created on: 18.12.2020
 *      Author: morit
 */
#include <unistd.h>
#include "com_api.h"

void my_exit(int argc, void* arg)
{
    (void)arg;

    com_end();
}

int main(void)
{
    on_exit(my_exit, NULL);

    com_init();
    ComMessage message;
    uint8_t data[3]={1,2,3};
    uint8_t i = 0;

    message.contentId = 0x12;
    message.timestamp = 0x1234;
    message.length = 3;
    message.data = data;

    /*for(uint8_t j=0; j<50; j++)
    {
        send_data(message);
    }*/

    while(true)
    {
        if(i<=64)
        {
            i++;
            com_send_data(message);
        }
        usleep(15000);
    }
    return 1;
}
