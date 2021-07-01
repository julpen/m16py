#include "uart.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define TX_FINISHED (1<<0)
#define TX_NEWLINE  (1<<1)
#define RX_OVERFLOW (1<<2)
#define RX_FULL     (1<<3)
#define RX_EMPTY    (1<<4)
#define TX_INTERRUPTS (1<<5)

volatile uint8_t uartStatus = RX_EMPTY | TX_FINISHED;

char *txPointer;
uint8_t   txRemaining;

// Store received bytes as ring buffer
char rxBuffer[UART_RX_BUF_SIZE];
uint8_t rxWrPos = 0;
uint8_t rxRdPos = 0;


#ifdef __AVR_ATmega16__


    uint16_t getBaudDivider(uint32_t baudrate, uint32_t clkFreq) {
        clkFreq = clkFreq/16;
        uint16_t divider = clkFreq/baudrate - 1;

        if (clkFreq/(divider+1) < baudrate) {
            return divider;
        } else {
            uint32_t current = clkFreq/(divider+1); // Bigger than the actual baudrate
            uint32_t next = clkFreq/(divider + 2); // Smaller than the actual baudrate

            if ((baudrate - next) <  (current - baudrate)) {
                return divider+1;
            }
            return divider;
        }
    }

    void uartInit(uint8_t parity, bool twoStopBits, uint16_t baudDivider, bool rxInterrupts, bool txInterrupts) {
        if (baudDivider > 0xFF) {
            UBRRH = (baudDivider >> 8) & 0xF;
        }
        UBRRL = baudDivider & 0xFF;

        if (txInterrupts) {
            uartStatus |= TX_INTERRUPTS;
        }
        else {
            uartStatus &= ~TX_INTERRUPTS;
        }

        UCSRB = (rxInterrupts << RXCIE) | (txInterrupts << TXCIE) | (1 << RXEN) | (1 << TXEN);
        uint8_t ucsrc = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0) | (twoStopBits << USBS);

        if (parity == UART_PARITY_EVEN) {
            UCSRC = ucsrc | (1 << UPM1);
        }
        if (parity == UART_PARITY_ODD) {
            UCSRC = ucsrc | (1 << UPM1) | (1<< UPM0);
        }
    }

    void uartTxInterruptHandler() {
        if (txRemaining) {
            UDR = txPointer[0];
            txPointer++;
            txRemaining--;
            return;
        }
        if (uartStatus & TX_NEWLINE) {
            UDR = '\n';
            uartStatus &= ~TX_NEWLINE;
            return;
        }
        uartStatus|=TX_FINISHED;
    }

    void uartRxInterruptHandler() {
        if (uartFullBuffer()) {
            uartStatus |= RX_OVERFLOW;
            return;
        }


        rxBuffer[rxWrPos % UART_RX_BUF_SIZE] =  UDR;
        rxWrPos = (rxWrPos + 1) % UART_RX_BUF_SIZE;
        uartStatus &= ~RX_EMPTY;

        if (rxWrPos == rxRdPos) {
            uartStatus |= RX_FULL;
        }
    }

    static uint8_t uartRxLen() {
        if (uartEmptyBuffer()) {
            return 0;
        }
        if (rxRdPos < rxWrPos) { // No wrap
            return rxWrPos - rxRdPos;
        }
        else { // Wrapping around
            return UART_RX_BUF_SIZE + rxWrPos - rxRdPos;
        }
    }

    uint8_t uartGetRxData(char *target, uint8_t len) {
        len = MIN(len, uartRxLen());
        if ((rxRdPos + len) <= UART_RX_BUF_SIZE) { // Ringbuffer is not wrapping
            memmove(target, rxBuffer + rxRdPos, len);
        }
        else { // Two separate copies are required
            memmove(target, rxBuffer + rxRdPos, UART_RX_BUF_SIZE-rxRdPos);
            memmove(target+UART_RX_BUF_SIZE-rxRdPos, rxBuffer, len+rxRdPos-UART_RX_BUF_SIZE);
        }
        rxRdPos = (rxRdPos + len) % UART_RX_BUF_SIZE;

        if (len > 0) {
            uartStatus &= ~RX_FULL;
            if (rxRdPos == rxWrPos) {
                uartStatus |= RX_EMPTY;
            }
        }
        return len;
    }

    void uartSendLine(char *line) {
        if (uartStatus & TX_INTERRUPTS) {
            UDR = line[0];
            txPointer = line+1;
            txRemaining = strlen(line)-1;
            uartStatus |=  TX_NEWLINE;
            uartStatus &= ~TX_FINISHED;
        }
        else {
            while (line[0] != 0) {
                while (!(UCSRA & (1<<UDRE)));
                UDR = line[0];
                line++;
            }
            while (!(UCSRA & (1<<UDRE)));
            UDR = '\n';
        }
    }

    void uartSendChar(char c) {
        while (!(UCSRA & (1<<UDRE)));
        UDR = c;
    }

    void uartSendStr(char *str, uint8_t len) {
        if (len == 0) {
            return;
        }
        if (uartStatus & TX_INTERRUPTS) {
            UDR = str[0];
            txPointer = str+1;
            txRemaining = len-1;
            uartStatus &= ~(TX_NEWLINE|TX_FINISHED);
        }
        else {
            for (uint8_t i=0; i < len; i++) {
                while (!(UCSRA & (1<<UDRE)));
                UDR = str[i];
            }
        }
    }

    bool uartEmptyBuffer() {
        return uartStatus & RX_EMPTY;
    }

    bool uartFullBuffer() {
        return uartStatus & RX_FULL;
    }

    bool uartOverflow() {
        return uartStatus & RX_OVERFLOW;
    }

    char* uartGetRxLine() {
        return rxBuffer;
    }

    bool uartTxFinished() {
        return uartStatus & TX_FINISHED;
    }

#endif