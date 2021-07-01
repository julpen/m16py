#ifndef __UART_H
#define __UART_H
#include <stdint.h>
#include <stdbool.h>

#ifndef UART_RX_BUF_SIZE
#define UART_RX_BUF_SIZE 8
#endif

enum {
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
    UART_PARITY_NONE
};


void uartInit(uint8_t parity, bool twoStopBits, uint16_t baudDivider, bool rxInterrupts, bool txInterrupts);

void uartTxInterruptHandler();

void uartRxInterruptHandler();

uint16_t getBaudDivider(uint32_t baudrate, uint32_t clkFreq);

void uartSendLine(char *line);

void uartSendStr(char *line, uint8_t len);

void uartSendChar(char c);

bool uartRxFinished();

bool uartEmptyBuffer();

bool uartFullBuffer();

bool uartOverflow();

bool uartTxFinished();

char* uartGetRxLine();

uint8_t uartGetRxData(char *target, uint8_t len);

#endif