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