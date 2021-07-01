#include "scanner.h"
#include <stdbool.h>
#include <string.h>
#include <crc/crc.h>

#define START_OF_LINE (1<<0)
#define IN_COMMENT    (1<<1)
#define IN_CHAR       (1<<2)
#define IN_MASK       (1<<3)
uint8_t scanStatus = START_OF_LINE;

char inputBuffer[SCAN_BUFFER_SIZE];
uint8_t inputPos = 0;

void (*consumeFunction)(parsedToken *);
parsedToken parsed;


#define KEYWORD_COUNT 19
const char keyword_0[] PROGMEM = "while";
const char keyword_1[] PROGMEM = "if";
const char keyword_2[] PROGMEM = "def";
const char keyword_3[] PROGMEM = "yeet";
const char keyword_4[] PROGMEM = "True";
const char keyword_5[] PROGMEM = "False";
const char keyword_6[] PROGMEM = "return";
const char keyword_7[] PROGMEM = "put";
const char keyword_8[] PROGMEM = "set";
const char keyword_9[] PROGMEM = "get";
const char keyword_10[] PROGMEM = "and";
const char keyword_11[] PROGMEM = "or";
const char keyword_12[] PROGMEM = "else";
const char keyword_13[] PROGMEM = "not";
const char keyword_14[] PROGMEM = "del";
const char keyword_15[] PROGMEM = "debug";
const char keyword_16[] PROGMEM = "break";
const char keyword_17[] PROGMEM = "print";
const char keyword_18[] PROGMEM = "putval";



const char* const keywords[] PROGMEM = {
    keyword_0, keyword_1, keyword_2,  keyword_3,  keyword_4,  keyword_5, keyword_6, keyword_7, keyword_8,
    keyword_9, keyword_10, keyword_11, keyword_12, keyword_13, keyword_14, keyword_15, keyword_16, keyword_17, keyword_18
};

#define CONSUME(target) { parsed.token = target; consumeFunction(&parsed); }

// We hash identifiers to reduce memory usage of the strings.
// The used function is CRC32
uint32_t hash_identifier(const uint8_t* key, size_t length) {
  return crc32(key, length);
}

void processCurrent(bool consumeAll);
void processChar(char c);
void shiftBuffer(uint8_t num);

void scannerInit(void *consumer) {
    consumeFunction = consumer;
}

void processStr(char *str, uint8_t len) {
    for (uint8_t i=0; i < len; i++) {
        processChar(str[i]);
    }
}

void processChar(char c) {
    if (scanStatus & IN_CHAR) {
        if (scanStatus & IN_MASK) {
            scanStatus &= ~IN_MASK;
            switch (c) {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                default:
                    break;
            }
        }
    }
    else if (c == ' ' || c == '\t') {
        if (scanStatus&START_OF_LINE) {
            parsed.indent++;
            return;
        }
        processCurrent(true);
        return;
    }
    else if (c == '#') {
        scanStatus|=IN_COMMENT;
    }
    else if (c == '\n') {
        processCurrent(true);
        scanStatus|=START_OF_LINE;
        scanStatus&=~(IN_COMMENT|IN_MASK|IN_CHAR);
        CONSUME(ID_EOL);
        parsed.indent = 0;
        return;
    }
    else {
        scanStatus&=~START_OF_LINE;
    }

    if ((scanStatus & IN_CHAR) && c == '\\') {
        scanStatus ^= IN_MASK;
        return;
    }
    if (c == '\'') {
        scanStatus ^= IN_CHAR;
    }

    if (inputPos == SCAN_BUFFER_SIZE) {
        processCurrent(false);
    }
    if (!(scanStatus & IN_COMMENT)) {
        inputBuffer[inputPos++] = c;
    }
}

#define IS_DEC(c) ('0' <= c && c <= '9')
#define IS_BIN(c) ('0' == c || c == '1')
#define IS_HEX(c) (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))
#define IS_TEXT(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_'))

