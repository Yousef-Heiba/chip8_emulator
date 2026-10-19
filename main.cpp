#include "chip8.h"
#include "platform.h"

#include <array>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay in ms> <ROM>\n";
        return 1;
    }

    try {
        std::size_t scale_end{};
        std::size_t delay_end{};
        int video_scale = std::stoi(argv[1], &scale_end);
        int cycle_delay = std::stoi(argv[2], &delay_end);
        if (argv[1][scale_end] != '\0' || argv[2][delay_end] != '\0' ||
            video_scale <= 0 || video_scale > std::numeric_limits<int>::max() / 64 || cycle_delay < 0) {
            std::cerr << "Scale must be a positive integer and delay must be a nonnegative integer.\n";
            return 1;
        }

        constexpr int video_width{64};
        constexpr int video_height{32};

        chip8 emulator;
        auto result = emulator.loadROM(argv[3]);
        if (!result) {
            std::cerr << "Could not load ROM: " << result.error() << '\n';
            return 1;
        }

        Platform platform("CHIP-8 Emulator", video_width * video_scale, video_height * video_scale,
                          video_width, video_height);
        std::array<uint8_t, 16> keys{};
        const auto& video = emulator.getVideoOutput();
        int video_pitch = static_cast<int>(sizeof(video[0]));

        auto last_cycle_time = std::chrono::steady_clock::now();
        bool quit = false;
        while (!quit) {
            quit = platform.ProcessInput(keys.data());
            if (quit) {
                break;
            }
            for (uint8_t i{}; i < keys.size(); ++i) {
                emulator.setKey(i, keys[i] != 0);
            }

            // Keep the 60 Hz timers running even between instruction cycles.
            emulator.updateTimers();
            auto current_time = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> elapsed = current_time - last_cycle_time;
            if (elapsed.count() >= cycle_delay) {
                last_cycle_time = current_time;
                emulator.cycle();
                platform.Update(video.data(), video_pitch);
            }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
