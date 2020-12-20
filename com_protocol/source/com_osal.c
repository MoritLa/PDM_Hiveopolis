/*
 * com_osal.c
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#include "com_osal.h"

#ifdef CHIBIOS
#include "hal.h"

bool com_osal_init(void)
{
    static const CANConfig can1_config = {
        .mcr = (1 << 6)  /* Automatic bus-off management enabled. */
               | (1 << 2), /* Message are prioritized by order of arrival. */

        /* APB Clock is 42 Mhz
           42MHz / 2 / (1tq + 12tq + 8tq) = 1MHz => 1Mbit */
        .btr = (1 << 0)  /* Baudrate prescaler (10 bits) */
               | (11 << 16)/* Time segment 1 (3 bits) */
               | (7 << 20) /* Time segment 2 (3 bits) */
               | (0 << 24) /* Resync jump width (2 bits) */
    };

    canStart(&CAND1, &can1_config) ;
    return true;
}

void com_osal_end(void)
{

}

void com_osal_thread_sleep_ms(uint32_t duration)
{
    chThdSleep(MS2ST(duration));
}

uint32_t com_osal_get_systime_ms(void)
{
    return ST2MS(chVTGetSystemTime());
}

BSEMAPHORE_DECL(canMessagesSem, 0) ;
BSEMAPHORE_DECL(canLockSem, 0) ;

uint8_t com_osal_send_CAN(MyMessage CANMessage)
{
// Format MyCanMessage to CANTxFrame
    msg_t msg;

    CANTxFrame txf;
    txf.DLC = CANMessage.length;
    txf.RTR = 0;
    txf.IDE = 0;
    txf.SID = CANMessage.id;

    txf.data32[0] = CANMessage.data32[0] ;
    txf.data32[1] = CANMessage.data32[1];

    msg = canTransmit(&CAND1, CAN_ANY_MAILBOX, &txf, MS2ST(7));
    return msg == MSG_OK?true:false;
}

void can_lock(void)
{
    chBSemWait(&canLockSem) ;
}

void can_unlock(void)
{
    chBSemSignal(&canLockSem) ;
}

#endif

#ifdef LINUX
#include <unistd.h>
//#include <linux.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

static int CAN_socket;

bool com_osal_init(void)
{
// init CAN
    int ret;

    struct sockaddr_can addr;
    struct ifreq ifr;

    system("sudo ip link set can0 type can bitrate 100000");
    system("sudo ifconfig can0 up");

    //1.Create socket
    CAN_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (CAN_socket < 0)
        return false;

    //2.Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(CAN_socket, SIOCGIFINDEX, &ifr);
    if (ret < 0)
        return false;

    //3.Bind the socket to can0
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(CAN_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
        return false;

    //4.Define receive rules
    struct can_filter rfilter[1];
    rfilter[0].can_id = 0x000;
    rfilter[0].can_mask = CAN_SFF_MASK;
    setsockopt(CAN_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    return true;
}

void com_osal_end(void)
{
    //7.Close the socket and can0
    close(CAN_socket);
    system("sudo ifconfig can0 down");
}

void com_osal_thread_sleep_ms(uint32_t duration)
{
    usleep(duration*1000);
}

uint32_t com_osal_get_systime_ms(void)
{
    clock_t time = clock();

    return time/(CLOCKS_PER_SEC/1000);
}

uint8_t com_osal_send_CAN(MyMessage sendMessage)
{
    int nbytes;
    struct can_frame frame;
    memset(&frame, 0, sizeof(struct can_frame));


    //5.Set send data
    frame.can_id = sendMessage.id;
    frame.can_dlc = sendMessage.length;
    frame.data[0] = sendMessage.data8[0];
    frame.data[1] = sendMessage.data8[0];
    frame.data[2] = sendMessage.data8[0];
    frame.data[3] = sendMessage.data8[0];
    frame.data[4] = sendMessage.data8[0];
    frame.data[5] = sendMessage.data8[0];
    frame.data[6] = sendMessage.data8[0];
    frame.data[7] = sendMessage.data8[0];

    //6.Send message
    nbytes = write(CAN_socket, &frame, sizeof(frame));
    if(nbytes != sizeof(frame))
        return false;

    return true;
}
#endif
