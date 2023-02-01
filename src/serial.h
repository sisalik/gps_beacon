/**
 * @file serial.h
 * @author Siim Lepik (siim.lepik@gmail.com)
 * @brief Serial interface header
 * @version 0.1
 * @date 2023-02-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef GPS_BEACON_SRC_SERIAL_H_
#define GPS_BEACON_SRC_SERIAL_H_

#define MSG_SIZE 32

extern char serial_buffer[MSG_SIZE];

/**
 * @brief Initialise the serial interface
 */
void serial_initialise();

/**
 * @brief Wait for a message to be received on the serial interface
 *
 * @retval 0 Message received.
 * @retval -ENOMSG Returned without waiting.
 * @retval -EAGAIN Waiting period timed out.
 */
int serial_await_message();

#endif  // GPS_BEACON_SRC_SERIAL_H_
