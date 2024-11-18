#ifndef __FLASH_H__
#define __FLASH_H__
#ifdef __cplusplus
extern "C" {
#endif
    #include <stdint.h>
    // #include<stm32f446xx.h>
    #include<sys_bus_matrix.h>

    // Flash memory constants for STM32F446RE
    #define FLASH_SECTOR_0     ((uint32_t)0x08000000) // 16KB
    #define FLASH_SECTOR_1     ((uint32_t)0x08004000) // 16KB
    #define FLASH_SECTOR_2     ((uint32_t)0x08008000) // 16KB
    #define FLASH_SECTOR_3     ((uint32_t)0x0800C000) // 16KB
    #define FLASH_SECTOR_4     ((uint32_t)0x08010000) // 64KB
    #define FLASH_SECTOR_5     ((uint32_t)0x08020000) // 128KB
    #define FLASH_SECTOR_6     ((uint32_t)0x08040000) // 128KB
    #define FLASH_SECTOR_7     ((uint32_t)0x08060000) // 128KB

    #define FLASH_KEY1         ((uint32_t)0x45670123)
    #define FLASH_KEY2         ((uint32_t)0xCDEF89AB)

    uint32_t get_sector(uint32_t address);
    uint32_t get_sector_size(uint32_t address);
    void flash_unlock(void);
    void flash_lock(void);
    void erase_os_memory_in_flash(void);
    int flash_erased_check(void);
    uint8_t flash_write(uint8_t *data, uint32_t length, uint32_t start_address);
    void flash_read(uint8_t *data, uint32_t length, uint32_t start_address);

#ifdef __cplusplus
}
#endif
#endif