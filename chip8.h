#include <cstdint> 
#include <fstream>
#include <string>
#include <expected>
#include <array>

class chip8 {
private:

public:

const uint16_t memory_start_address {0x200};

chip8() = default;
~chip8() = default;

std::array<uint8_t, 16> registers{};
std::array <uint8_t, 4096> ram{};
uint16_t index_register{};
uint16_t program_counter{};
std::array<uint16_t, 16> stack{};
uint8_t stack_pointer{};
uint8_t delay_timer{};
uint8_t sound_timer{};
std::array<uint8_t, 16> keys{};
std::array<std::array<uint32_t, 64>, 32> video_output{};
uint16_t opcode{};

std::expected<void, std::string> loadROM (const std::string &filename);
};

std::expected<void, std::string> chip8::loadROM (const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return std::unexpected("File stream is in a failed state.");
    } else if (file.is_open()) {
        uint16_t rom_address{};
        char byte;
        while (file.get(byte)) {
            if ((memory_start_address + rom_address) >= ram.size()) {
                return std::unexpected("ROM is too large.");
            }
            ram[memory_start_address + rom_address] = static_cast<uint8_t> (byte);
            rom_address++;
        }
    }
    return {};
}
//prob want a constructor to intialize the rom and one for the fonts or one that does all (decide which)