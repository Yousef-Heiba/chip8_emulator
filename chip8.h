#pragma once

#include <cstdint> 
#include <fstream>
#include <string>
#include <expected>
#include <array>

class chip8 {
private:

public:

static constexpr uint16_t memory_start_address {0x200};

chip8() = default;
~chip8() = default;

std::array<uint8_t, 16> registers{};
std::array <uint8_t, 4096> ram{};

uint16_t index_register{};
uint16_t program_counter{/*memory_start_address*/};

std::array<uint16_t, 16> stack{};
uint8_t stack_pointer{};

uint8_t delay_timer{};
uint8_t sound_timer{};

std::array<uint8_t, 16> keys{};

std::array<std::array<uint32_t, 64>, 32> video_output{};

uint16_t opcode{};

std::expected<void, std::string> loadROM (const std::string &filename);
};


//prob want a constructor to intialize the rom and one for the fonts or one that does all (decide which)
chip8::chip8 () {
    program_counter = memory_start_address;
}