#ifndef __CRC32_H__
#define __CRC32_H__
#ifdef __cplusplus
extern "C" {
#endif
    #include <stdint.h>
    #include <stddef.h>

    #define CRC32_POLYNOMIAL 0x04C11DB7

    uint32_t crc32_calculate(const uint8_t *data, uint64_t length);
    int crc32_verify(const uint8_t *data, uint64_t length, uint32_t expected_crc);
    void crc32_init(void);

#ifdef __cplusplus 
}
#endif
#endif