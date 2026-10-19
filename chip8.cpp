#include <chip8.h>

chip8::chip8()
    : randGen(std::random_device{}()),
      randByte(0, 255)
{
    program_counter = memory_start_address;
    for (uint8_t i{}; i < font.size(); ++i)
    {
        ram[font_start_address + i] = font[i];
    }

    table.fill(&chip8::opcode_NULL);
    table0.fill(&chip8::opcode_NULL);
    table8.fill(&chip8::opcode_NULL);
    tableE.fill(&chip8::opcode_NULL);
    tableF.fill(&chip8::opcode_NULL);
    
    // Primary table
    table[0x0] = &chip8::Table0;
    table[0x1] = &chip8::opcode_1NNN;
    table[0x2] = &chip8::opcode_2NNN;
    table[0x3] = &chip8::opcode_3XKK;
    table[0x4] = &chip8::opcode_4XKK;
    table[0x5] = &chip8::opcode_5XY0;
    table[0x6] = &chip8::opcode_6XKK;
    table[0x7] = &chip8::opcode_7XKK;
    table[0x8] = &chip8::Table8;
    table[0x9] = &chip8::opcode_9XY0;
    table[0xA] = &chip8::opcode_ANNN;
    table[0xB] = &chip8::opcode_BNNN;
    table[0xC] = &chip8::opcode_CXKK;
    table[0xD] = &chip8::opcode_DXYN;
    table[0xE] = &chip8::TableE;
    table[0xF] = &chip8::TableF;

    // Secondary table 0
    table0[0x0] = &chip8::opcode_00E0;
    table0[0xE] = &chip8::opcode_00EE;

    // Secondary table 8
    table8[0x0] = &chip8::opcode_8XY0;
    table8[0x1] = &chip8::opcode_8XY1;
    table8[0x2] = &chip8::opcode_8XY2;
    table8[0x3] = &chip8::opcode_8XY3;
    table8[0x4] = &chip8::opcode_8XY4;
    table8[0x5] = &chip8::opcode_8XY5;
    table8[0x6] = &chip8::opcode_8XY6;
    table8[0x7] = &chip8::opcode_8XY7;
    table8[0xE] = &chip8::opcode_8XYE;

    // Secondary table E
    tableE[0x1] = &chip8::opcode_EXA1;
    tableE[0xE] = &chip8::opcode_EX9E;

    // Secondary table F
    tableF[0x07] = &chip8::opcode_FX07;
    tableF[0x0A] = &chip8::opcode_FX0A;
    tableF[0x15] = &chip8::opcode_FX15;
    tableF[0x18] = &chip8::opcode_FX18;
    tableF[0x1E] = &chip8::opcode_FX1E;
    tableF[0x29] = &chip8::opcode_FX29;
    tableF[0x33] = &chip8::opcode_FX33;
    tableF[0x55] = &chip8::opcode_FX55;
    tableF[0x65] = &chip8::opcode_FX65;
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
    // Fetch the next two-byte instruction.
    assert (program_counter + 1u < ram.size());
    opcode = (ram[program_counter] << 8u) | ram[program_counter + 1];
    program_counter += 2;

    // Decode and execute using the instruction's highest nibble.
    (this->*table[(opcode & 0xF000u) >> 12u])();

    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        --sound_timer;
    }
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


//TO-DO: Right now there is no need to optimize since this is O(8n) = o(n), but n is at most a 4 bit number (15)
// so worst case is 15 * 8 (2nd loop always runs 8 times) = 120, implement bit packing for fun

//todo: make x,y wrap around instead of causing segmentation fault
void chip8::opcode_DXYN()  {
    uint8_t x = registers[(opcode & 0x0F00u) >> 8];
    uint8_t y = registers[(opcode & 0x00F0u) >> 4];
    uint8_t n = opcode & 0x000Fu;
    registers[15] = 0;
    assert ((x + 7) < 64);
    if (n != 0) {
        assert ((y + n - 1) < 32);
    }
    for (int i{0}; i < n; ++i) {
        uint8_t sprite = ram[index_register + i]; 
        for (int j{0}; j < 8; j++) {
            if ((sprite & (0b10000000u >> j)) != 0) {
                if (video_output[y+i][x+j] == 0xFFFFFFFFu) {
                    registers[15] = 1;
                } 
                video_output[y + i][x + j] ^= 0xFFFFFFFFu;
            }
        }
    }
}

void chip8::opcode_EX9E() {
    uint8_t key = registers[(opcode & 0x0F00u) >> 8]; 
    assert(key <= 15);
    if (keys[key]) {
        program_counter += 2;
    }
}

void chip8::opcode_EXA1() {
    uint8_t key = registers[(opcode & 0x0F00u) >> 8]; 
    assert(key <= 15);
    if (!keys[key]) {
        program_counter += 2;
    }  
}

void chip8::opcode_FX07() {
    registers[(opcode & 0X0F00u) >> 8] = delay_timer;
}

void chip8::opcode_FX0A() {
    for (int i{0}; i < keys.size(); ++i) {
        if(keys[i]) {
            registers[(opcode & 0X0F00u) >> 8] = i;
            return;
        }
    }
    program_counter -= 2;
}

void chip8::opcode_FX15() {
    delay_timer = registers[(opcode & 0x0F00u) >> 8];
}

void chip8::opcode_FX18() {
    sound_timer = registers[(opcode & 0x0F00u) >> 8];
}

void chip8::opcode_FX1E() {
    index_register += registers[(opcode & 0x0F00u) >> 8];
}

void chip8::opcode_FX29() {
    uint16_t Vx = registers[(opcode & 0x0F00u) >> 8];
    index_register = 0x50 + ((Vx & 0x0Fu) * 5);
}

void chip8::opcode_FX33() {
    uint16_t temp_number = registers[(opcode & 0x0F00u) >> 8];
    ram[index_register] = temp_number / 100;
    ram[index_register + 1] = (temp_number / 10) % 10;
    ram[index_register + 2] = temp_number % 10; 
}

void chip8::opcode_FX55() {
    for (int i{}; i <= ((opcode & 0x0F00u) >> 8); ++i) {
        ram[index_register + i] = registers[i];
    }
}

void chip8::opcode_FX65() {
    for (int i{}; i <= ((opcode & 0x0F00u) >> 8); ++i) {
        registers[i] = ram[index_register + i];
    }
}
