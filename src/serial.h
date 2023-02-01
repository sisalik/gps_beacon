#ifndef SERIAL_H
#define SERIAL_H

#define MSG_SIZE 32

extern char serial_buffer[MSG_SIZE];

void serial_initialise();
int serial_await_message();

#endif  // SERIAL_H