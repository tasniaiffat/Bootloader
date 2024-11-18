#include<flash.h>

uint32_t get_sector(uint32_t address) {
    if (address < FLASH_SECTOR_1) {
        return FLASH_SECTOR_0;
    } else if (address < FLASH_SECTOR_2) {
        return FLASH_SECTOR_1;
    } else if (address < FLASH_SECTOR_3) {
        return FLASH_SECTOR_2;
    } else if (address < FLASH_SECTOR_4) {
        return FLASH_SECTOR_3;
    } else if (address < FLASH_SECTOR_5) {
        return FLASH_SECTOR_4;
    } else if (address < FLASH_SECTOR_6) {
        return FLASH_SECTOR_5;
    } else if (address < FLASH_SECTOR_7) {
        return FLASH_SECTOR_6;
    } 
    return FLASH_SECTOR_7;
}

uint32_t get_sector_size(uint32_t address) {
    uint32_t sector = get_sector(address);
    
    if (sector <= FLASH_SECTOR_3)
        return 16 * 1024;  // 16KB for sectors 0-3
    else if (sector == FLASH_SECTOR_4)
        return 64 * 1024;  // 64KB for sector 4
    else
        return 128 * 1024; // 128KB for sectors 5-7
}

void flash_unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

void flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

// // Flash programming helper function
// uint32_t FLASH_Program_Word(uint32_t address, uint32_t data) {
//     while(FLASH->SR & FLASH_SR_BSY);
//     // Clear any previous errors
//     FLASH->SR |= FLASH_SR_EOP | FLASH_SR_SOP | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR;

//    // Set programming configuration
//     FLASH->CR &= ~FLASH_CR_PSIZE;
//     FLASH->CR |= FLASH_CR_PSIZE_1; // Set for 32-bit programming
//     FLASH->CR |= FLASH_CR_PG;      // Programming mode
    
//     // Program the word
//     *(volatile uint32_t*)address = data;
    
//     // Wait for programming to complete
//     while(FLASH->SR & FLASH_SR_BSY);
    
//     // Clear PG bit
//     FLASH->CR &= ~FLASH_CR_PG;

//     // Check for errors
//     if(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
//     {
//         return 1; // Error occurred
//     }

//     // Verify written data
//     if(*(uint32_t*)address != data)
//     {
//         return 1; // Verification failed
//     }
    
//     return 0; // Success
// }

// uint32_t FLASH_EraseSector(uint32_t sector_address) {
//     // Wait if flash is busy
//     while(FLASH->SR & FLASH_SR_BSY);
    
//     // Clear any previous errors
//     FLASH->SR |= FLASH_SR_EOP | FLASH_SR_SOP | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR;
    
//     // Get sector number
//     uint32_t sector = get_sector(sector_address);
    
//     // Set sector erase
//     FLASH->CR &= ~FLASH_CR_PSIZE;
//     FLASH->CR |= FLASH_CR_PSIZE_1; // Set for 32-bit parallelism
//     FLASH->CR |= FLASH_CR_SER;
    
//     // Select sector
//     FLASH->CR &= ~FLASH_CR_SNB;
//     FLASH->CR |= (sector << 3);
    
//     // Start erase
//     FLASH->CR |= FLASH_CR_STRT;
    
//     // Wait for completion
//     while(FLASH->SR & FLASH_SR_BSY);
    
//     // Clear sector erase flag
//     FLASH->CR &= ~FLASH_CR_SER;
    
//     // Check for errors
//     if(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
//     {
//         return 1; // Error occurred
//     }
    
//     return 0; // Success
// }

// // Write 1024 bytes to flash starting at given address
// uint32_t write_flash_chunk(uint32_t start_address, uint8_t* data, uint32_t len) {
//     uint32_t status;
    
//     // 1. Unlock flash
//     flash_unlock();
    
//     // 2. Ensure voltage range is correct for flash operations
//     FLASH->CR &= ~FLASH_CR_PSIZE;
//     FLASH->CR |= FLASH_CR_PSIZE_1;  // Set for 32-bit parallelism
    
//     // 3. Clear all error flags before starting
//     FLASH->SR |= FLASH_SR_EOP | FLASH_SR_SOP | FLASH_SR_WRPERR | 
//                  FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR;
    
//     // 4. Erase the sector if this is the first chunk for this sector
//     uint32_t current_sector = get_sector(start_address);
//     uint32_t sector_start = (current_sector <= FLASH_SECTOR_3) ? 
//                             FLASH_SECTOR_0 + (current_sector * 16 * 1024) :
//                             (current_sector == FLASH_SECTOR_4) ? 
//                             FLASH_SECTOR_4 :
//                             FLASH_SECTOR_5 + ((current_sector - 5) * 128 * 1024);
                            
//     if (start_address == sector_start) {
//         status = FLASH_EraseSector(start_address);
//         if (status != 0) {
//             flash_lock();
//             return 1;
//         }
//     }
    
//     // 5. Program data word by word (32-bits at a time)
//     for(uint32_t i = 0; i < len; i += 4) {
//         // Wait for any previous operations to complete
//         while(FLASH->SR & FLASH_SR_BSY);
        
//         uint32_t word_data = (data[i+3] << 24) | 
//                             (data[i+2] << 16) | 
//                             (data[i+1] << 8)  | 
//                             data[i];
                            
