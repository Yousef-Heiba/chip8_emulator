#include <chip8.h>

chip8::chip8 ()
    : randGen(std::random_device{}()),
      randByte(0, 255)
{
    program_counter = memory_start_address;
    for (uint8_t i{}; i < font.size(); ++i) {
        ram[font_start_address + i] = font[i];
    }
}

std::expected<void, std::string> chip8::loadROM (const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return std::unexpected("File stream is in a failed state.");
    } else if (file.is_open()) {
        uint16_t rom_address{memory_start_address};
        char byte;
        while (file.get(byte)) {
            if (rom_address >= ram.size()) {
                return std::unexpected("ROM is too large.");
            }
            ram[rom_address] = static_cast<uint8_t> (byte);
            rom_address++;
        }
    }
    return {};
}

void chip8::cycle() {
    assert (program_counter + 1 >= ram.size());
    opcode = ((ram[program_counter] << 8) | ram[program_counter+1]); 
    program_counter += 2;
}

void chip8::opcode_00E0() {
    video_output = {};
}

void chip8::opcode_00EE() {
    stack_pointer--;
    program_counter = stack[stack_pointer];
}

void chip8::opcode_1NNN() {
    program_counter = opcode & 0x0FFFu;
}

void chip8::opcode_2NNN() {
    stack[stack_pointer] = program_counter;
    stack_pointer++;   
    program_counter = opcode & 0X0FFFu; 
}

void chip8::opcode_3XKK() {
    if (registers[(opcode & 0x0F00u) >> 8] == (opcode & 0x00FFu)) {
        program_counter += 2;
    }
}

void chip8::opcode_4XKK() {
    if (registers[(opcode & 0x0F00u) >> 8] != (opcode & 0x00FFu)) {
        program_counter += 2;
    }
}

void chip8::opcode_5XY0() {
    if (registers[(opcode & 0x0F00u) >> 8] == registers[(opcode & 0x00F0u) >> 4]) {
        program_counter += 2;
    }
}

void chip8::opcode_6XKK() {
    registers[(opcode & 0x0F00u) >> 8u] = (opcode & 0x00FFu);
}

void chip8::opcode_7XKK() {
    registers[(opcode & 0x0F00u) >> 8u] += (opcode & 0x00FFu);
}

void chip8::opcode_8XY0() {
    registers[(opcode & 0x0F00u) >> 8u] = registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY1() {
    registers[(opcode & 0x0F00u) >> 8u] |= registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY2() {
    registers[(opcode & 0x0F00u) >> 8u] &= registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY3() {
    registers[(opcode & 0x0F00u) >> 8u] ^= registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY4() {
    if ((registers[(opcode & 0x0F00u) >> 8u] + registers[(opcode & 0x00F0u) >> 4u]) > 0xFFu) {
        registers[15] = 1; 
    } else {
        registers[15] = 0;
    }
    registers[(opcode & 0x0F00u) >> 8u] += registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY5() {
    if (registers[(opcode & 0x0F00u) >> 8u] >= registers[(opcode & 0x00F0u) >> 4u]) {
        registers[15] = 1; 
    } else {
        registers[15] = 0;
    }
    registers[(opcode & 0x0F00u) >> 8u] -= registers[(opcode & 0x00F0u) >> 4u];
}

void chip8::opcode_8XY6() {
    if (((registers[(opcode & 0x0F00u) >> 8]) & 0b0001)  == 1) {
        registers[15] = 1;
    } else {
        registers[15]  = 0;
    }

    registers[(opcode & 0x0F00u) >> 8] >>= 1; 
}

void chip8::opcode_8XY7() {
    if ((registers[(opcode & 0x00F0u) >> 4]) >= (registers[(opcode & 0x0F00u) >> 8])) {
        registers[15] = 1;
    } else {
        registers [15] = 0;
    }

    registers[(opcode & 0x0F00u) >> 8] = registers[(opcode & 0x00F0u) >> 4] - registers[(opcode & 0x0F00u) >> 8];
}

void chip8::opcode_8XYE() {
    if (((registers[(opcode & 0x0F00u) >> 8] & 0b10000000u) >> 7) == 1) {
        registers[15] = 1;
    } else { 
        registers[15] = 0;
    }

    registers[(opcode & 0x0F00u) >> 8] <<= 1;
}

void chip8::opcode_9XY0() {
    if (registers[(opcode & 0x0F00u) >> 8] != registers[(opcode & 0x00F0u) >> 4]) {
        program_counter += 2;
    }  
}

void chip8::opcode_ANNN() {
    index_register = opcode & 0x0FFFu;
}

void chip8::opcode_BNNN() {
    program_counter = (opcode & 0x0FFFu) + (registers[0]);
}

void chip8::opcode_CXKK() {
    registers[(opcode & 0x0F00u) >> 8] = static_cast<uint8_t>(randByte(randGen)) & (opcode & 0x00FFu);
}