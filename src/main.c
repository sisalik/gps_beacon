/**
 * @file main.c
 * @author Siim Lepik (siim.lepik@gmail.com)
 * @brief Application main entry point
 * @version 0.1
 * @date 2023-01-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include "bluetooth.h"
#include "serial.h"

LOG_MODULE_REGISTER(main);

int parse_cmd(char *cmd) {
    int err = 0;
    char *token;
    char *saveptr;

    token = strtok_r(cmd, " ", &saveptr);
    if (token == NULL) {
        return -1;
    }

    if (strcmp(token, "help") == 0) {
        printk("Available commands:\r\n");
        printk("  ad-start - Start advertising\r\n");
        printk("  ad-stop - Stop advertising\r\n");
        printk("  help - Print this help message\r\n");
        printk("  pos-set x y - Set position to x and y\r\n");
    } else if (strcmp(token, "ad-start") == 0) {
        if (bt_advertising) {
            printk("OK: Advertising already started\r\n");
            return 0;
        }
        err = bt_advertise_start_or_update();
        if (!err) {
            printk("OK: Advertising started\r\n");
        } else {
            printk("ERROR: Failed to start advertising (code %d)\r\n", err);
        }
    } else if (strcmp(token, "ad-stop") == 0) {
        if (!bt_advertising) {
            printk("OK: Advertising already stopped\r\n");
            return 0;
        }
        err = bt_advertise_stop();
        if (!err) {
            printk("OK: Advertising stopped\r\n");
            bt_advertising = false;
        } else {
            printk("ERROR: Failed to stop advertising (code %d)\r\n", err);
        }
    } else if (strcmp(token, "pos-set") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            printk("Missing argument: x\r\n");
            return -EINVAL;
        }
        int x = atoi(token);
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            printk("Missing argument: y\r\n");
            return -EINVAL;
        }
        int y = atoi(token);

        bt_set_payload_data(x, y);
        // Only update advertising if it is already started
        if (bt_advertising) {
            bt_advertise_start_or_update();
        }
        printk("OK: Set position to x=%d, y=%d\r\n", x, y);
    } else {
        printk("Unknown command: %s\r\n", token);
        return -EINVAL;
    }
    return err;
}

void main() {
    int err;
    // Initialize the serial (UART) device and the Bluetooth subsystem
    serial_initialise();
    err = bt_initialise();
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Boot complete\r\n");
    // Indefinitely wait for input from the user
    while (serial_await_message() == 0) {
        parse_cmd(serial_buffer);
    }
}
