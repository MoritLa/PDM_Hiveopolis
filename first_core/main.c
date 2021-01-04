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

void my_exit(int argc, void* arg)
{
    (void)arg;

    com_end();
}

OSAL_DEFINE_THREAD(read_buffer1, 256, arg) {
        uint8 inMailbox;
        int16 proxVal[8];
        ComMessage inMessage;

        inMailbox = com_open_mailbox(0x441);

        uint8 downsample=0;

        uint32 time_old, time_new;
        time_old = com_osal_get_systime_ms();

        FILE* io = fopen("/home/debian/remote/0x441_messagecounts.csv", "w");
        fseek(io,0,SEEK_SET);
        fprintf(io,"Second;number of messages\n");
        fflush(io);
        uint32 secondCount=0;
        uint16  msgCount=0;
        while(true)
        {
            time_new = com_osal_get_systime_ms();
            if((time_new > time_old && time_new >= time_old+1000) ||
                (time_new < time_old && time_new >= 1000-(((uint32)-1)-time_old)))
            {
                time_old = time_new;
                fprintf(io,"%d;%d\n",secondCount, msgCount);
                fflush(io);
                printf("1:%d;%d\n",secondCount, msgCount);
                secondCount++;
                msgCount = 0;
            }
            while(com_poll_mailbox() & (0x1<<inMailbox))
            {
                com_read_mailbox(inMailbox, (uint8*) proxVal);
                msgCount+=2;
/*                downsample++;
                if(com_get_message_length(inMailbox)!=16)
                {
                    inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                    continue;
                }
                inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                if(inMessage.length == BUFFER_ERROR)
                    continue;

                if(downsample == 100)
                {
                    printf("Proximity 1:\t");
                    for(uint8 i = 0; i<8; i++)
                        printf("\t%d",proxVal[i]);
                    printf("\n");
                    downsample = 0;
                }*/
            }

            com_osal_thread_sleep_ms(1);
        }
}

OSAL_DEFINE_THREAD(read_buffer2, 256, arg) {
    uint8 inMailbox;
    int16 proxVal[8];
    ComMessage inMessage;

    inMailbox = com_open_mailbox(0x442);

    uint8 downsample=0;

    uint32 time_old, time_new, time_start;
    time_old = com_osal_get_systime_ms();
    time_start = time_old;
    FILE* io = fopen("/home/debian/remote/0x442_messagecounts.csv", "w");
    fseek(io,0,SEEK_SET);
    fprintf(io,"Second;number of messages\n");
    fflush(io);
    uint32 secondCount=0;
    uint16  msgCount=0;

    while(true)
    {
        time_new = com_osal_get_systime_ms();
        if((time_new > time_old && time_new >= time_old+1000) ||
            (time_new < time_old && time_new >= 1000-(((uint32)-1)-time_old)))
        {
            time_old = time_new;
            fprintf(io,"%d;%d\n",secondCount, msgCount);
            fflush(io);
            printf("2:%d;%d\n",secondCount, msgCount);
            secondCount++;
            msgCount = 0;
            if(secondCount == 3600)
                exit(1);
        }
        if(secondCount%10==0)
        {
            com_CAN_output_send_msg(CORE_CHOKE,msg);
        }

        while(com_poll_mailbox() & (0x1<<inMailbox))
        {
            com_read_mailbox(inMailbox, (uint8*) proxVal);
            msgCount+=2;
/*                downsample++;
            if(com_get_message_length(inMailbox)!=16)
            {
                inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                continue;
            }
            inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
            if(inMessage.length == BUFFER_ERROR)
                continue;

            if(downsample == 100)
            {
                printf("Proximity 2:\t");
                for(uint8 i = 0; i<8; i++)
                    printf("\t%d",proxVal[i]);
                printf("\n");
                downsample = 0;
            }*/
        }

        com_osal_thread_sleep_ms(1);
    }
}

OSAL_DEFINE_THREAD(read_buffer3, 256, arg) {
        uint8 inMailbox;
        int16 proxVal[8];
        ComMessage inMessage;

        inMailbox = com_open_mailbox(0x443);

        uint8 downsample=0;

        while(true)
        {
            while(com_poll_mailbox() & (0x1<<inMailbox))
            {
                downsample++;
                if(com_get_message_length(inMailbox)!=16)
                {
                    inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                    continue;
                }
                inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                if(inMessage.length == BUFFER_ERROR)
                    continue;

                if(downsample == 100)
                {
                    printf("Proximity 3:\t");
                    for(uint8 i = 0; i<8; i++)
                        printf("\t%d",proxVal[i]);
                    printf("\n");
                    downsample = 0;
                }
            }

            com_osal_thread_sleep_ms(1);
        }
}

OSAL_DEFINE_THREAD(read_buffer_burst, 256, arg) {
        uint8 inMailbox;
        int16 proxVal[8];
        ComMessage inMessage;

        inMailbox = BURST_BUFFER;

        uint8 downsample=0;

        while(true)
        {
            while(com_poll_mailbox() & (((uint64)0x1)<<inMailbox))
            {
                downsample++;
                if(com_get_message_length(inMailbox)!=16)
                {
                    inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                    continue;
                }
                inMessage = com_read_mailbox(inMailbox, (uint8*) proxVal);
                if(inMessage.length == BUFFER_ERROR)
                    continue;

                if(downsample == 50)
                {
                    printf("Proximity (burst) %d:", com_get_origin(inMailbox)&0x3);
                    for(uint8 i = 0; i<8; i++)
                        printf("\t%d",proxVal[i]);
                    printf("\n");
                    downsample = 0;
                }
            }

            com_osal_thread_sleep_ms(1);
        }
}

int main(void)
{
    on_exit(my_exit, NULL);
    uint8 inMailbox[3];
    com_init();
    ComMessage message;
    uint8 data[3]={1,2,3};
    uint8 i = 0;

    OSAL_CREATE_THREAD(read_buffer1,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer2,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer3,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer_burst,NULL, OSAL_MEDIUM_PRIO);

    message.contentId = 0x12;
    message.timestamp = 0x1234;
    message.length = 3;
    message.data = data;
    message.destination = 0x501;

    /*for(uint8 j=0; j<50; j++)
    {
        send_data(message);
    }*/

    inMailbox[0] = com_open_mailbox(0x441);
    inMailbox[1] = com_open_mailbox(0x442);
    inMailbox[2] = com_open_mailbox(0x443);

    while(true)
    {
        //printf("%x \n",com_input_buffer_msg_available(BURST_BUFFER));
        //com_send_data(message);
        usleep(20000);
    }
    return 1;
}
