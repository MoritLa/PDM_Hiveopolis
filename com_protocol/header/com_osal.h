/*
 * com_osal.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_OSAL_H_
#define HEADER_COM_OSAL_H_

#include <stdbool.h>

#include "com_utilities.h"
#include "configuration.h"

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

void com_osal_setup_GPIO(void * GPIO, uint8 pin, uint8 dir);
uint8 com_osal_get_GPIO(void * GPIO, uint8 pin);
void com_osal_set_GPIO(void * GPIO, uint8 pin, uint8 val);

#endif

#ifdef LINUX
#define POSIX
/*
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef signed char int8;*/

void com_osal_setup_GPIO(uint8 pin, uint8 dir);
void com_osal_set_GPIO(uint8 pin, uint8 val);
uint8 com_osal_get_GPIO(uint8 pin);

#endif

#ifdef POSIX
#include <unistd.h>
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
void com_osal_thread_sleep_ms(uint32 duration);
void com_osal_thread_sleep_us(uint32 duration);

uint32 com_osal_get_systime_ms(void);


// everything needed for CAN

uint8 com_osal_send_CAN(struct MyMessage_struct CANMessage);
struct MyMessage_struct com_osal_poll_CAN(void);
void can_lock(void);
void can_unlock(void);

// buffer structure
typedef struct ComMessage_struct
{
    uint8 contentId ;
    uint16 timestamp;
    uint8 length;

#ifdef CORE
    uint16 destination;
#endif

    uint8* data;
} ComMessage;

#endif /* HEADER_COM_OSAL_H_ */
