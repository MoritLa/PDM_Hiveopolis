/*
 * definition.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_DEFINITION_H_
#define HEADER_DEFINITION_H_

#include "configuration.h"
//#include "com_osal.h"

typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef signed char int8;

typedef struct MyMessage_struct
{
    uint16 id:11 ;
    uint8 length:4;

    union{
        uint8 data8[8] ;
        uint16 data16[4] ;
        uint32 data32[2];
    } ;
} MyMessage;

#define CAN_REC_ERROR       0x7FF

typedef void (*ComMessageCb)(uint8 msgId, MyMessage msg);
void com_generic_message_cb(uint8 msg_id, MyMessage msg);


//CORE module info
#define CORE_CAN_ID     0x000

#ifdef CORE
#define MODULE_TYPE     0x0
#define MODULE_ID       0x00
#endif

//Message IDs
enum coreMsgID{
    HEARTBEAT=0x00,
	CORE_HEAD=0x01,
	CORE_CONT=0x02,
	CORE_BURST_ACCEPT=0x04,
	CORE_REG = 0x0E,
	CORE_HS = 0x0F
};

enum moduleMsgID{
//    Heartbeat=0x00,
	MOD_HEAD=0x10,
	MOD_CONT=0x11,
	MOD_BURST_REQ=0x012,
	MOD_BURST_CONT=0x013,
	MOD_FILE_HEAD=0x014,
	MOD_FILE_CONT=0x015,
	MOD_REG = 0x1E,
	MOD_HS=0x1F
};

//Message Formats
//core

//Module
#ifdef CORE
enum modHeadFormat{
    MOD_HEAD_TIME=2,
    MOD_HEAD_CONT=4,
    MOD_HEAD_LENGTH=5,
    MOD_HEAD_DLC = 6
};
enum modContFormat{
    MOD_CONT_DATA = 2
};
#else
enum modHeadFormat{
    MOD_HEAD_TIME=1,
    MOD_HEAD_CONT=3,
    MOD_HEAD_LENGTH=4,
    MOD_HEAD_DLC = 5
};
enum modContFormat{
    MOD_CONT_DATA = 1
};

#ifndef NB_MODULES
    NB_MODULES = 1;
#endif

#define CORE_BUFFER     NB_MODULES-1
#endif

#define BURST_BUFFER    NB_MODULES

enum InOutDef{
    IN=0,
    OUT=1
};
#define IN_OUT_SIZE     2

#define BUFFER_ERROR    (uint8)(-1)

#define NO_MAILBOX      0xFF

#endif /* HEADER_DEFINITION_H_ */
