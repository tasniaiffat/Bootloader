import serial
import time
import os
import zlib
from typing import List, Tuple
import json
import math

POLYNOMIAL = 0xEDB88320

def read_version() -> str:
    try:
        with open('version.json', 'r') as f:
            version_data = json.load(f)
            major_version = version_data.get('major', 0)
            minor_version = version_data.get('minor', 0)
            print(f"Read version: {major_version}.{minor_version}")
            return f"{major_version}.{minor_version}"
    except (FileNotFoundError, json.JSONDecodeError) as e:
        print(f"Error reading version file: {e}")
        return "0.0"


class CRC32Calculator:
    def __init__(self):
        # Precompute the CRC lookup table for efficiency
        self.crc_table = self.generate_crc_table()

    def generate_crc_table(self):
        """Generate a CRC-32 lookup table using the STM32 polynomial."""
        table = []
        for i in range(256):
            crc = i << 24
            for _ in range(8):
                if crc & 0x80000000:
                    crc = (crc << 1) ^ POLYNOMIAL
                else:
                    crc = crc << 1
            table.append(crc & 0xFFFFFFFF)
        return table

    def reverse_bits(self, byte: int) -> int:
        """Reverse the bits in a byte (bit reflection)."""
        reversed_byte = 0
        for i in range(8):
            reversed_byte = (reversed_byte << 1) | (byte & 1)
            byte >>= 1
        return reversed_byte

    def calc_crc_32(self, data: bytes, crc: int = 0xFFFFFFFF) -> int:
        """Calculate CRC32 to match STM32 hardware CRC behavior."""
        for byte in data:
            # Reflect input byte
            byte = self.reverse_bits(byte)
            # Update CRC using the lookup table, ensuring proper masking
            crc = self.crc_table[((crc >> 24) ^ byte) & 0xFF] ^ (crc << 8)
        # Reflect output CRC
        return self.reverse_bits(crc & 0xFFFFFFFF)

# Example of CRC calculation for "Hello"
crc_calculator = CRC32Calculator()

def crc32(s: str) -> int:
    crc = 0xFFFFFFFF
    polynomial = 0x04C11DB7  # Reversed CRC32 polynomial (LSB-first)

    for ch in s:
        byte = ord(ch)  # Get the ASCII value of each character
        for _ in range(8):  # Process each bit
            # Extract the least significant bit (LSB)
            b = (byte ^ (crc & 0xFF)) & 1  
            crc >>= 1  # Shift the CRC register
            if b:
                crc ^= polynomial  # Apply the polynomial if the bit is set
            byte >>= 1  # Shift the byte to the right to process the next bit

    return ~crc & 0xFFFFFFFF  # Return the CRC value with the complement

# # Example usage:
# input_string = "Hello"
# crc_value = crc32(input_string)
# print(f"CRC32 value for '123456789': {crc_value:#010x}")


