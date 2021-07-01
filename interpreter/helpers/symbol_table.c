#include "../interpreter.h"
#include "../interpreter_strings.h"

int8_t symbolTableDepth = 0;

#define SYMBOL_IS_FUNCTION(hash) (hash & (1UL<<30))
#define GET_SYMBOL_LEVEL(hash) ((hash >> 24) & 0x3F)

void increaseSymbolTableDepth() {
    symbolTableDepth++;
}

void decreaseSymbolTableDepth() {
    for (uint8_t i=0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].name & 0x80000000) { // Check if populated
            if (GET_SYMBOL_LEVEL(symbolTable[i].name) == symbolTableDepth) {
                symbolTable[i].name = 0;
            }
        }
    }
    if (symbolTableDepth > 0) {
        symbolTableDepth--;
    }
}

SymbolEntry *getSymbolEntry(uint32_t hash) {
    for(int8_t level=symbolTableDepth; level >= 0;) { // Check current level and top level (global vars/functions)
        for (uint8_t i=0; i < MAX_SYMBOLS; i++) {
            if (symbolTable[i].name & 0x80000000) { // Check if populated
                if ((symbolTable[i].name & 0xFFFFFF) == hash && GET_SYMBOL_LEVEL(symbolTable[i].name) == level) {
                    return &symbolTable[i];
                }
            }
        }
        if (level == 0) {
            return NULL;
        }
        level=0;
    }
    return NULL;
}

void deleteSymbolEntry(uint32_t hash) {
    for (uint8_t i=0; i < MAX_SYMBOLS; i++) {
        if (symbolTable[i].name & 0x80000000) { // Check if populated
            if ((symbolTable[i].name & 0xFFFFFF) == hash) {
                symbolTable[i].name = 0;
            }
        }
    }
}

SymbolEntry *createSymbolEntry(uint32_t hash, uint16_t val) {
    for (uint8_t i=0; i < MAX_SYMBOLS; i++) {
        if (!(symbolTable[i].name & 0x80000000)) { // Find empty entry in table
            symbolTable[i].name = 0x80000000 | ((uint32_t) symbolTableDepth << 24) | hash;
            symbolTable[i].value = val;
            return &symbolTable[i];
        }
    }
    return NULL;
}

SymbolEntry *getOrCreateSymbolEntry(uint32_t hash) {
    if (getSymbolEntry(hash) == NULL) {
        return createSymbolEntry(hash, 0);
    }
    return getSymbolEntry(hash);
}

void createFunctionEntry(uint32_t hash, uint8_t nodeId) {
    SymbolEntry *entry = getSymbolEntry(hash);
    if (entry != NULL) { // Remove old entry with same name
        if (SYMBOL_IS_FUNCTION(entry->name)) {
            freeNode(&nodes[entry->value]);
        }
        entry->name = 0;
    }
    entry = getOrCreateSymbolEntry(hash);
    entry->name |= (1UL << 30); // Use 2nd highest bit to mark functions
    entry->value = nodeId;
}