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

OSAL_DEFINE_THREAD(testDatarate, 1024, arg) {
    uint8 inMailbox[2];
    ComMessage msg;
    ComMessage inMsg;

    uint8 dataLength[13] = {50, 100, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 253};
    uint8 data[255];
    inMailbox[0] = com_open_mailbox(0x441);
    inMailbox[1] = com_open_mailbox(0x442);

    uint32 time_old, time_new;

    static uint16 buffers = 0;
    uint32 secondCount=0;
    uint32  msgCount[2]={0,0};
    uint16  burstByteCount=0;
    uint32  burstMsgCount[2]={0,0};

    bool burstStarted = com_get_burst_pending(BURST_BUFFER);

    msg.length = 1;

    time_old = com_osal_get_systime_ms();

    while(true)
    {
        for(uint8 i=0;i<13;i++)
        {
            com_osal_thread_sleep_us(400);

            msg.length = 1;
            msg.data = &dataLength[i];
            msg.destination = 0x441;
            com_send_data(msg);
            com_osal_thread_sleep_us(100);
            msg.destination = 0x442;
            com_send_data(msg);

            while(true)
            {
                time_new = com_osal_get_systime_ms();
                if((time_new > time_old && time_new >= time_old+10000) ||
                    (time_new < time_old && time_new >= 10000-(((uint32)-1)-time_old)))
                {
                    time_old = time_new;

                    msgCount[0] = 0;
                    msgCount[1] = 0;
                    //burstByteCount[0] = 0;
                    //burstByteCount[1] = 0;
                    burstMsgCount[0] = 0;
                    burstMsgCount[1] = 0;

                    secondCount++;
                    if (secondCount%6==0)//360)
                    {
                        break;
                    }
                }
                buffers = com_poll_mailbox();
                while(buffers&((1<<inMailbox[0])|(1<<inMailbox[1])|(1<<BURST_BUFFER)))
                {
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
                    if((buffers & (0x1<<BURST_BUFFER)) &&
                        com_get_origin(BURST_BUFFER)== (0x441&0x3FF))
                    {
                        inMsg = com_read_mailbox(BURST_BUFFER, data);
                        burstMsgCount[0] += inMsg.length;
                    }
                    if((buffers & (0x1<<BURST_BUFFER)) &&
                            com_get_origin(BURST_BUFFER)== (0x442&0x3FF))
                    {
                        inMsg = com_read_mailbox(BURST_BUFFER, data);
                        burstMsgCount[1] += inMsg.length;
                    }

                    buffers = com_poll_mailbox();
                }
                com_osal_thread_sleep_us(100);
            }
        }
    }
}

int main(void)
{
    uint8 inMailbox[3];

    halInit();
    chSysInit();
    mpu_init();

    com_init();
    ComMessage message;
    uint8 data[3]={1,2,3};
    uint8 i = 0;

    OSAL_CREATE_THREAD(testDatarate,NULL,OSAL_LOW_PRIO);

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
        com_osal_thread_sleep_us(20000);
    }
    return 1;
}


#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
