#include <cstdint> 

class chip8 {
private:

public:
uint8_t registers[16]{};
uint8_t ram[4096]{};
uint16_t index_register{};
uint16_t program_counter{};
uint16_t stack[16]{};
uint8_t stack_pointer{};
uint8_t delay_timer{};
uint8_t sound_timer{};
uint8_t keys[16]{};
uint32_t video_output[2048]{};
uint16_t opcode{};
};