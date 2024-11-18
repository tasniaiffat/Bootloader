#include<crc32.h>
#include<stm32f446xx.h>
#include<sys_bus_matrix.h>

// uint32_t crc32_calculate(const uint8_t *data, uint64_t length) {
//     uint32_t crc = 0xFFFFFFFF;
    
//     for (uint64_t i = 0; i < length; i++) {
//         crc ^= ((uint32_t)data[i] << 24);
//         for (int j = 0; j < 8; j++) {
//             if (crc & 0x80000000) {
//                 crc = (crc << 1) ^ CRC32_POLYNOMIAL;
//             } else {
//                 crc = crc << 1;
//             }
//         }
//     }
    
//     return ~crc; // Final XOR value
// }

void crc_init(void) {
    // Enable the CRC peripheral clock
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
}

void crc_reset(void) {
    // Reset the CRC unit (optional but recommended)
    RCC->AHB1RSTR |= RCC_AHB1RSTR_CRCRST;  // Set the reset bit
    RCC->AHB1RSTR &= ~RCC_AHB1RSTR_CRCRST; // Clear the reset bit
    // CRC->CR = CRC_CR_RESET;
}

// uint32_t crc32_calculate(const uint8_t *data, uint64_t length) {
//     crc_reset();  // Reset the CRC peripheral before use

//     // Process data in 32-bit words (4 bytes at a time)
//     const uint32_t *data_words = (const uint32_t *)data;
//     uint64_t word_count = length / 4;

//     // Write each 32-bit word to the CRC Data Register
//     for (size_t i = 0; i < word_count; i++) {
//         CRC->DR = data_words[i];
//     }

//     // Handle any remaining bytes that don't fit into a full word
//     const uint8_t *remaining_bytes = data + (word_count * 4);
//     for (size_t i = 0; i < (length % 4); i++) {
//         CRC->DR = remaining_bytes[i];
//     }

//     // Return the computed CRC value
//     return CRC->DR;
// }


uint32_t crc32_calculate(const uint8_t *data, uint64_t length) {
    crc_reset();  // Reset CRC before use

    // Write data to the CRC Data Register (CRC_DR)
    for (uint32_t i = 0; i < length; i++) {
        CRC->DR = data[i];  // Write data byte by byte
    }

    // Return the CRC value from the CRC Data Register
    return CRC->DR;  // The result of the CRC calculation
}

//araf
// uint32_t crc32_calculate(const uint8_t *data, uint64_t length)
// {
//     // Reset the CRC calculation unit
//     CRC->CR = CRC_CR_RESET;

//     // Write data to the CRC Data Register
//     for (uint32_t i = 0; i < length; i++)
//     {
//         CRC->DR = data[i];
//     }

//     // Return the CRC value
//     return CRC->DR;
// }

// uint32_t crc32_calculate(const uint8_t *data, uint64_t length) {
//     crc_reset();  // Reset CRC before use

//     // Write data to the CRC Data Register (CRC_DR)
//     for (size_t i = 0; i < length; i++) {
//         CRC->DR = data[i];
//     }

//     // Return the CRC value from the CRC Data Register with final XOR
//     return CRC->DR ^ 0xFFFFFFFF;
// }


int crc32_verify(const uint8_t *data, uint64_t length, uint32_t expected_crc) {
    uint32_t calculated_crc = crc32_calculate(data, length);
    return (calculated_crc == expected_crc) ? 1 : 0;
}

void crc32_init(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;  // Enable CRC clock on AHB1 bus

}

