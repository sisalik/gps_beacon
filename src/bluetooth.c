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

LOG_MODULE_REGISTER(bluetooth);

#define DEVICE_NAME "GPS Beacon"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// Bluetooth advertising data and flags
static int bt_payload_x = 0;
static int bt_payload_y = 0;
static bool bt_ad_initialized = false;
bool bt_advertising = false;

// Set Scan Response data (constant)
static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)};

// Function declarations
static void bt_ready(int err);

// Public functions

int bt_initialise() { return bt_enable(bt_ready); }

void bt_set_payload_data(int x, int y) {
    bt_payload_x = x;
    bt_payload_y = y;
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
        BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA, 0x0F, 0x02,  // Comodule ID
                      bt_payload_x, bt_payload_y)};

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
    // Test code
    // bt_set_payload_data(0, 0);
    // bt_advertise();
}
