#include "main.h"
#include <../interpreter/scanner.h>
#include <../interpreter/interpreter.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include <display/7segment.h>
#include <uart/uart.h>

ISR(TIMER0_COMP_vect) {
    TCNT0 = 0;
    sevenSegmentInterruptHandler();
}

ISR(USART_RXC_vect) {
    uartRxInterruptHandler();
}

ISR(USART_TXC_vect) {
    uartTxInterruptHandler();
}


int main(void) {
    uartInit(UART_PARITY_EVEN, 0, getBaudDivider(19200, F_CPU), true, false);
    sevenSegmentInit(true);

    scannerInit(&consumeToken);

    interpreterInit(&uartSendChar, __malloc_heap_start);

    // Initialize timer for an interval of 4ms
    // --> seven segment numbers run at ~42Hz
    TCCR0 = 0b100;
    OCR0 = 249;
    TIMSK |= 2;
    sei();

    while(true) {
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();

        char testString[8];

        if (!uartEmptyBuffer()) {
            uint8_t rxBytes = uartGetRxData(testString, 8);
            processStr(testString, rxBytes);
        }
    }
}
