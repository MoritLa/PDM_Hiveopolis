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

OSAL_DEFINE_THREAD(testChoke1, 256, arg) {
    uint8 inMailbox[2];
    MyMessage msg;
    ComMessage inMsg;

//    uint8 fps[7] = {32, 45, 55, 60, 64, 70, 80};
    uint8 fps[7] = {57, 58, 59, 61, 62, 63, 65};
    uint8 data[16];
    inMailbox[0] = com_open_mailbox(0x441);
    inMailbox[1] = com_open_mailbox(0x442);

    uint32 time_old, time_new;
    time_old = com_osal_get_systime_ms();

    FILE* io = fopen("/home/debian/remote/messagecounts.csv", "w");
    fseek(io,0,SEEK_SET);
    fprintf(io,"Time;0x441;0x442;nominal,\n");
    fflush(io);

    uint32 secondCount=0;
    uint16  msgCount[2]={0,0};

    msg.length = 1;

    for(uint8 i=0;i<7;i++)
    {
        msg.data8[0] = fps[i];
        com_osal_can_lock();
        com_CAN_output_send_msg(CORE_CHOKE, msg);
        com_osal_can_unlock();
        while(true)
        {
            time_new = com_osal_get_systime_ms();
            if((time_new > time_old && time_new >= time_old+1000) ||
                (time_new < time_old && time_new >= 1000-(((uint32)-1)-time_old)))
            {
                time_old = time_new;
                fprintf(io,"%d;%d;%d;%d\n",secondCount, msgCount[0],msgCount[1],fps[i]);
                fflush(io);
                printf("1:%d;%d;%d;%d\n",secondCount, msgCount[0],msgCount[1],fps[i]);
                secondCount++;
                msgCount[0] = 0;
                msgCount[1] = 0;

                if (secondCount%20==0)//3600)
                {
                    break;
                }
            }
            if(com_poll_mailbox() & (0x1<<inMailbox[0]))
            {
                com_read_mailbox(inMailbox[0], data);
                msgCount[0]+=2;
            }
            if(com_poll_mailbox() & (0x1<<inMailbox[1]))
            {
                com_read_mailbox(inMailbox[1], data);
                msgCount[1]+=2;
            }
            com_osal_thread_sleep_ms(1);
        }
    }
    exit(1);
}

