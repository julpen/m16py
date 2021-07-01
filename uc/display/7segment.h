#ifndef __LCD_H
#define __LCD_H

#include <stdbool.h>
#include <stdio.h>
#include <main.h>
#include <avr/pgmspace.h>

#define SEGMENT_MASK (((1<<SEGMENT_DIGITS)-1)<<2)

#ifndef SEGMENT_RUNNING_DOTS
    #define SEGMENT_RUNNING_DOTS 0
#endif

void sevenSegmentInit();

void sevenSegmentInterruptHandler();

void sevenSegmentPrintDecimal(uint32_t num);

void sevenSegmentPrintHex(uint32_t num);

void sevenSegmentPrintBinary(uint32_t num);

#endif
