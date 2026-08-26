#include <chrono>
#include <iostream>
#include "../../core/src/Chip8.h"
#include "SDL3/SDL.h"

int main(int argc, char** argv){
    std::string path;
    if (argc > 1) {
        path = argv[1];
    }
    else {
        std::getline(std::cin, path);
    }


    Chip8 chip8 =  Chip8();
    chip8.Load_ROM(path.c_str());

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Chip8", 640, 320, SDL_WINDOW_RESIZABLE, &window, &renderer);
    bool quit = false;
    SDL_Event event{};
    SDL_Palette *palette = SDL_CreatePalette(2);
    SDL_Color const colors[2] = {
        {0,   0,   0,   255}, // 0 -> black
        {255, 255, 255, 255}  // 1 -> white
    };
    SDL_SetPaletteColors(palette, colors, 0, 2);


    // Buffer for converted pixels (64x32 RGBA)
    uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                    case SDLK_1: chip8.keyboard[0x1] = 1;
                        break;
                    case SDLK_2: chip8.keyboard[0x2] = 1;
                        break;
                    case SDLK_3: chip8.keyboard[0x3] = 1;
                        break;
                    case SDLK_4: chip8.keyboard[0xC] = 1;
                        break;
                    case SDLK_Q: chip8.keyboard[0x4] = 1;
                        break;
                    case SDLK_W: chip8.keyboard[0x5] = 1;
                        break;
                    case SDLK_E: chip8.keyboard[0x6] = 1;
                        break;
                    case SDLK_R: chip8.keyboard[0xD] = 1;
                        break;
                    case SDLK_A: chip8.keyboard[0x7] = 1;
                        break;
                    case SDLK_S: chip8.keyboard[0x8] = 1;
                        break;
                    case SDLK_D: chip8.keyboard[0x9] = 1;
                        break;
                    case SDLK_F: chip8.keyboard[0xE] = 1;
                        break;
                    case SDLK_Z: chip8.keyboard[0xA] = 1;
                        break;
                    case SDLK_X: chip8.keyboard[0x0] = 1;
                        break;
                    case SDLK_C: chip8.keyboard[0xB] = 1;
                        break;
                    case SDLK_V: chip8.keyboard[0xF] = 1;
                        break;
                    default: break;
                }
            } else if (event.type == SDL_EVENT_KEY_UP) {
                switch (event.key.key) {
                    case SDLK_1: chip8.keyboard[0x1] = 0;
                        break;
                    case SDLK_2: chip8.keyboard[0x2] = 0;
                        break;
                    case SDLK_3: chip8.keyboard[0x3] = 0;
                        break;
                    case SDLK_4: chip8.keyboard[0xC] = 0;
                        break;
                    case SDLK_Q: chip8.keyboard[0x4] = 0;
                        break;
                    case SDLK_W: chip8.keyboard[0x5] = 0;
                        break;
                    case SDLK_E: chip8.keyboard[0x6] = 0;
                        break;
                    case SDLK_R: chip8.keyboard[0xD] = 0;
                        break;
                    case SDLK_A: chip8.keyboard[0x7] = 0;
                        break;
                    case SDLK_S: chip8.keyboard[0x8] = 0;
                        break;
                    case SDLK_D: chip8.keyboard[0x9] = 0;
                        break;
                    case SDLK_F: chip8.keyboard[0xE] = 0;
                        break;
                    case SDLK_Z: chip8.keyboard[0xA] = 0;
                        break;
                    case SDLK_X: chip8.keyboard[0x0] = 0;
                        break;
                    case SDLK_C: chip8.keyboard[0xB] = 0;
                        break;
                    case SDLK_V: chip8.keyboard[0xF] = 0;
                        break;
                    default: break;
                }
            }
        }

        for (int i = 0; i < 10; i++) {
            chip8.Cycle();
        }

        // Convert 1bpp display buffer to 32-bit ARGB (branchless)
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                // Each byte holds 8 pixels, MSB first
                int byteIndex = y * (DISPLAY_WIDTH / 8) + (x / 8);
                int bitIndex = 7 - (x % 8);  // MSB first
                uint32_t pixelOn = (chip8.display[byteIndex] >> bitIndex) & 1;
                pixels[y * DISPLAY_WIDTH + x] = pixelOn ? 0xFFFFFFFF : 0xFF000000;
            }
        }

        // Create surface from converted pixels
        SDL_Surface *surface = SDL_CreateSurfaceFrom(DISPLAY_WIDTH, DISPLAY_HEIGHT,
            SDL_PIXELFORMAT_ARGB8888, pixels, DISPLAY_WIDTH * sizeof(uint32_t));
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer); // Update the screen with rendering results

        if (texture) {
            SDL_DestroyTexture(texture);
        }
        if (surface) {
            SDL_DestroySurface(surface);
        }

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
