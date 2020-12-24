/*
 * com_api.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_API_H_
#define HEADER_COM_API_H_

#include "configuration.h"
#include "definition.h"
#include "com_osal.h"

bool com_init(void);
void com_end(void);

uint8_t com_send_data(ComMessage message);

#endif /* HEADER_COM_API_H_ */