#define IS_SINGLE_CHAR(in, c, target) if (in == c) { parsed.token = target; consumeFunction(&parsed); shiftBuffer(1); shifted=true; continue; }
void processCurrent(bool consumeAll) {
    bool init = true;
    bool shifted = true;
    while ((init || consumeAll) && shifted) {
        if (inputPos == 0) {
            return;
        }
        init = false;
        shifted = false;
        char c = inputBuffer[0];

        IS_SINGLE_CHAR(c, ':', ID_COLON);
        IS_SINGLE_CHAR(c, ';', ID_SEMICOLON);
        IS_SINGLE_CHAR(c, ',', ID_COMMA);
        IS_SINGLE_CHAR(c, '(', ID_OPEN_ROUND);
        IS_SINGLE_CHAR(c, ')', ID_CLOSE_ROUND);
        IS_SINGLE_CHAR(c, '[', ID_OPEN_SQUARE);
        IS_SINGLE_CHAR(c, ']', ID_CLOSE_SQUARE);
        IS_SINGLE_CHAR(c, '+', ID_PLUS);
        IS_SINGLE_CHAR(c, '-', ID_MINUS);
        IS_SINGLE_CHAR(c, '/', ID_DIV);
        IS_SINGLE_CHAR(c, '%', ID_MOD);

        if (inputPos == 1) {
            IS_SINGLE_CHAR(c, '<', ID_LT);
            IS_SINGLE_CHAR(c, '>', ID_GT);
            IS_SINGLE_CHAR(c, '=', ID_ASSIGN);
            IS_SINGLE_CHAR(c, '!', ID_NOT);
            IS_SINGLE_CHAR(c, '*', ID_MULT);
        }
        else if (c == '<') {
            shiftBuffer(1);
            shifted = true;
            IS_SINGLE_CHAR(inputBuffer[0], '<', ID_SHIFT_LEFT);
            IS_SINGLE_CHAR(inputBuffer[0], '=', ID_LE);
            CONSUME(ID_LT);
            continue;
        }
        else if (c == '>') {
            shiftBuffer(1);
            shifted = true;
            IS_SINGLE_CHAR(inputBuffer[0], '>', ID_SHIFT_RIGHT);
            IS_SINGLE_CHAR(inputBuffer[0], '=', ID_GE);
            CONSUME(ID_GT);
            continue;
        }
        else if (c == '!') {
            shiftBuffer(1);
            shifted = true;
            IS_SINGLE_CHAR(inputBuffer[0], '=', ID_NEQ);
            CONSUME(ID_NOT);
            continue;
        }
        else if (c == '=') {
            shiftBuffer(1);
            shifted = true;
            IS_SINGLE_CHAR(inputBuffer[0], '=', ID_EQU);
            CONSUME(ID_ASSIGN);
            continue;
        }
        else if (c == '*') {
            shiftBuffer(1);
            shifted = true;
            IS_SINGLE_CHAR(inputBuffer[0], '*', ID_EXP);
            CONSUME(ID_MULT);
            continue;
        }
        else if (inputPos >= 3) {
            if (inputBuffer[0] == '\'' && inputBuffer[2] == '\'') {
                parsed.token = ID_NUM;
                parsed.tokenValue = inputBuffer[1];
                consumeFunction(&parsed);
                shiftBuffer(3);
                shifted = true;
                continue;
            }
        }

        for (uint8_t i=0; i < KEYWORD_COUNT; i++) {
            char *strpos = (char *)pgm_read_word(&(keywords[i]));

            if (inputPos < strlen_P(strpos)) {
                continue;
            }
            if (strstr_P(inputBuffer, strpos) != inputBuffer) {
                continue;
            }
            else {
                if (inputPos > strlen_P(strpos)) {
                    if (IS_TEXT(inputBuffer[strlen_P(strpos)]) || IS_DEC(inputBuffer[strlen_P(strpos)])) {
                        continue;
                    }
                }

                if ((ID_WHILE+i) == ID_TRUE) {
                    parsed.token = ID_NUM;
                    parsed.tokenValue = 1;
                    consumeFunction(&parsed);
                }
                else if ((ID_WHILE+i) == ID_FALSE) {
                    parsed.token = ID_NUM;
                    parsed.tokenValue = 0;
                    consumeFunction(&parsed);
                }
                else {
                    CONSUME(ID_WHILE+i);
                }

                shiftBuffer(strlen_P(strpos));
                shifted = true;
                break;
            }
        }
        if (shifted) {
            continue;
        }

        if (IS_DEC(c)) { // Looks like a number
            uint8_t numLen = 1;
            uint8_t pos = 0;
            uint8_t base = 10;

            if (c == '0' && inputPos > 1) {
                if (inputBuffer[1] == 'x' || inputBuffer[1] == 'X') {
                    base = 16;
                    pos = 2;
                    numLen++;
                }
                if (inputBuffer[1] == 'b' || inputBuffer[1] == 'B') {
                    base = 2;
                    pos = 2;
                    numLen++;
                }
            }

            while (numLen < inputPos) {
                c = inputBuffer[numLen];
                if (base == 10 && IS_DEC(c)) {
                    numLen++;
                }
                else if (base == 16 && IS_HEX(c)) {
                    numLen++;
                }
                else if (base == 2 && IS_BIN(c)) {
                    numLen++;
                }
                else {
                    break;
                }
            }
            uint32_t num = 0;
            for (; pos < numLen; pos++) {
                if (base != 16) {
                    num = (num * base) + inputBuffer[pos]-'0';
                }
                else {
                    if (inputBuffer[pos] >= 'a') {
                        num = (num << 4) + inputBuffer[pos] - 'a' + 10;
                    }
                    else if (inputBuffer[pos] >= 'A') {
                        num = (num << 4) + inputBuffer[pos] - 'A' + 10;
                    }
                    else {
                        num = (num << 4) + inputBuffer[pos]-'0';
                    }
                }
            }
            parsed.token = ID_NUM;
            parsed.tokenValue = num;
            consumeFunction(&parsed);
            shiftBuffer(numLen);
            shifted = true;
        }
        else if (IS_TEXT(c)) { // Starts like an identifier
            uint8_t idLen = 1;
            while (idLen < inputPos) {
                c = inputBuffer[idLen];
                if (IS_TEXT(c) || IS_DEC(c)) {
                    idLen++;
                }
                else {
                    break;
                }
            }

            parsed.token = ID_NAME;
            parsed.tokenValue = (hash_identifier((uint8_t *)inputBuffer, idLen) >> 4) & 0xFFFFFF; // We only use middle 24 bits to save space
            consumeFunction(&parsed);
            shiftBuffer(idLen);
            shifted=true;
         }
    }
    if (consumeAll && inputPos != 0) {
        sevenSegmentPrintHex(0xE2);
        inputPos = 0;
        return;
    }
}

void shiftBuffer(uint8_t num) {
    memmove(inputBuffer, inputBuffer+num, inputPos-num);
    inputPos = inputPos-num;
}