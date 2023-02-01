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
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include "bluetooth.h"

LOG_MODULE_REGISTER(main);

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define MSG_SIZE 32

// Queue to store up to 10 messages (aligned to 4-byte boundary)
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Receive buffer used in UART ISR callback
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

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

/**
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data) {
    uint8_t c;

    if (!uart_irq_update(uart_dev)) {
        return;
    }

    while (uart_irq_rx_ready(uart_dev)) {
        uart_fifo_read(uart_dev, &c, 1);

        if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
            // Terminate string
            rx_buf[rx_buf_pos] = '\0';

            // If queue is full, message is silently dropped
            k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

            // Reset the buffer (it was copied to the msgq)
            rx_buf_pos = 0;
        } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
            rx_buf[rx_buf_pos++] = c;
        }
    }
}

void main() {
    int err;
    char tx_buf[MSG_SIZE];

    // Configure interrupt and callback to receive serial data
    uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    // Initialize the Bluetooth Subsystem
    err = bt_initialise();
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Boot complete\r\n");
    // Indefinitely wait for input from the user
    while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
        parse_cmd(tx_buf);
    }
}
