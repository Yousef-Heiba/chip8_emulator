#include "chip8.h"

#include <algorithm>

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
    last_timer_update = std::chrono::steady_clock::now();
    timer_elapsed = {};
    return {};
}

void chip8::cycle() {
    updateTimers();

    // Fetch the next two-byte instruction.
    checkMemoryRange(program_counter, 2);
    opcode = (ram[program_counter] << 8u) | ram[program_counter + 1];
    program_counter += 2;

    // Decode and execute using the instruction's highest nibble.
    (this->*table[(opcode & 0xF000u) >> 12u])();
}

void chip8::updateTimers() {
    auto now = std::chrono::steady_clock::now();
    timer_elapsed += now - last_timer_update;
    last_timer_update = now;

    // Preserve fractional ticks so instruction speed does not affect the timers.
    using timer_ticks = std::chrono::duration<int64_t, std::ratio<1, 60>>;
    auto ticks = std::chrono::duration_cast<timer_ticks>(timer_elapsed);
    if (ticks.count() == 0) {
        return;
    }
    timer_elapsed -= ticks;

    if (ticks.count() >= delay_timer) {
        delay_timer = 0;
    } else {
        delay_timer -= static_cast<uint8_t>(ticks.count());
    }

    if (ticks.count() >= sound_timer) {
        sound_timer = 0;
    } else {
        sound_timer -= static_cast<uint8_t>(ticks.count());
    }
}

void chip8::setKey(uint8_t key, bool pressed) {
    if (key >= keys.size()) {
        throw std::out_of_range("Key must be between 0 and 15.");
    }
    keys[key] = pressed;
}

const std::array<std::array<uint32_t, 64>, 32>& chip8::getVideoOutput() const {
    return video_output;
}

bool chip8::isSoundActive() const {
    return sound_timer > 0;
}

void chip8::checkMemoryRange(uint16_t address, std::size_t count) const {
    if (address > ram.size() || count > ram.size() - address) {
        throw std::out_of_range("RAM access is out of bounds.");
    }
}

void chip8::opcode_NULL() {
}

void chip8::Table0() {
    if (opcode == 0x00E0u || opcode == 0x00EEu) {
        (this->*table0.at(opcode & 0x000Fu))();
    } else {
        opcode_NULL();
    }
}

void chip8::Table8() {
    (this->*table8.at(opcode & 0x000Fu))();
}

void chip8::TableE() {
    uint8_t byte = opcode & 0x00FFu;
    if (byte == 0x9Eu || byte == 0xA1u) {
        (this->*tableE.at(opcode & 0x000Fu))();
    } else {
        opcode_NULL();
    }
}

void chip8::TableF() {
    (this->*tableF.at(opcode & 0x00FFu))();
}

void chip8::opcode_00E0() {
    video_output = {};
}

void chip8::opcode_00EE() {
    if (stack_pointer == 0) {
        throw std::runtime_error("Stack underflow.");
    }
    stack_pointer--;
    program_counter = stack[stack_pointer];
}

void chip8::opcode_1NNN() {
    program_counter = opcode & 0x0FFFu;
}

void chip8::opcode_2NNN() {
    if (stack_pointer >= stack.size()) {
        throw std::runtime_error("Stack overflow.");
    }
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
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint16_t sum = registers[x] + registers[y];
    registers[x] = static_cast<uint8_t>(sum);
    if (sum > 0xFFu) {
        registers[15] = 1; 
    } else {
        registers[15] = 0;
    }
}

void chip8::opcode_8XY5() {
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t flag = registers[x] >= registers[y];
    registers[x] -= registers[y];
    registers[15] = flag;
}

void chip8::opcode_8XY6() {
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t flag = registers[x] & 0b00000001u;
    registers[x] >>= 1;
    registers[15] = flag;
}

void chip8::opcode_8XY7() {
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t flag = registers[y] >= registers[x];
    registers[x] = registers[y] - registers[x];
    registers[15] = flag;
}

void chip8::opcode_8XYE() {
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t flag = (registers[x] & 0b10000000u) >> 7u;
    registers[x] <<= 1;
    registers[15] = flag;
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

void chip8::opcode_DXYN()  {
    // Wrap the starting coordinates; clip the sprite at the screen edges.
    uint8_t x = registers[(opcode & 0x0F00u) >> 8] % 64;
    uint8_t y = registers[(opcode & 0x00F0u) >> 4] % 32;
    uint8_t n = opcode & 0x000Fu;
    uint8_t rows = std::min<uint8_t>(n, 32 - y);
    checkMemoryRange(index_register, rows);
    registers[15] = 0;
    for (int i{0}; i < rows; ++i) {
        uint8_t sprite = ram[index_register + i]; 
        for (int j{0}; j < 8 && x + j < 64; j++) {
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
    if (key >= keys.size()) {
        throw std::out_of_range("Key must be between 0 and 15.");
    }
    if (keys[key]) {
        program_counter += 2;
    }
}

void chip8::opcode_EXA1() {
    uint8_t key = registers[(opcode & 0x0F00u) >> 8]; 
    if (key >= keys.size()) {
        throw std::out_of_range("Key must be between 0 and 15.");
    }
    if (!keys[key]) {
        program_counter += 2;
    }  
}

void chip8::opcode_FX07() {
    registers[(opcode & 0X0F00u) >> 8] = delay_timer;
}

void chip8::opcode_FX0A() {
    for (uint8_t i{}; i < keys.size(); ++i) {
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
    checkMemoryRange(index_register, 3);
    uint16_t temp_number = registers[(opcode & 0x0F00u) >> 8];
    ram[index_register] = temp_number / 100;
    ram[index_register + 1] = (temp_number / 10) % 10;
    ram[index_register + 2] = temp_number % 10; 
}

void chip8::opcode_FX55() {
    checkMemoryRange(index_register, ((opcode & 0x0F00u) >> 8) + 1);
    for (uint8_t i{}; i <= ((opcode & 0x0F00u) >> 8); ++i) {
        ram[index_register + i] = registers[i];
    }
}

void chip8::opcode_FX65() {
    checkMemoryRange(index_register, ((opcode & 0x0F00u) >> 8) + 1);
    for (uint8_t i{}; i <= ((opcode & 0x0F00u) >> 8); ++i) {
        registers[i] = ram[index_register + i];
    }
}
