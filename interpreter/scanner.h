#ifndef SCANNER_H
#define SCANNER_H
#include <stdint.h>

#ifndef PROGRAM
    #include <avr/pgmspace.h>
    #include <display/7segment.h>
#else // We are compiling for the test program and not the microcontroller
    #include <stdio.h>
    #define PROGMEM
    #define pgm_read_word(a) *(a)
    #define pgm_read_byte_near(a) *(a)
    #define strlen_P strlen
    #define strstr_P strstr
    #define sevenSegmentPrintHex(a) printf("CODE: 0x%x\n", a)
#endif

#define SCAN_BUFFER_SIZE 16

// The first keywords need to directly map to the string keywords defined in scanner.c
typedef enum {
    ID_WHILE,
    ID_IF,
    ID_DEF,
    ID_YEET,
    ID_TRUE,
    ID_FALSE,
    ID_RETURN,
    ID_PUT,
    ID_SET,
    ID_GET,
    ID_AND,
    ID_OR,
    ID_ELSE,
    ID_NOT,
    ID_DEL,
    ID_DEBUG,
    ID_BREAK,
    ID_PRINT,
    ID_PRINTN,
    ID_COLON,
    ID_GT,
    ID_LT,
    ID_EXP,
    ID_ASSIGN,
    ID_SEMICOLON,
    ID_COMMA,
    ID_OPEN_ROUND,
    ID_CLOSE_ROUND,
    ID_OPEN_SQUARE,
    ID_CLOSE_SQUARE,
    ID_SHIFT_RIGHT,
    ID_SHIFT_LEFT,
    ID_LE,
    ID_GE,
    ID_EQU,
    ID_NEQ,
    ID_NUM,
    ID_NAME,
    ID_PLUS,
    ID_MINUS,
    ID_MULT,
    ID_DIV,
    ID_MOD,
    ID_EOL,
} keyword;

typedef struct {
    keyword token;
    uint32_t tokenValue;
    uint8_t indent;
} parsedToken;

void processStr(char *str, uint8_t len);

void scannerInit(void *consumer);

#endif