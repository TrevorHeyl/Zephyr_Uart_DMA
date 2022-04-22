/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <string.h>


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   500

// Different ways to access node identifiers
/* The devicetree node identifier for the "green_led_6" label */
#define LED0_NODE DT_NODELABEL(green_led_6)
/* The devicetree node identifier for the "led0" alias. */
#define LED1_NODE DT_ALIAS(led1)
#define UART_NODE2 DT_NODELABEL(usart2)





/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
const struct device *uart2 = DEVICE_DT_GET(UART_NODE2);


#define MSG_SIZE 32  // NB message size must be modulo 2

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);


static char rx_buf[MSG_SIZE];
static char tx_buf[MSG_SIZE];
static int rx_buf_pos;

/*
 * init_failed
 * If a device init failed, we come here
 */
void init_failed(void)
{
     while(1);
}


/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
        uint8_t c;

        gpio_pin_toggle_dt(&led0);

        if (!uart_irq_update(uart2)) {
                return;
        }


        while (uart_irq_rx_ready(uart2)) {

                uart_fifo_read(uart2, &c, 1);

                if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
                        /* terminate string */
                        rx_buf[rx_buf_pos] = '\0';

                        /* if queue is full, message is silently dropped */
                        k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

                        /* reset the buffer (it was copied to the msgq) */
                        rx_buf_pos = 0;
                } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
                        rx_buf[rx_buf_pos++] = c;
                }
                /* else: characters beyond buffer size are dropped */
        }
}



/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
        int msg_len = strlen(buf);

        for (int i = 0; i < msg_len; i++) {
                uart_poll_out(uart2, buf[i]);
        }
}


void main(void)
{
        int ret;
        volatile int var = 0;	


        /* Check if all devices initalised properly */
	if (!device_is_ready(led0.port)) {
		init_failed();
	}
	
	var++;

	if (!device_is_ready(led1.port)) {
		init_failed();
	}

	var+=10;

	if (!device_is_ready(uart2)  ) {
		//printk("uart device not ready\n");
		init_failed();
	}

	var = 100;
	
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		init_failed();
	}
	
	var -= 10;

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		init_failed();
	}



        /* configure interrupt and callback to receive data */
        uart_irq_callback_user_data_set(uart2, serial_cb, NULL);
        uart_irq_rx_enable(uart2);

        print_uart("Hello! I'm your echo bot.\r\n");
        print_uart("Tell me something and press enter:\r\n");


        /* indefinitely wait for input from the user */
        while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
                print_uart("Echo: ");
                print_uart(tx_buf);
                print_uart("\r\n");
                gpio_pin_toggle_dt(&led1);
        }

}
