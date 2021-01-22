/*
 * com_main.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_MAIN_H_
#define HEADER_COM_MAIN_H_

#include "com_osal.h"
#include "com_utilities.h"

// Initializes the communication protocol
//  (CAN ID, input and output loop, callback functions and OSAL)
void com_main_init(void);

// Closes open ports
void com_main_end(void);
#endif /* HEADER_COM_MAIN_H_ */
