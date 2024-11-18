/*
 * Copyright (c) 2022
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys_init.h>
#include <cm4.h>
#include <kmain.h>
#include <stdint.h>
#include <sys_usart.h>
#include <kstdio.h>
#include <sys_rtc.h>
#include <kstring.h>
#include <system_config.h>
#include <UsartRingBuffer.h>
#include <flash.h>
#include <crc32.h>
#ifndef DEBUG
#define DEBUG 1
// #define VERSION_ADDR ((volatile uint8_t *)0x2000FFFCU)
#endif

#define BOOTLOADER_SIZE         (0x8000u)
#define MAIN_APP_START_ADDRESS  (FLASH_BASE+BOOTLOADER_SIZE)

static void vector_setup(void){
    SCB->VTOR = BOOTLOADER_SIZE; 
}

void jump_to_main(void){

        typedef void(*void_fn)(void);
        uint32_t *reset_vector_entry = (uint32_t *)(MAIN_APP_START_ADDRESS + 4U);
        uint32_t *reset_vector= (uint32_t *)(*reset_vector_entry);
        void_fn jump_fn= (void_fn)reset_vector;
        kprintf("Jumping to main application\n");
        jump_fn();

}

char* uint32_to_hex(uint32_t value) {
    static char hex_buffer[11];  // Buffer to hold the hex string (0xXXXXXXXX\0)

    // Start by filling the buffer with '0x'
    hex_buffer[0] = '0';
    hex_buffer[1] = 'x';

    // Convert each nibble (4 bits) of the uint32_t value to a character
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;  // Get the nibble (4 bits) for this position
        hex_buffer[2 + (7 - i)] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));  // Convert to hex character
    }

    // Null-terminate the string
    hex_buffer[10] = '\0';

    return hex_buffer;
}

int new_version_available(char* server_version, char* os_version) {
    if(strcomp(server_version, os_version) == 0) {
        return 0;
    }
    return 1;
}

void sleep(int t) {
    for (volatile int i = 0; i < t * 1000000; i++);
}

int ceiling_divide(int num, int divisor) {
    if (divisor == 0) {
        return -1; // Error: division by zero
    }
    return (num + divisor - 1) / divisor;
}

void kmain(void)
{
    __sys_init();
    crc32_init();
    uint8_t data[] = "123456789";  // Data as string "123456789"
    uint64_t length = sizeof(data) - 1;  // Get length without null terminator

    // Calculate CRC32
    uint32_t crc_value = crc32_calculate(data, length);

    // Convert CRC32 to hexadecimal string
    char* crc_hex = uint32_to_hex(crc_value);

    // Print the result using kprintf (simulating printf)
    kprintf("Hello CRC32 of '123456789': %s\n", crc_hex);
    // kprintf("Hudai hehe\n");
    char *VERSION_ADDR = "0.0";
    char buff[100];
    char crc_buff[100];
    int file_size;
    uint32_t recv_crc;
    int iteration;
    // char data[2048];
    uint32_t current_address = 0x08008000;
    kprintf("VERSION_REQ:\n");
    char server_version[100];
    read_str(server_version,5);
    kprintf("Server version: %s\n", server_version);
    sleep(1);
    if(new_version_available(server_version, VERSION_ADDR)) {
        kprintf("FIRMWARE_REQ:\n");
        sleep(1);
        // get file size
        read_str(buff,5);
        file_size = __str_to_num(buff, 10);
        iteration = ceiling_divide(file_size, 1024);
        kprintf("File size: %d, Iteration %d\n", file_size,iteration);

        //get crc from server
        // read_str(crc_buff,5);
        // recv_crc = __str_to_num(crc_buff, 10);
        // kprintf("CRC value: %u\n", recv_crc);

        sleep(1);
        for (int i = 0; i < iteration; i++) {
            if(i < iteration-1)
                read_str(data, 1024);
            else 
                read_str(data,file_size%1024);
            
            sleep(1);
            sleep(1);
            uint64_t data_length = 1024;
            uint32_t dummydata = 1302922624;
            uint32_t my_crc = crc32_calculate(data, data_length);
            // uint32_t my_crc = calculate_crc32(data, data_length);
           
            kprintf("ACK %u", my_crc);
            // if(my_crc == recv_crc){
            //     kprintf("ACK %d", i);
            // }
            // else kprintf("NAK %u %d", dummydata, file_size);
            
            sleep(1);
        }
        
        
    } else {
        kprintf("NO_UPDATE_NEEDED\n");
        sleep(1);
    }
    
    jump_to_main();
    while (1)
    {
      
    }
}
