#include "serial.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

// Queue to store up to 10 messages (aligned to 4-byte boundary)
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

// Get UART device from device tree
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Receive buffer used in UART ISR callback
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

char serial_buffer[MSG_SIZE];

// Function declarations
void serial_cb(const struct device *dev, void *user_data);

// Public functions

void serial_initialise() {
    // Configure interrupt and callback to receive serial data
    uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
    uart_irq_rx_enable(uart_dev);
}

int serial_await_message() {
    return k_msgq_get(&uart_msgq, &serial_buffer, K_FOREVER);
}

// Private functions

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
