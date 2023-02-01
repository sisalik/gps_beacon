/**
 * @file bluetooth.h
 * @author Siim Lepik (siim.lepik@gmail.com)
 * @brief Bluetooth interface header
 * @version 0.1
 * @date 2023-02-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef GPS_BEACON_SRC_BLUETOOTH_H_
#define GPS_BEACON_SRC_BLUETOOTH_H_

#include <stdbool.h>

/**
 * @brief Flag indicating whether advertising is currently active
 */
extern bool bt_advertising;

/**
 * @brief Initialise the Bluetooth interface
 *
 * @return Error code returned by bt_enable()
 */
int bt_initialise();

/**
 * @brief Start or update Bluetooth advertising
 *
 * If advertising is already active, this function will update the advertising
 * payload with the current position data.
 *
 * If advertising is not active, this function will start advertising.
 *
 * @return Error code returned by bt_le_adv_start() or bt_le_adv_update_data()
 */
int bt_advertise_start_or_update();

/**
 * @brief Stop Bluetooth advertising
 *
 * @return Error code returned by bt_le_adv_stop()
 */
int bt_advertise_stop();

/**
 * @brief Set the payload data to be sent in the Bluetooth advertising packet
 *
 * @param latitude Latitude in degrees * 1e7. Positive values are north,
 *  negative values are south.
 * @param longitude Longitude in degrees * 1e7. Positive values are east,
 *  negative values are west.
 * @param altitude Altitude in meters
 */
void bt_set_payload_data(long latitude, long longitude, int altitude);

#endif  // GPS_BEACON_SRC_BLUETOOTH_H_
