/**
 * @file bluetooth.c
 * @author Siim Lepik (siim.lepik@gmail.com)
 * @brief Bluetooth interface implementation
 * @version 0.1
 * @date 2023-02-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "bluetooth.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>

#include "utils.h"

LOG_MODULE_REGISTER(bluetooth);

#define DEVICE_NAME "GPS Beacon"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// Bluetooth advertising data and flags
struct PositionData {
    // 4 bytes for latitude, in units of 1/10000000 of a degree.
    // Positive values are north of the equator, negative values are south.
    // The range of values is, therefore, from -900000000 to 900000000.
    // This results in a worst-case precision of 11.1 mm (at the equator).
    // See: https://en.wikipedia.org/wiki/Decimal_degrees
    // This is clearly overkill for a GPS beacon, but it maximises the
    // utilisation of the 4 bytes available in the advertising packet.
    long latitude;
    // 4 bytes for longitude, in units of 1/10000000 of a degree as above.
    // Positive values are east of the prime meridian, negative values are west.
    long longitude;
    // 2 bytes for altitude, in meters. This results in a range of -32768 to
    // 32767 meters, with a precision of 1 meter.
    int altitude;
};
struct PositionData pos_data;
static bool bt_ad_initialized = false;
bool bt_advertising = false;

// Set Scan Response data (constant)
static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)};

// Function declarations
static void bt_ready(int err);

// Public functions

int bt_initialise() { return bt_enable(bt_ready); }

void bt_set_payload_data(long latitude, long longitude, int altitude) {
    pos_data.latitude = latitude;
    pos_data.longitude = longitude;
    pos_data.altitude = altitude;
    bt_ad_initialized = true;
}

int bt_advertise_start_or_update() {
    int err;
    char addr_s[BT_ADDR_LE_STR_LEN];
    bt_addr_le_t addr = {0};
    size_t count = 1;

    if (!bt_ad_initialized) {
        LOG_ERR("Advertising data not initialized");
        return -1;
    }

    // Create advertising data packet
    struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
        BT_DATA_BYTES(
            BT_DATA_MANUFACTURER_DATA, 0x0F, 0x02,  // Comodule GMBH company ID
            // Split the 4-byte latitude and longitude into 4 1-byte values
            // and put them in the advertising packet in big-endian order
            GET_BYTE(3, pos_data.latitude), GET_BYTE(2, pos_data.latitude),
            GET_BYTE(1, pos_data.latitude), GET_BYTE(0, pos_data.latitude),
            GET_BYTE(3, pos_data.longitude), GET_BYTE(2, pos_data.longitude),
            GET_BYTE(1, pos_data.longitude), GET_BYTE(0, pos_data.longitude),
            // Same for the 2-byte altitude value
            GET_BYTE(1, pos_data.altitude), GET_BYTE(0, pos_data.altitude))};

    if (!bt_advertising) {
        err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad), sd,
                              ARRAY_SIZE(sd));
    } else {
        err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    }

    if (err) {
        LOG_ERR("Advertising failed to %s (err %d)\r\n",
                bt_advertising ? "update" : "start", err);
        return err;
    }
    bt_advertising = true;

    // Get the device address for logging
    bt_id_get(&addr, &count);
    bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
    LOG_INF("Beacon started, advertising as %s", addr_s);
    return 0;
}

int bt_advertise_stop() { return bt_le_adv_stop(); }

// Private functions

static void bt_ready(int err) {
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }
    LOG_INF("Bluetooth initialized");
}
