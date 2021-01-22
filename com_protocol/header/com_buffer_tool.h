/*
 * com_buffer_tool.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_BUFFER_TOOL_H_
#define HEADER_COM_BUFFER_TOOL_H_

#include <stdlib.h>
#include "com_osal.h"
#include "com_utilities.h"

// Possible operation modes
enum lastOpsTypeDef {
    NO_OPS, MESSAGE_OPS, HEAD_OPS, DATA_OPS
};

// Sets the FIFO queue up, with a size of bufSize and a memory location (buffer)
//  The buffer is empty and blocked, the origin is 0x7FF, no burst pending.
void com_buffer_tools_init_queue(queue_t* queue, size_t bufSize, uint8* buffer);

// Resets the FIFO queue without changing size and memory location.
//  The buffer is empty and blocked, the origin is 0x7FF, no burst pending.
void com_buffer_tools_clear_buffer(queue_t* queue);

// Writes a message to the buffer (header and data)
//  Returns the number of data bytes written
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_buffer_tool_write_message(queue_t * queue, ComMessage message);

// Writes the header of a message to the buffer
//  Returns the number of data bytes expected
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_buffer_tool_write_header(queue_t * queue, ComMessage message);

// Writes the data of a message to the buffer
//  Returns the number of data bytes written, the number of data bytes are
//  indicated by the message.length field, it is possible to write the data
//  in more than one write (ex two writes of 8 bytes for a 16 byte messages). If
//  too much data is presented, only the data missing for the message is written.
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_buffer_tool_write_data(queue_t * queue, ComMessage message);

// Reads a message from the buffer, data is removed from the buffer
//  Returns the number of data bytes of the message read
//  Returns BUFFER_ERROR if an error occurs during the read access, the last
//  message stays in the buffer
uint8 com_buffer_tool_read_message(queue_t * queue, ComMessage* message);

// Returns the length of the data field of the next message in the buffer
uint8 com_buffer_tool_get_next_length(queue_t * queue);

// Reads the header of the next message in the buffer, data is removed
//  Returns the number of data bytes of the message
//  Returns BUFFER_ERROR if an error occurs during the read access, the last
//  message stays in the buffer
uint8 com_buffer_tool_read_header(queue_t * queue, ComMessage* message);

// Reads the data from the buffer, the maximal length of the data is given
//  by message->length. If less data is available, only this data is read.
//  Returns the number of bytes read
//  Returns BUFFER_ERROR if an error occurs during the read access, the content
//  stays in the buffer
uint8 com_buffer_tool_read_data(queue_t * queue, ComMessage* message);

// Returns the number of bytes left to READ or WRITE (direction) before the
//  next message is complete.
uint8 com_buffer_tool_get_left(queue_t* queue, uint8 direction);

// Indicates whether the buffer is empty
bool com_buffer_tool_empty(queue_t* queue);

// Indicates whether at least one message is left in the buffer
bool com_buffer_tool_msg_available(queue_t* queue);

// Indicates whether the message fits in the buffer
bool com_buffer_tool_msg_free(queue_t* queue, ComMessage message);

// Indicates whether the buffer is half full
bool com_buffer_tool_half_full(queue_t* queue);

// Undoes the last read operation to the buffer, this function can be called once
//  Returns false, when the last operation could not be undone (no space in buffer)
bool com_buffer_tool_undo_read(queue_t * queue);

// Undoes the last write operation to the buffer, this function can be called once
//  Returns false, when the last operation could not be undone (no message left)
bool com_buffer_tool_undo_write(queue_t * queue);

// Returns whether the buffer blocked flag
//  If it is set, nothing can be written to the buffer
uint8 com_buffer_tool_is_blocked(queue_t *queue);

// Sets the buffer blocked flag
//  If it is set, nothing can be written to the buffer
void com_buffer_tool_block_buffer(queue_t *queue);

// Clears the buffer blocked flag
//  If it is set, nothing can be written to the buffer
void com_buffer_tool_unblock_buffer(queue_t *queue);

// Sets the origin of the buffer
// Returns false if the buffer is either blocked, there is data left to write
//  or the buffer is not empty
uint8 com_buffer_tool_set_origin(queue_t *queue, uint16 origin);

// Returns the origin of the buffer
uint16 com_buffer_tool_get_origin(queue_t *queue);

// Sets the burst request flag
void com_buffer_tool_burst_requested(queue_t *queue);

// Clears the burst request flag
void com_buffer_tool_burst_terminated(queue_t *queue);

// Returns the burst request flag
bool com_buffer_tool_get_burst_request(queue_t *queue);

#endif /* HEADER_COM_BUFFER_TOOL_H_ */
