#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

class Platform {
public:
Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
~Platform();

Platform(const Platform&) = delete;
Platform& operator=(const Platform&) = delete;

void Update(const void* buffer, int pitch);
// keys points to a 16-entry keypad array; true means quit was requested.
bool ProcessInput(uint8_t* keys);

private:
SDL_Window* window{};
SDL_Renderer* renderer{};
SDL_Texture* texture{};
};