OSAL_DEFINE_THREAD(testChoke2, 256, arg) {
    uint8 inMailbox[2];
    MyMessage msg;
    ComMessage inMsg;

//    uint8 fps[7] = {32, 45, 55, 60, 64, 70, 80};
//    uint8 fps[7] = {57, 58, 59, 61, 62, 63, 65};
    uint8 fps[13] = {32, 45, 55, 58, 59, 60, 61, 62, 63, 64, 65, 70, 80};
    uint8 data[16];
    inMailbox[0] = com_open_mailbox(0x441);
    inMailbox[1] = com_open_mailbox(0x442);

    uint32 time_old, time_new;
    time_old = com_osal_get_systime_ms();

    FILE* io = fopen("/home/debian/remote/burstMessageCount.csv", "w");
    fseek(io,0,SEEK_SET);
    fprintf(io,"Time;0x441;0x442;0x441 burst; 0x442 burst;nominal,\n");
    fflush(io);

    uint64 buffers = 0;
    uint32 secondCount=0;
    uint16  msgCount[2]={0,0};
    uint16  burstByteCount=0;
    uint16  burstMsgCount[2]={0,0};

    bool burstStarted = com_get_burst_pending(BURST_BUFFER);

    msg.length = 1;

    for(uint8 i=0;i<13;i++)
    {
        msg.data8[0] = fps[i];
        com_CAN_output_send_msg(CORE_CHOKE, msg);
        while(true)
        {
            time_new = com_osal_get_systime_ms();
            if((time_new > time_old && time_new >= time_old+1000) ||
                (time_new < time_old && time_new >= 1000-(((uint32)-1)-time_old)))
            {
                time_old = time_new;
                fprintf(io,"%d;%d;%d;%d;%d;%d\n",secondCount, msgCount[0],msgCount[1], burstMsgCount[0],burstMsgCount[1],fps[i]);
                fflush(io);
                printf("%d;%d;%d;%d;%d;%d\n",secondCount, msgCount[0],msgCount[1], burstMsgCount[0],burstMsgCount[1],fps[i]);
                secondCount++;
                msgCount[0] = 0;
                msgCount[1] = 0;
                //burstByteCount[0] = 0;
                //burstByteCount[1] = 0;
                burstMsgCount[0] = 0;
                burstMsgCount[1] = 0;

                if (secondCount%20==0)//3600)
                {
                    break;
                }
            }
            buffers = com_poll_mailbox();
            while(buffers&((1<<inMailbox[0])|(1<<inMailbox[1])|((uint64)1<<BURST_BUFFER)))
            {
                if(burstStarted == false && com_get_burst_pending(BURST_BUFFER))
                    burstByteCount = 0;
                if(com_get_burst_pending(BURST_BUFFER))
                    burstStarted = true;
                if(buffers & (0x1<<inMailbox[0]))
                {
                    com_read_mailbox(inMailbox[0], data);
                    msgCount[0]+=2;
                }
                if(buffers & (0x1<<inMailbox[1]))
                {
                    com_read_mailbox(inMailbox[1], data);
                    msgCount[1]+=2;
                }
                if((buffers & ((uint64)0x1<<BURST_BUFFER)) &&
                    com_get_origin(BURST_BUFFER)== 0x441)
                {
                    inMsg = com_read_mailbox(BURST_BUFFER, data);
                    burstByteCount+=4+inMsg.length;
                    while(burstByteCount >= 7)
                    {
                        burstByteCount -= 7;
                        burstMsgCount[0]++;
                    }
                    if(burstStarted && !com_get_burst_pending(BURST_BUFFER))
                    {
                        burstStarted = false;
                        burstMsgCount[0]++;
                    }
                }
                if((buffers & ((uint64)0x1<<BURST_BUFFER)) &&
                        com_get_origin(BURST_BUFFER)== 0x442)
                {
                    inMsg = com_read_mailbox(BURST_BUFFER, data);
                    burstByteCount+=4+inMsg.length;
                    while(burstByteCount >= 7)
                    {
                        burstByteCount -= 7;
                        burstMsgCount[1]++;
                    }
                    if(burstStarted && !com_get_burst_pending(BURST_BUFFER))
                    {
                        burstStarted = false;
                        burstMsgCount[1]++;
                    }
                }
                buffers = com_poll_mailbox();
            }
            com_osal_thread_sleep_ms(1);
        }
    }
    exit(1);
}
OSAL_DEFINE_THREAD(testDatarate, 256, arg) {
    uint8 inMailbox[2];
    MyMessage msg;
    ComMessage inMsg;

//    uint8 fps[7] = {32, 45, 55, 60, 64, 70, 80};
//    uint8 fps[7] = {57, 58, 59, 61, 62, 63, 65};
    uint8 fps[13] = {32, 45, 55, 58, 59, 60, 61, 62, 63, 64, 65, 70, 80};
    uint8 data[255];
    inMailbox[0] = com_open_mailbox(0x441);
    inMailbox[1] = com_open_mailbox(0x442);

    uint32 time_old, time_new;

    FILE* io = fopen("/home/debian/remote/datarates.csv", "w");
    fseek(io,0,SEEK_SET);
    fprintf(io,"Time;0x441;0x442;0x441 burst; 0x442 burst;nominal,\n");
    fflush(io);

    uint64 buffers = 0;
    uint32 secondCount=0;
    uint32  msgCount[2]={0,0};
    uint16  burstByteCount=0;
    uint32  burstMsgCount[2]={0,0};

    bool burstStarted = com_get_burst_pending(BURST_BUFFER);

    msg.length = 1;
    //com_osal_thread_sleep_ms(50000);
    //com_input_buffer_clear_buffer(inMailbox[0]);
    //com_input_buffer_clear_buffer(inMailbox[1]);

    time_old = com_osal_get_systime_ms();

    for(uint8 i=0;i<1;i++)
    {
        msg.data8[0] = fps[i];
        //com_CAN_output_send_msg(CORE_CHOKE, msg);

        printf("start\n");
        while(true)
        {
            time_new = com_osal_get_systime_ms();
            if((time_new > time_old && time_new >= time_old+10000) ||
                (time_new < time_old && time_new >= 10000-(((uint32)-1)-time_old)))
            {
                time_old = time_new;
                if(secondCount>=5)
                {
                    fprintf(io,"%d;%d;%d;%d;%d;%d\n",10*secondCount, msgCount[0],msgCount[1], burstMsgCount[0],burstMsgCount[1],fps[i]);
                    fflush(io);
                }
                printf("%d;%d;%d;%d;%d;%d\n",10*secondCount, msgCount[0],msgCount[1], burstMsgCount[0],burstMsgCount[1],fps[i]);
                msgCount[0] = 0;
                msgCount[1] = 0;
                //burstByteCount[0] = 0;
                //burstByteCount[1] = 0;
                burstMsgCount[0] = 0;
                burstMsgCount[1] = 0;

                secondCount++;
                if (secondCount%26==0)//360)
                {
                    break;
                }
            }
            buffers = com_poll_mailbox();
            while(buffers&((1<<inMailbox[0])|(1<<inMailbox[1])|((uint64)1<<BURST_BUFFER)))
            {
                //printf("",)
                if(burstStarted == false && com_get_burst_pending(BURST_BUFFER))
                    burstByteCount = 0;
                if(com_get_burst_pending(BURST_BUFFER))
                    burstStarted = true;
                if(buffers & (0x1<<inMailbox[0]))
                {
                    inMsg = com_read_mailbox(inMailbox[0], data);
                    msgCount[0]+=inMsg.length;
                }
                if(buffers & (0x1<<inMailbox[1]))
                {
                    inMsg = com_read_mailbox(inMailbox[1], data);
                    msgCount[1]+=inMsg.length;
                }
                if((buffers & ((uint64)0x1<<BURST_BUFFER)) &&
                    com_get_origin(BURST_BUFFER)== 0x441)
                {
                    inMsg = com_read_mailbox(BURST_BUFFER, data);
                    burstMsgCount[0] += inMsg.length;
                }
                if((buffers & ((uint64)0x1<<BURST_BUFFER)) &&
                        com_get_origin(BURST_BUFFER)== 0x442)
                {
                    inMsg = com_read_mailbox(BURST_BUFFER, data);
                    burstMsgCount[1] += inMsg.length;
                }
                buffers = com_poll_mailbox();
            }
            com_osal_thread_sleep_us(100);
        }
    }
    exit(1);
}

OSAL_DEFINE_THREAD(read_buffer1, 256, arg) {
        uint8 inMailbox;
        int16 proxVal[8];
        ComMessage inMessage;

        inMailbox = com_open_mailbox(0x441);

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
                    printf("Proximity 1:\t");
                    for(uint8 i = 0; i<8; i++)
                        printf("\t%d",proxVal[i]);
                    printf("\n");
                    downsample = 0;
                }
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
                printf("Proximity 2:\t");
                for(uint8 i = 0; i<8; i++)
                    printf("\t%d",proxVal[i]);
                printf("\n");
                downsample = 0;
            }
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

/*    OSAL_CREATE_THREAD(read_buffer1,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer2,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer3,NULL, OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(read_buffer_burst,NULL, OSAL_MEDIUM_PRIO);*/

//    OSAL_CREATE_THREAD(testChoke1,NULL,OSAL_MEDIUM_PRIO);
//    OSAL_CREATE_THREAD(testChoke2,NULL,OSAL_MEDIUM_PRIO);
    OSAL_CREATE_THREAD(testDatarate,NULL,OSAL_MEDIUM_PRIO);

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
