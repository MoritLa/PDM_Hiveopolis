/*
 * com_output_buffer.h
 *
 *  Created on: 10.12.2020
 *      Author: morit
 */

#ifndef HEADER_COM_OUTPUT_BUFFER_H_
#define HEADER_COM_OUTPUT_BUFFER_H_

#include <stdlib.h>

#include "com_osal.h"
#include "com_utilities.h"

// Sets up the different input buffers.
//  NB_MODULE buffers of IN_BUF_SIZE and one of 2*BURST_LENGTH
//  The buffers are empty and blocked, the origin is 0x7FF, no burst pending.
void com_output_buffer_init(void);

// Resets the FIFO queue without changing size and memory location.
//  The buffer is empty and blocked, the origin is 0x7FF, no burst pending.
void com_output_buffer_clear_buffer(void);

// Writes a message to the buffer (header and data)
//  Returns the number of data bytes written
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_output_buffer_write_message(ComMessage message);

// Writes the header of a message to the buffer
//  Returns the number of data bytes expected
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_output_buffer_write_header(ComMessage message);

// Writes the data of a message to the buffer
//  Returns the number of data bytes written, the number of data bytes are
//  indicated by the message.length field, it is possible to write the data
//  in more than one write (ex two writes of 8 bytes for a 16 byte messages). If
//  too much data is presented, only the data missing for the message is written.
//  Returns BUFFER_ERROR if an error during the write occurs. Nothing is written
//  in case of an error
uint8 com_output_buffer_write_data(ComMessage message);

// Reads a message from the buffer, data is removed
//  Returns the number of data bytes of the message read
//  Returns BUFFER_ERROR if an error occurs during the read access, the last
//  message stays in the buffer
uint8 com_output_buffer_read_message(ComMessage* message);

// Returns the length of the data field of the next message in the buffer
uint8 com_output_buffer_get_next_length(void);

// Reads the header of the next message in the buffer, data is removed
//  Returns the number of data bytes of the message
//  Returns BUFFER_ERROR if an error occurs during the read access, the last
//  message stays in the buffer
uint8 com_output_buffer_read_header(ComMessage* message);

// Reads the data from the buffer, the maximal length of the data is given
//  by message->length. If less data is available, only this data is read.
//  Returns the number of bytes read
//  Returns BUFFER_ERROR if an error occurs during the read access, the content
//  stays in the buffer
uint8 com_output_buffer_read_data(ComMessage* message);

// Returns the number of bytes left to write before the next message is complete.
uint8 com_output_buffer_get_left_write(void);

// Returns the number of bytes left to read before the next message is complete.
uint8 com_output_buffer_get_left_read(void);

// Indicates whether the buffer is empty
bool com_output_buffer_empty(void);

// Indicates whether at least one message is left in the buffer
bool com_output_buffer_msg_available(void);

// Indicates whether the message fits in the buffer
bool com_output_buffer_msg_free(ComMessage message);

// Indicates whether the buffer is half full
bool com_output_buffer_half_full(void);

// Undoes the last write operation to the buffer, this function can be called once
//  Returns false, when the last operation could not be undone (no message left)
bool com_output_buffer_undo_write(void);

// Undoes the last read operation to the buffer, this function can be called once
//  Returns false, when the last operation could not be undone (no space in buffer)
bool com_output_buffer_undo_read(void);

// Returns whether the buffer blocked flag
//  If it is set, nothing can be written to the buffer
uint8 com_output_buffer_is_blocked(void);

// Sets the buffer blocked flag
//  If it is set, nothing can be written to the buffer
void com_output_buffer_block_buffer(void);

// Clears the buffer blocked flag
//  If it is set, nothing can be written to the buffer
void com_output_buffer_unblock_buffer(void);

// Sets the burst request flag
void com_output_buffer_burst_requested(void);

// Clears the burst request flag
void com_output_buffer_burst_terminated(void);

// Returns the burst request flag
bool com_output_buffer_get_burst_request(void);

#endif /* HEADER_COM_OUTPUT_BUFFER_H_ */
