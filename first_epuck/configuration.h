/*
 * configuration.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_CONFIGURATION_H_
#define HEADER_CONFIGURATION_H_

#define CHIBIOS

#define BURST_LENGTH	    1024
#define NB_MSG_P_S          32

#define INPUT_BUF_LENGTH    256
#define NB_MODULES          1

#define USE_SECOND_BUS	    true

#define CAN_BITRATE		    1000000

#define MODULE_TYPE         0x1

#endif /* HEADER_CONFIGURATION_H_ */
