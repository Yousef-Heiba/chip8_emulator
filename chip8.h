#pragma once

#include <cstdint> 
#include <fstream>
#include <string>
#include <expected>
#include <array>
#include <random>
#include <cassert>

class chip8 {
public:
chip8() = default;
~chip8() = default;

std::expected<void, std::string> loadROM (const std::string &filename);

private:

static constexpr uint16_t memory_start_address {0x200};
static constexpr uint8_t font_start_address {0x50};

std::mt19937 randGen;
std::uniform_int_distribution<unsigned int> randByte;

std::array<uint8_t, 16> registers{};
std::array<uint8_t, 4096> ram{};

uint16_t index_register{};
uint16_t program_counter{/*memory_start_address*/};

std::array<uint16_t, 16> stack{};
uint8_t stack_pointer{};

uint8_t delay_timer{};
uint8_t sound_timer{};

std::array<uint8_t, 16> keys{};
std::array<uint8_t, 80> font{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

std::array<std::array<uint32_t, 64>, 32> video_output{};

uint16_t opcode{};

void cycle();

void opcode_00E0();
void opcode_00EE();
void opcode_1NNN();
void opcode_2NNN();
void opcode_3XKK();
void opcode_4XKK();
void opcode_5XY0();
void opcode_6XKK();
void opcode_7XKK();
void opcode_8XY0();
void opcode_8XY1();
void opcode_8XY2();
void opcode_8XY3();
void opcode_8XY4();
void opcode_8XY5();
void opcode_8XY6();
void opcode_8XY7();
void opcode_8XYE();
void opcode_9XY0();
void opcode_ANNN();
void opcode_BNNN();
void opcode_CXKK();
void opcode_DXYN();
void opcode_EX9E();
void opcode_EXA1();
void opcode_FX07();
void opcode_FX0A();
void opcode_FX15();
void opcode_FX18();
void opcode_FX1E();
void opcode_FX29();
void opcode_FX33();
void opcode_FX55();
void opcode_FX65();

void (chip8::*opcodeDecoder[35])() = {
    opcode_00E0,
    opcode_00EE,
    opcode_1NNN,
    opcode_2NNN,
    opcode_3XKK,
    opcode_4XKK,
    opcode_5XY0,
    opcode_6XKK,
    opcode_7XKK,
    opcode_8XY0,
    opcode_8XY1,
    opcode_8XY2,
    opcode_8XY3,
    opcode_8XY5,
    opcode_8XY6,
    opcode_8XY7,
    opcode_8XYE,
    opcode_9XY0,
    opcode_ANNN,
    opcode_BNNN,
    opcode_CXKK,
    opcode_DXYN,
    opcode_EX9E,
    opcode_EXA1,
    opcode_FX07,
    opcode_FX0A,
    opcode_FX15,
    opcode_FX18,
    opcode_FX1E,
    opcode_FX29,
    opcode_FX33,
    opcode_FX55,
    opcode_FX65,
};
};