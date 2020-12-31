/*
 * configuration.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_CONFIGURATION_H_
#define HEADER_CONFIGURATION_H_

#include "ch.h"

#define CHIBIOS

#define BURST_LENGTH	    1024
#define NB_MSG_P_S          32

#define INPUT_BUF_LENGTH    256
#define NB_MODULES          2

#define USE_SECOND_BUS	    true

#define MODULE_TYPE         0x1

#define ADRESS_PIN0         GPIOC, GPIOC_SEL_0
#define ADRESS_PIN1         GPIOC, GPIOC_SEL_1
#define ADRESS_PIN2         GPIOC, GPIOC_SEL_2
#define ADRESS_PIN3         GPIOD, GPIOD_SEL_3

#endif /* HEADER_CONFIGURATION_H_ */
