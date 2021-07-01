#ifndef __MAIN_H
#define __MAIN_H

#include <avr/io.h>

#define CHANNEL 119

#ifdef __AVR_ATtiny2313__
    #define F_CPU 8000000UL
    #define NRF_CE_DDR  DDRB
    #define NRF_CE_PORT PORTB
    #define NRF_CE_PIN  PB3

    #define NRF_CS_DDR  DDRB
    #define NRF_CS_PORT PORTB
    #define NRF_CS_PIN  PB4

    #define NRF_IRQ_DDR  DDRB
    #define NRF_IRQ_PORT PINB
    #define NRF_IRQ_PIN  PB2

    // 8-bit interface if 1, 4-bit if 0
    #define LCD_8BIT 0

    #define LCD_DATA        PORTD
    #define LCD_DATA_DDR    DDRD

    // Only used if LCD_8BIT is 0
    #define LCD_DATA_OFFSET 2
    #define LCD_DATA_MASK (~(0xF<<LCD_DATA_OFFSET))

    #define LCD_CURSOR (0)
    #define LCD_CURSOR_BLINK (0)

    #define LCD_RS_PORT (PORTB)
    #define LCD_RS_DDR  (DDRB)
    #define LCD_RS      (PB1)

    #define LCD_RW_PORT (PORTD)
    #define LCD_RW_DDR  (DDRD)
    #define LCD_RW      (PD0)

    #define LCD_E_PORT  (PORTD)
    #define LCD_E_DDR   (DDRD)
    #define LCD_E       (PD1)

    #define LCD_OUTPUT 0
#endif

#ifdef __AVR_ATmega16__
    #define F_CPU 16000000UL
    #define NRF_CE_DDR  DDRB
    #define NRF_CE_PORT PORTB
    #define NRF_CE_PIN  PB3

    #define NRF_CS_DDR  DDRB
    #define NRF_CS_PORT PORTB
    #define NRF_CS_PIN  PB4

    #define NRF_IRQ_DDR  DDRB
    #define NRF_IRQ_PORT PINB
    #define NRF_IRQ_PIN  PB1

    // 8-bit interface if 1, 4-bit if 0
    #define LCD_8BIT 0

    #define LCD_DATA        PORTC
    #define LCD_DATA_DDR    DDRC

    // Only used if LCD_8BIT is 0
    #define LCD_DATA_OFFSET 0
    #define LCD_DATA_MASK (~(0xF<<LCD_DATA_OFFSET))

    #define LCD_CURSOR (0)
    #define LCD_CURSOR_BLINK (0)

    #define LCD_RS_PORT (PORTC)
    #define LCD_RS_DDR  (DDRC)
    #define LCD_RS      (PC4)

    #define LCD_RW_PORT (PORTC)
    #define LCD_RW_DDR  (DDRC)
    #define LCD_RW      (PC5)

    #define LCD_E_PORT  (PORTC)
    #define LCD_E_DDR   (DDRC)
    #define LCD_E       (PC6)
    #define LCD_OUTPUT 1


    #define SEGMENT_DIGITS 6
    #define SEGMENT_SELECTOR_PORT PORTD
    #define SEGMENT_SELECTOR_DDR DDRD
    #define SEGMENT_SELECTOR_OFFSET 2
    #define SEGMENT_LINES_PORT PORTB
    #define SEGMENT_LINES_DDR DDRB
    #define SEGMENT_RUNNING_DOTS 1
#endif
#endif
