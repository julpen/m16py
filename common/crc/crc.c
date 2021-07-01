#include "crc.h"

#define CRC32_GENERATOR 0xEDB88320


uint32_t crc32(const uint8_t *data, int len) {
    uint32_t result = 0xFFFFFFFF;
    for (uint8_t i=0; i < len; i++) {
        uint8_t byte = data[i];
        for (uint8_t k=0; k < 8; k++) {
            uint8_t b = (byte ^ result) & 1;
            result >>= 1;

            if (b) {
                result ^= CRC32_GENERATOR;
            }
            byte = byte >> 1;
        }
    }
    return result;
}