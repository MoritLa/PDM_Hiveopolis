/*
 * main.c
 *
 *  Created on: 18.12.2020
 *      Author: morit
 */
#include <unistd.h>
#include "com_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <sys/types.h>
#include <sys/stat.h>

void my_exit(int argc, void* arg)
{
    (void)arg;

    com_end();
}

OSAL_DEFINE_THREAD(led_handler, 256, arg) {

    FILE* io = fopen("/tmp/pipes/pipeBack", "r");
    char line[100];
    uint8 length = 100;
    uint8 LED[4] = {false,false,false,false};
    uint16 * modules = arg;

    ComMessage message;
    uint8 data=0;

    message.contentId = 0x10;
    message.timestamp = 0x1234;
    message.length = 1;
    message.data = &data;

    while(true)
    {
        if(fgets(line, length, io))
        {
            sscanf(line,"%d;%d;%d;%d",&LED[0],&LED[1],&LED[2],&LED[3]);
            printf("Raw: %d;%d;%d;%d\n",LED[0],LED[1],LED[2],LED[3]);

            data = 0;
            for(uint8 i=0;i<4;i++)
                data |= LED[i]<<i;
            for(uint8 i=0; i<10;i++)
            {
                message.destination = modules[i];
                printf("Dest: %d\n", message.destination);
                com_send_data(message);
            }
        }
        usleep(100000);
    }
}

OSAL_DEFINE_THREAD(read_buffer, 512, arg) {

    uint8 inMailbox;
    uint8 data[253];
    char path[100];

    ComMessage inMsg;

    uint64 buffers;

    uint32 msgCount = 0;
    uint32 burstMsgCount;

    inMailbox = com_open_mailbox(*((uint16*) arg));
    sprintf(path, "/tmp/pipes/pipe%x", *((uint16*)arg));
    mkfifo(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );

    FILE* io = fopen(path, "w");

    uint32 time_old, time_new;

    while(true)
    {
        time_new = com_osal_get_systime_ms();
        if((time_new > time_old && time_new >= time_old+1000) ||
            (time_new < time_old && time_new >= 1000-(((uint32)-1)-time_old)))
        {
            time_old = time_new;

            printf("0x%x: %d, %d\n", *((uint16*) arg), msgCount, burstMsgCount);
            fprintf(io,"%d; %d\n", msgCount, burstMsgCount);
            fflush(io);

            msgCount = 0;
            burstMsgCount = 0;
        }
        buffers = com_poll_mailbox();
        while(buffers&((1<<inMailbox)|((uint64)1<<BURST_BUFFER)))
        {
            if(buffers & (0x1<<inMailbox))
            {
                inMsg = com_read_mailbox(inMailbox, data);
                msgCount+=inMsg.length;
            }
            if((buffers & ((uint64)0x1<<BURST_BUFFER)) &&
                com_get_origin(BURST_BUFFER)== com_get_origin(inMailbox))
            {
                inMsg = com_read_mailbox(BURST_BUFFER, data);
                burstMsgCount += inMsg.length;
            }

            buffers = com_poll_mailbox();
        }

        com_osal_thread_sleep_ms(1);
    }
}

int main(void)
{
    on_exit(my_exit, NULL);

    com_init();
    ComMessage message;
    uint8 data=0;

    uint16 modules[10] = {0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a};

    OSAL_CREATE_THREAD(led_handler, modules, OSAL_MEDIUM_PRIO);

    for(uint8 i = 0; i<10;i++)
        OSAL_CREATE_THREAD(read_buffer,&modules[i], OSAL_MEDIUM_PRIO);

    message.contentId = 0x10;
    message.timestamp = 0x1234;
    message.length = 1;
    message.data = &data;

    while(true)
    {
        /*data = (1<<0);
        for(uint8 i=0; i<10;i++)
        {
            message.destination = modules[i];
            com_send_data(message);
        }
        sleep(2);
        data = (1<<1);
        for(uint8 i=0; i<10;i++)
        {
            message.destination = modules[i];
            com_send_data(message);
        }
        sleep(2);
        data = (1<<2);
        for(uint8 i=0; i<10;i++)
        {
            message.destination = modules[i];
            com_send_data(message);
        }
        sleep(2);
        data = (1<<3);
        for(uint8 i=0; i<10;i++)
        {
            message.destination = modules[i];
            com_send_data(message);
        }*/
        sleep(2);
    }
    return 1;
}