//         // Enable programming mode for each word
//         FLASH->CR |= FLASH_CR_PG;
        
//         if(FLASH_Program_Word(start_address + i, word_data) != 0) {
//             flash_lock();
//             return 1;
//         }
        
//         // Disable programming mode
//         FLASH->CR &= ~FLASH_CR_PG;
        
//         // Verify written data
//         if(*(volatile uint32_t*)(start_address + i) != word_data) {
//             flash_lock();
//             return 1;
//         }
//     }
    
//     flash_lock();
//     return 0; // Success
// }

void erase_os_memory_in_flash(void)
{
 
    /*
    sector 4: 0x0801 0000 - 0x0801 FFFF length= 64KB
    sector 5: 0x0802 0000 - 0x0803 FFFF length= 128KB
    sector 6: 0x0804 0000 - 0x0805 FFFF length= 128KB
    sector 7: 0x0806 0000 - 0x0807 FFFF length= 128KB
 
    */
 
    flash_unlock();
 
    for (uint8_t sector = 0x4; sector <= 0x7; sector++)
    {
 
        while (FLASH->SR & FLASH_SR_BSY)
            ; // Wait for the flash to be ready
 
        // Clear previous errors
        FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);
 
        FLASH->CR |= FLASH_CR_SER; // Sector erase enabled
 
        FLASH->CR &= ~(0xF << 3); // Clear the sector number
        FLASH->CR |= sector << 3; // select the sector to erase in hex
 
        FLASH->CR |= FLASH_CR_STRT; // start the erase operation
 
        while (FLASH->SR & FLASH_SR_BSY);
 
        // Check for errors
        if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            kprintf("Flash write error at sector: %d\n", sector);
            kprintf("SR: 0x%x\n", FLASH->SR);
            kprintf("CR: 0x%x\n", FLASH->CR);
            kprintf("WRPERR: 0x%x\n", FLASH->SR & FLASH_SR_WRPERR);
            kprintf("PGAERR: 0x%x\n", FLASH->SR & FLASH_SR_PGAERR);
            kprintf("PGPERR: 0x%x\n", FLASH->SR & FLASH_SR_PGPERR);
            kprintf("PGSERR: 0x%x\n", FLASH->SR & FLASH_SR_PGSERR);
 
 
            FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);
 
            FLASH->CR &= ~FLASH_CR_SER; 
 
            flash_lock();
            return; 
        }
 
        FLASH->CR &= ~FLASH_CR_SER;         // Clear sector erase bit after each erase
 
 
    }
 
    flash_lock();
}
 
int flash_erased_check(void)
{
 
    int start_address = FLASH_BASE;
    int end_address = FLASH_BASE + 0x8000U;
 
    for (int i = start_address; i < end_address; i += 4)
    {
        if (*(uint32_t *)i != 0xFFFFFFFF)
        {
            return 0;
        }
    }
 
    return 1;
}


uint8_t flash_write(uint8_t *data, uint32_t length, uint32_t start_address)
{
    // kprintf("write len : %d\n", length);
    flash_unlock();
 
    // Clear previous errors
    FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);
 
    while (FLASH->SR & FLASH_SR_BSY)
        ; // Wait for the flash to be ready
 
    FLASH->CR |= FLASH_CR_PG; // Enable programming mode
 
    for (uint32_t i = 0; i < length; i++)
    {
        // Program byte (8-bit) data to flash
        *(uint8_t *)(start_address + i) = data[i];
 
        while (FLASH->SR & FLASH_SR_BSY)
            ; // Wait for the flash to be ready
 
        // Verify the written data
        if (*(uint8_t *)(start_address + i) != data[i]) {
            // kprintf("Verification failed at address 0x%x\n", (start_address + i));
            flash_lock();
            return 1;
        }
 
        // Check for errors
        if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            
            // kprintf("SR: 0x%x\n", FLASH->SR);
            // kprintf("CR: 0x%x\n", FLASH->CR);
            // kprintf("WRPERR: 0x%x\n", FLASH->SR & FLASH_SR_WRPERR);
            // kprintf("PGAERR: 0x%x\n", FLASH->SR & FLASH_SR_PGAERR);
            // kprintf("PGPERR: 0x%x\n", FLASH->SR & FLASH_SR_PGPERR);
            // kprintf("PGSERR: 0x%x\n", FLASH->SR & FLASH_SR_PGSERR);
            FLASH->CR &= ~FLASH_CR_PG; // Disable programming mode
            flash_lock();
            return 1; // Exit on error
        }
        return 0;
    }
 
    FLASH->CR &= ~FLASH_CR_PG; // Programming disabled
 
    flash_lock();
}
 
 
 
 
void flash_read(uint8_t *data, uint32_t length, uint32_t start_address)
{   
 
    kprintf("read len : %d\n", length);
 
    for (uint32_t i = 0; i < length; i += 1)
    {
 
        // *(uint8_t *)(data + i) = *(uint8_t *)(start_address + i);
 
        uint8_t val = *(uint8_t *)(start_address + i);
 
        kprintf("read : %c --- index : %d\n", val, i);
 
        while (FLASH->SR & FLASH_SR_BSY);
 
 
    }
}
