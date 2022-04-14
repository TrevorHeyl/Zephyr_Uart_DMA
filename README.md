# Zephyr Uart DMA

* Showing how to use the UART DMA API. 
* This application must use a standard board file eg west build -p -b stm32f3_disco 
* The application implements a UART line echo
* The board must support an led0 and led 1 node at least
* The board must support one UART

## What does it do?
This demo uses a UART to echo back what has been sent from an external terminal. The UART receive waits for a CR or LF then echos back what was received. An LED is toggled on each RX byte received and another LED on each message echoed back

## What config is needed
* See proj.conf, CONFIG_UART_INTERRUPT_DRIVEN=y
