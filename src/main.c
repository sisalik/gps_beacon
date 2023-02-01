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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include "bluetooth.h"
#include "serial.h"
#include "utils.h"

LOG_MODULE_REGISTER(main);

/**
 * @brief Parse a command from the serial interface
 *
 * @param cmd Command string
 *
 * @retval 0 Command parsed successfully
 * @retval ERROR_MISSING_ARGUMENTS Command is missing arguments
 * @retval ERROR_INVALID_ARGUMENTS Command has invalid arguments
 * @retval ERROR_UNRECOGNISED_CMD Command is unrecognised
 */
int parse_cmd(char *cmd) {
    int err = 0;
    char *token;
    char *saveptr;

    // Split the command by spaces
    token = strtok_r(cmd, CMD_SEPARATOR, &saveptr);

    if (strcmp(token, "help") == 0) {
        printk("Available commands:\r\n");
        printk("  ad-start - Start advertising\r\n");
        printk("  ad-stop - Stop advertising\r\n");
        printk("  help - Print this help message\r\n");
        printk(
            "  pos-set lat lon alt - Set latitude, longitude and altitude\r\n");
    } else if (strcmp(token, "ad-start") == 0) {
        if (bt_advertising) {
            printk("OK: Advertising already started\r\n");
            return SUCCESS;
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
            return SUCCESS;
        }
        err = bt_advertise_stop();
        if (!err) {
            printk("OK: Advertising stopped\r\n");
            bt_advertising = false;
        } else {
            printk("ERROR: Failed to stop advertising (code %d)\r\n", err);
        }
    } else if (strcmp(token, "pos-set") == 0) {
        // Parse latitude
        token = strtok_r(NULL, CMD_SEPARATOR, &saveptr);
        if (token == NULL) {
            printk("Missing argument: latitude\r\n");
            return ERROR_MISSING_ARGUMENTS;
        }
        long lat = coord_to_fixed_point(token);
        // Check if latitude is valid and between -90 and 90 degrees
        if (lat == ERROR_INVALID_COORDINATE || lat > 900000000 ||
            lat < -900000000) {
            printk("Invalid argument: latitude\r\n");
            return ERROR_INVALID_ARGUMENTS;
        }
        // Parse longitude
        token = strtok_r(NULL, CMD_SEPARATOR, &saveptr);
        if (token == NULL) {
            printk("Missing argument: longitude\r\n");
            return ERROR_MISSING_ARGUMENTS;
        }
        long lon = coord_to_fixed_point(token);
        if (lon == ERROR_INVALID_COORDINATE || lon > 1800000000 ||
            lon < -1800000000) {
            printk("Invalid argument: longitude\r\n");
            return ERROR_INVALID_ARGUMENTS;
        }
        // Parse altitude
        token = strtok_r(NULL, CMD_SEPARATOR, &saveptr);
        if (token == NULL) {
            printk("Missing argument: altitude\r\n");
            return ERROR_MISSING_ARGUMENTS;
        }
        int alt = atoi(token);

        bt_set_payload_data(lat, lon, alt);
        // Only update advertising if it is already started
        if (bt_advertising) {
            bt_advertise_start_or_update();
        }
        printk("OK: Set position to %ld, %ld, %d\r\n", lat, lon, alt);
    } else {
        printk("Unknown command: %s\r\n", token);
        return ERROR_UNRECOGNISED_CMD;
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
