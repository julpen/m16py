#include <stdio.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "7segment.h"


#if SEGMENT_RUNNING_DOTS == 1
    volatile uint8_t dots=1;
    volatile uint8_t dotPrescaler;
#endif

volatile uint8_t sevenSegmentCurrentDigit = 0;
volatile uint8_t sevenSegmentContent[SEGMENT_DIGITS];

const uint8_t sevenSegmentHexDigitMapping[16] PROGMEM = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,
    0b01110111,
    0b01111100,
    0b00111001,
    0b01011110,
    0b01111001,
    0b01110001,
};

void sevenSegmentInit() {
    SEGMENT_LINES_DDR = 0xFF;
    SEGMENT_SELECTOR_DDR |= SEGMENT_MASK;
}

inline void sevenSegmentInterruptHandler() {
    sevenSegmentCurrentDigit = (sevenSegmentCurrentDigit+1) % SEGMENT_DIGITS;

    SEGMENT_SELECTOR_PORT = (SEGMENT_SELECTOR_PORT & ~(SEGMENT_MASK)) | ((1<< sevenSegmentCurrentDigit) << SEGMENT_SELECTOR_OFFSET);
    if (!SEGMENT_RUNNING_DOTS) {
        SEGMENT_LINES_PORT = sevenSegmentContent[sevenSegmentCurrentDigit];
    }
    else {
        dotPrescaler++;
        if (dotPrescaler == 0) {
            dots = dots << 1;
            if (dots >= (1<<SEGMENT_DIGITS)) {
                dots = 1;
            }
        }
        SEGMENT_LINES_PORT = sevenSegmentContent[sevenSegmentCurrentDigit] | (((dots&(1<<sevenSegmentCurrentDigit)) > 0)<<7);
    }

}

void sevenSegmentPrintDecimal(uint32_t num) {
    sevenSegmentContent[0] = pgm_read_byte_near(sevenSegmentHexDigitMapping +(num % 10));
    for(uint8_t i=1; i < SEGMENT_DIGITS; i++) {
        num = num/10;
        if (num == 0) {
            sevenSegmentContent[i] = 0;
        }
        else {
            sevenSegmentContent[i] = pgm_read_byte_near(sevenSegmentHexDigitMapping + (num % 10));
        }
    }
}

void sevenSegmentPrintHex(uint32_t num) {
    sevenSegmentContent[0] = pgm_read_byte_near(sevenSegmentHexDigitMapping +(num&0xF));
    for(uint8_t i=1; i < SEGMENT_DIGITS; i++) {
        num = num >> 4;
        if (num == 0) {
            sevenSegmentContent[i] = 0;
        }
        else {
            sevenSegmentContent[i] = pgm_read_byte_near(sevenSegmentHexDigitMapping + (num&0xF));
        }
    }
}

void sevenSegmentPrintBinary(uint32_t num) {
    sevenSegmentContent[0] = pgm_read_byte_near(sevenSegmentHexDigitMapping +(num&1));
    for(uint8_t i=1; i < SEGMENT_DIGITS; i++) {
        num = num >> 1;
        if (num == 0) {
            sevenSegmentContent[i] = 0;
        }
        else {
            sevenSegmentContent[i] = pgm_read_byte_near(sevenSegmentHexDigitMapping + (num&1));
        }
    }
}

