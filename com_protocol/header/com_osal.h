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

// Thread priorities
#define OSAL_IDLE_PRIO                          IDLEPRIO
#define OSAL_LOW_PRIO                           LOWPRIO
#define OSAL_MEDIUM_PRIO                        NORMALPRIO
#define OSAL_HIGH_PRIO                          HIGHPRIO
#define OSAL_HIGHEST_PRIO                       ABSPRIO

// Definition of a thread
#define OSAL_DEFINE_THREAD(tname, size, arg)    static THD_WORKING_AREA(wa ## tname, size) ;\
                                                static THD_FUNCTION(tname, arg)

// Starts the thread
#define OSAL_CREATE_THREAD(tname, arg, prio)    chThdCreateStatic(wa ## tname, sizeof(wa ## tname), prio, tname, arg);
#define OSAL_SET_CHANNEL_NAME(tname)            chRegSetThreadName(tname);

//Time functions
#define COM_OSAL_MAX_TIME   ST2MS((systime_t)-1)

// Sets up a GPIO port either as input or output
void com_osal_setup_GPIO(void * GPIO, uint8 pin, uint8 dir);

// Gets the value of the GPIO if it is configured as input
uint8 com_osal_get_GPIO(void * GPIO, uint8 pin);

// Sets the value of the GPIO if it is configured as output
void com_osal_set_GPIO(void * GPIO, uint8 pin, uint8 val);

#endif

#ifdef LINUX
#define POSIX

// Sets up a GPIO port either as input or output
void com_osal_setup_GPIO(uint8 pin, uint8 dir);

// Gets the value of the GPIO if it is configured as input
void com_osal_set_GPIO(uint8 pin, uint8 val);

// Sets the value of the GPIO if it is configured as output
uint8 com_osal_get_GPIO(uint8 pin);

#endif

#ifdef POSIX
#include <unistd.h>
#include "pthread.h"
#include "time.h"

// Thread priorities
#define OSAL_IDLE_PRIO                          1
#define OSAL_LOW_PRIO                           2
#define OSAL_MEDIUM_PRIO                        50
#define OSAL_HIGH_PRIO                          98
#define OSAL_HIGHEST_PRIO                       99

// Definition of a thread
#define OSAL_DEFINE_THREAD(tname, size, arg)    static size_t POSIX_SIZE_ ## tname = size;\
                                                void* tname ## _func(void* arg)

// Starts the thread
#define OSAL_CREATE_THREAD(tname, arg, prio)    {pthread_t tname;\
                                                pthread_attr_t attr;\
                                                pthread_attr_init(&attr);\
                                                pthread_attr_setstacksize(&attr, POSIX_SIZE_ ## tname);\
                                                pthread_create(&tname, &attr, tname ## _func, arg);\
                                                pthread_setschedprio(tname, prio);}
#define OSAL_SET_CHANNEL_NAME(tname)            //pthread_setname_np(pthread_self(), tname);

//Time functions
#define COM_OSAL_MAX_TIME   (sizeof(clock_t)*8/2)//ST2MS((systime_t)-1)

#endif

// Sets up the CAN interface
bool com_osal_init(void);

// Closes the CAN interface
void com_osal_end(void);

// Thread sleep function
void com_osal_thread_sleep_ms(uint32 duration);
void com_osal_thread_sleep_us(uint32 duration);

// Get system time
uint32 com_osal_get_systime_ms(void);


// everything needed for CAN
// Sends one CAN message
uint8 com_osal_send_CAN(struct MyMessage_struct CANMessage);

// Reads one CAN message from buffer
struct MyMessage_struct com_osal_poll_CAN(void);

// Mutex to block multiple access to the CAN interface
void com_osal_can_lock(void);
void com_osal_can_unlock(void);

#endif /* HEADER_COM_OSAL_H_ */
