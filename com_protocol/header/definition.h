/*
 * definition.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_DEFINITION_H_
#define HEADER_DEFINITION_H_

//Message IDs
enum coreMsgID{
	CORE_HB=0x00,
	CORE_HEAD=0x01,
	CORE_CONTENT=0x02,
	CORE_BURST_ACCEPT=0x04,
	CORE_REG = 0x0E,
	CORE_HS = 0x0F
};

enum moduleMsgID{
	MOD_HB=0x00,
	MOD_HEAD=0x10,
	MOD_CONT=0x11,
	MOD_BURST_REQ=0x012,
	MOD_BURST_CONT=0x013,
	MOD_FILE_HEAD=0x014,
	MOD_FILE_CONT=0x015,
	MOD_REG = 0x1E,
	MOD_HS=0x0F
};

//Message Formats
//core

//Module
enum modHeadFormat{
    MOD_HEAD_TIME=1,
    MOD_HEAD_CONT=3,
    MOD_HEAD_LENGTH=4,
    MOD_HEAD_DLC = 5
};
enum modContFormat{
    MOD_CONT_DATA = 1
};
#endif /* HEADER_DEFINITION_H_ */
