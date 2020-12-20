/*
 * com_osal.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_OSAL_H_
#define HEADER_COM_OSAL_H_

#include <stdbool.h>

#ifdef CHIBIOS
#include "ch.h"

#define OSAL_IDLE_PRIO                          IDLEPRIO
#define OSAL_LOW_PRIO                           LOWPRIO
#define OSAL_MEDIUM_PRIO                        NORMALPRIO
#define OSAL_HIGH_PRIO                          HIGHPRIO
#define OSAL_HIGHEST_PRIO                       ABSPRIO

#define OSAL_DEFINE_THREAD(tname, size, arg)    static THD_WORKING_AREA(wa ## tname, size) ;\
                                                static THD_FUNCTION(tname, arg)
#define OSAL_CREATE_THREAD(tname, arg, prio)    chThdCreateStatic(wa ## tname, sizeof(wa ## tname), prio, tname, arg);
#define OSAL_SET_CHANNEL_NAME(tname)            chRegSetThreadName(tname);

//Time functions
#define COM_OSAL_MAX_TIME   ST2MS((systime_t)-1)
#endif

#ifdef LINUX
#define POSIX

typedef unsigned long long uint64_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#endif

#ifdef POSIX
#include "pthread.h"
#include "time.h"

#define OSAL_IDLE_PRIO                          1
#define OSAL_LOW_PRIO                           2
#define OSAL_MEDIUM_PRIO                        50
#define OSAL_HIGH_PRIO                          98
#define OSAL_HIGHEST_PRIO                       99

#define OSAL_DEFINE_THREAD(tname, size, arg)    static size_t POSIX_SIZE = size;\
                                                void* tname ## _func(void* arg)

#define OSAL_CREATE_THREAD(tname, arg, prio)    pthread_t tname;\
                                                pthread_attr_t attr;\
                                                pthread_attr_init(&attr);\
                                                pthread_attr_setstacksize(&attr, POSIX_SIZE);\
                                                pthread_create(&tname, &attr, tname ## _func, arg);\
                                                pthread_setschedprio(tname, prio);
#define OSAL_SET_CHANNEL_NAME(tname)            //pthread_setname_np(pthread_self(), tname);

//Time functions
#define COM_OSAL_MAX_TIME   (sizeof(clock_t)*8/2)//ST2MS((systime_t)-1)

#endif

//if nowhere else defined uint8_t, etc. must be defined here.

bool com_osal_init(void);
void com_osal_end(void);

//Time functions
void com_osal_thread_sleep_ms(uint32_t duration);

uint32_t com_osal_get_systime_ms(void);


// everything needed for CAN
#define CAN_ID		0x001

typedef struct MyMessage_struct
{
    uint16_t id:11 ;
    uint8_t length:4;

    union{
        uint8_t data8[8] ;
        uint16_t data16[4] ;
        uint32_t data32[2];
    } ;
} MyMessage;

uint8_t com_osal_send_CAN(MyMessage CANMessage);
void can_lock(void);
void can_unlock(void);

#endif /* HEADER_COM_OSAL_H_ */