class BootloaderTerminal:
    def __init__(self, port: str = 'COM4', baudrate: int = 115200):
        try:
            self.serial = serial.Serial(port, baudrate, timeout=1.0)
            self.current_version = read_version()  # Should be stored/managed better
            self.kernel_binary = "target/duos"
            self.packet_size = 1024
            self.max_retries = 3
            print(f"Connected to {port} at {baudrate} baud")
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")
            raise SystemExit
        except Exception as e:
            print(f"Unexpected error: {e}")
            raise SystemExit

    def calc_crc_32(self, data, crc = 0xFFFFFFFF) -> int:
        """Calculate CRC32 to match STM32 hardware CRC behavior."""
        for byte in data:
            crc = self.crc32(byte, crc)
            
        remaining_bytes = len(data) % 4
        if remaining_bytes > 0:
            for _ in range(4 - remaining_bytes):
                crc = self.crc32(0, crc)

        return crc
    
    def crc32(self, data, crc = 0xFFFFFFFF) -> int:
        crc = crc ^ data

        for i in range(32):
            if (crc & 0x80000000) != 0:
                crc = (crc << 1) ^ POLYNOMIAL
                
            else:
                crc = crc << 1
        
        return crc & 0xFFFFFFFF

    # def calculate_crc32(self, data: bytes) -> int:
    #     return zlib.crc32(data) & 0xFFFFFFFF

    # def calculate_crc32(self, data: bytes) -> int:
    #     crc = zlib.crc32(data) & 0xFFFFFFFF  # Calculate CRC
    #     return crc ^ 0xFFFFFFFF              # Remove final XOR
    
    def calculate_crc32(self, data: bytes) -> int:
        # Calculate the CRC using zlib
        crc = zlib.crc32(data) & 0xFFFFFFFF  # zlib applies a final XOR with 0xFFFFFFFF by default
        # Undo the final XOR to match the STM32 hardware implementation
        return crc ^ 0xFFFFFFFF

    
    def verify_crc32(self, data: bytes, expected_crc: int) -> bool:
        calculated_crc = self.calculate_crc32(data)
        return calculated_crc == expected_crc

    def wait_for_ack(self, timeout: float = 1.0) -> bool:
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            if self.serial.in_waiting:
                response = self.serial.readline().decode().strip()
                if response == "ACK":
                    return True
                elif response == "NACK":
                    return False
            time.sleep(0.1)
        return False

    def send(self, data: bytes, expect_ack: bool = True) -> bool:
        print(f"Sending: {data}")
        self.serial.write(data)
        return True


    def read_firmware(self,binary_path:str):
        try:
            with open(binary_path, 'rb') as f:
                firmware_data = f.read()
        except FileNotFoundError:
            print(f"Error: Could not find {binary_path}")
            return False
        return firmware_data


    def get_firmware_size(self,firmware_data:bytes):
        return len(firmware_data)
    

    def send_firmware_size(self,file_size:int):
        return self.send_and_receive(f"{file_size}".encode())

    def send_and_receive(self,data):
        self.serial.write(data)
        while True:
            if self.serial.in_waiting:
                response = self.serial.readline().decode().strip()
                print(f"Received response length: {len(response)}")
                print(f"Raw received bytes: {response.encode().hex()}")
                print(f"Decoded response: {response}")
                return response
            time.sleep(0.1)

    def send_data(self, data: bytes):
        """Function to send data over the serial port."""
        print(f"Sending data: {data}")
        self.serial.write(data)

    def receive_data(self, timeout: float = 1.0) -> str:
        """Function to receive data over the serial port."""
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            if self.serial.in_waiting:
                response = self.serial.readline().decode().strip()
                print(f"Received response length: {len(response)}")
                print(f"Raw received bytes: {response.encode().hex()}")
                print(f"Decoded response: {response}")
                return response
            time.sleep(0.1)
        return "timeout"  

    def send_firmware_packet(self, data,iter) -> bool:
        crc_value = self.calc_crc_32(data)
        hellocrc = crc32("123456789")  # 'b' converts string to bytes
        print(f"CRC32 for '123456789': {hellocrc:#010x}")  # Print CRC as hexadecimal
        print(f"{hellocrc}")
        print(f"Sending packet with CRC32: {crc_value:#010x}")
        print(f"Sending packet with CRC32: {crc_value}")
        #send crc
        # self.serial.write(f"{10}".encode())
        # recv_crc = self.serial.readline().decode().strip()
        # print("HELLO" + recv_crc)
        # ALLAH_PLS = self.receive_data()
        # print("HJV<JKSV" + ALLAH_PLS)
        recv = self.send_and_receive(data)
        print(f"ACK: {recv}")
        if recv == f"ACK {iter}":
            return True
        return False

    def run(self):
        while True:
            try:
                if self.serial.in_waiting:
                    print("Message received")
                    encoded_message = self.serial.readline()
                    message = encoded_message.decode(errors='replace').strip()
                    print(f"Received: {message}")

                    if message.startswith("Hudai"):
                        print("Paisi")
                    
                    elif message.startswith("VERSION_REQ:"):
                        bootloader_version = message.split(":")[1]
                        print(f"Current version: {self.current_version}")
                        print(f"Bootloader version: {bootloader_version}")
                        
                        self.send(f"{self.current_version}".encode())
                        continue

                    elif message.startswith("FIRMWARE_REQ:"):
                        print("Received firmware request")
                        firmware_data = self.read_firmware(self.kernel_binary)
                        if not firmware_data:
                            print("Firmware read failed")
                            continue
                        file_size = self.get_firmware_size(firmware_data)
                        print(f"Sending firmware of size {file_size} bytes")
                        recv=self.send_firmware_size(file_size)
                        print(recv)
                        time.sleep(0.1)
                        iteration = math.ceil(file_size/self.packet_size)
                        print(f"Total iteration: {iteration}")

                        i=0
                        while(i<iteration):
                            print(f"Sending firmware packet {i+1}/{iteration}")
                            res = self.send_firmware_packet(firmware_data[i*self.packet_size:(i+1)*self.packet_size]+b'`',i)
                            # print(f"Packet Info: {firmware_data[i*self.packet_size:(i+1)*self.packet_size]+b'`'}")
                            if not res:
                                print("Firmware update failed")
                                break
                            i+=1
                        print("Firmware update successful")
                        continue

                time.sleep(0.1)

            except serial.SerialException as e:
                print(f"Serial communication error: {e}")
                break
            except Exception as e:
                print(f"Unexpected error: {e}")
                break

if __name__ == "__main__":
    terminal = BootloaderTerminal()
    try:
        terminal.run()
    except KeyboardInterrupt:
        print("\nTerminal stopped by user")
    finally:
        terminal.serial.close()