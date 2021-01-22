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
        .btr = ( 1 /*39 45 52 159 181 211 1*/ << 0)  /* Baudrate prescaler (10 bits) */
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

void com_osal_setup_GPIO(void* GPIO, uint8 pin, uint8 dir)
{
    (void) GPIO;
    (void) pin;
    (void) dir;
}

uint8 com_osal_get_GPIO(void * GPIO, uint8 pin)
{
    switch((uint32)GPIO)
    {
    case (uint32)GPIOA: return palReadPad(GPIOA, pin); break;
    case (uint32)GPIOB: return palReadPad(GPIOB, pin); break;
    case (uint32)GPIOC: return palReadPad(GPIOC, pin); break;
    case (uint32)GPIOD: return palReadPad(GPIOD, pin); break;
    case (uint32)GPIOE: return palReadPad(GPIOD, pin); break;
    case (uint32)GPIOF: return palReadPad(GPIOD, pin); break;
    case (uint32)GPIOG: return palReadPad(GPIOD, pin); break;
    case (uint32)GPIOH: return palReadPad(GPIOD, pin); break;
    case (uint32)GPIOI: return palReadPad(GPIOD, pin); break;
    default: return (uint8) -1; break;
    }
}

void com_osal_set_GPIO(void * GPIO, uint8 pin, uint8 val)
{
    switch((uint32) GPIO)
    {
    case (uint32)GPIOA:
        if(val)
            palSetPad(GPIOA,pin);
        else
            palClearPad(GPIOA,pin);break;
    case (uint32)GPIOB:
        if(val)
            palSetPad(GPIOB,pin);
        else
            palClearPad(GPIOB,pin);break;
    case (uint32)GPIOC:
        if(val)
            palSetPad(GPIOC,pin);
        else
            palClearPad(GPIOC,pin);break;
    case (uint32)GPIOD:
        if(val)
            palSetPad(GPIOD,pin);
        else
            palClearPad(GPIOD,pin);break;
    case (uint32)GPIOE:
        if(val)
            palSetPad(GPIOE,pin);
        else
            palClearPad(GPIOE,pin);break;
    case (uint32)GPIOF:
        if(val)
            palSetPad(GPIOF,pin);
        else
            palClearPad(GPIOF,pin);break;
    case (uint32)GPIOG:
        if(val)
            palSetPad(GPIOG,pin);
        else
            palClearPad(GPIOG,pin);break;
    case (uint32)GPIOH:
        if(val)
            palSetPad(GPIOH,pin);
        else
            palClearPad(GPIOH,pin);break;
    case (uint32)GPIOI:
        if(val)
            palSetPad(GPIOI,pin);
        else
            palClearPad(GPIOI,pin);break;
    default: break;
    }
}

void com_osal_thread_sleep_ms(uint32 duration)
{
    chThdSleep(MS2ST(duration));
}

void com_osal_thread_sleep_us(uint32 duration)
{
    chThdSleep(US2ST(duration));
}

uint32 com_osal_get_systime_ms(void)
{
    return ST2MS(chVTGetSystemTime());
}

MUTEX_DECL(canLockMux) ;

uint8 com_osal_send_CAN(MyMessage CANMessage)
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

MyMessage com_osal_poll_CAN()
{
    MyMessage input;
    CANRxFrame rxf;

    msg_t m = canReceive(&CAND1, CAN_ANY_MAILBOX, &rxf, MS2ST(7));
    if (m != MSG_OK ||
            rxf.IDE ||
            rxf.RTR)
    {
        input.id = CAN_REC_ERROR;
        input.length = 0;
    }
    else
    {
        input.id = rxf.SID;
        input.length = rxf.DLC;
        input.data8[0] = rxf.data8[0];
        input.data8[1] = rxf.data8[1];
        input.data8[2] = rxf.data8[2];
        input.data8[3] = rxf.data8[3];
        input.data8[4] = rxf.data8[4];
        input.data8[5] = rxf.data8[5];
        input.data8[6] = rxf.data8[6];
        input.data8[7] = rxf.data8[7];
    }
    return input;
}

void com_osal_can_lock(void)
{
    chMtxLock(&canLockMux) ;
}

void com_osal_can_unlock(void)
{
    chMtxUnlock(&canLockMux) ;
}

#endif

#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

static int CAN_socket;

pthread_mutex_t CANlock = PTHREAD_MUTEX_INITIALIZER;

