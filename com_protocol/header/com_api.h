/*
 * com_api.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_API_H_
#define HEADER_COM_API_H_

#include "configuration.h"
#include "com_osal.h"

bool com_init(void);
void com_end(void);

uint8_t send_data(uint8_t msg_id, uint16_t timestamp, uint8_t length, uint8_t* data);

#endif /* HEADER_COM_API_H_ */
