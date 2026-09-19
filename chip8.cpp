#include <chip8.h>

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