bool com_osal_init(void)
{
// init CAN
    int ret;

    struct sockaddr_can addr;
    struct ifreq ifr;

    //1.Create socket
    CAN_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (CAN_socket < 0)
        return false;

    //2.Specify can1 device
    strcpy(ifr.ifr_name, "can1");
    ret = ioctl(CAN_socket, SIOCGIFINDEX, &ifr);
    if (ret < 0)
        return false;

    //3.Bind the socket to can1
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(CAN_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
        return false;

    //4.Define receive rules
    struct can_filter rfilter[1];
    rfilter[0].can_id = 0x000;
    rfilter[0].can_mask = 0x000;
    setsockopt(CAN_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    // set timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 7000;
    setsockopt(CAN_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    return true;
}

void com_osal_end(void)
{
    //7.Close the socket and can0
    close(CAN_socket);
}

void com_osal_setup_GPIO(uint8 pin, uint8 dir)
{
    FILE *io,*iodir;
    char file[34];

    //activate pin
    io = fopen("/sys/class/gpio/export", "w");
    fseek(io,0,SEEK_SET);
    fprintf(io,"%d",pin);
    fflush(io);

    //set direction
    sprintf(file, "/sys/class/gpio/gpio%d/direction", pin);
    iodir = fopen(file, "w");
    fseek(iodir,0,SEEK_SET);
    if(dir == OUT)
        fprintf(iodir,"out");
    else
        fprintf(iodir,"in");
    fflush(iodir);

    fclose(io);
    fclose(iodir);
}

void com_osal_set_GPIO(uint8 pin, uint8 val)
{
    char file[30];
    FILE* ioval;

    sprintf(file, "/sys/class/gpio/gpio%d/value",pin);
    ioval = fopen(file, "w");
    fseek(ioval,0,SEEK_SET);
    if(val>1)
        val=1;
    fprintf(ioval,"%d",val);

    fclose(ioval);
}

uint8 com_osal_get_GPIO(uint8 pin)
{
    char file[30];
    FILE* ioval;
    char val=0;

    sprintf(file, "/sys/class/gpio/gpio%d/value",pin);
    ioval = fopen(file, "r");
    fseek(ioval,0,SEEK_SET);

    val = fgetc(ioval);

    fclose(ioval);
    if(val=='0')
        return 0;
    else if(val=='1')
        return 1;
    else
        return (uint8) -1;
}

void com_osal_thread_sleep_ms(uint32 duration)
{
    usleep(duration*1000);
}

void com_osal_thread_sleep_us(uint32 duration)
{
    usleep(duration);
}

uint32 com_osal_get_systime_ms(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000+tp.tv_usec/1000;
}

uint8 com_osal_send_CAN(MyMessage sendMessage)
{
    int nbytes;
    struct can_frame frame;
    memset(&frame, 0, sizeof(struct can_frame));


    //5.Set send data
    frame.can_id = sendMessage.id;
    frame.can_dlc = sendMessage.length;
    frame.data[0] = sendMessage.data8[0];
    frame.data[1] = sendMessage.data8[1];
    frame.data[2] = sendMessage.data8[2];
    frame.data[3] = sendMessage.data8[3];
    frame.data[4] = sendMessage.data8[4];
    frame.data[5] = sendMessage.data8[5];
    frame.data[6] = sendMessage.data8[6];
    frame.data[7] = sendMessage.data8[7];

    //6.Send message
    nbytes = write(CAN_socket, &frame, sizeof(frame));

    if(nbytes != sizeof(frame))
        return false;

    return true;
}

MyMessage com_osal_poll_CAN()
{
    MyMessage input;
    uint8 nbBytes;
    struct can_frame frame;
    memset(&frame, 0, sizeof(struct can_frame));

    nbBytes = read(CAN_socket, &frame, sizeof(frame));

    if (nbBytes == 0 ||
        (frame.can_id&(0x1<<29)) || // no remote transmission requests
        (frame.can_id&(0x1<<30)) || // no error messages
        (frame.can_id&(0x1<<31)) )  // no extended identifiers
    {
        input.id = CAN_REC_ERROR;
        input.length = 0;
    }
    else
    {
        input.id = frame.can_id;
        input.length = frame.can_dlc;
        input.data8[0] = frame.data[0];
        input.data8[1] = frame.data[1];
        input.data8[2] = frame.data[2];
        input.data8[3] = frame.data[3];
        input.data8[4] = frame.data[4];
        input.data8[5] = frame.data[5];
        input.data8[6] = frame.data[6];
        input.data8[7] = frame.data[7];
    }
    return input;
}

void com_osal_can_lock(void)
{
    pthread_mutex_lock(&CANlock);
}

void com_osal_can_unlock(void)
{
    pthread_mutex_unlock(&CANlock);
}
#endif
