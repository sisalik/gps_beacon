#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdbool.h>

extern bool bt_advertising;

int bt_initialise();
int bt_advertise_start_or_update();
int bt_advertise_stop();
void bt_set_payload_data(int x, int y);

#endif  // BLUETOOTH_H