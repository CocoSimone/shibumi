#pragma once
#include <SDL2/SDL.h>
#include <core.h>
#include <nfd.h>
#define W 1024
#define H 768

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  u8 currentFormat;
  u32 currentW, currentH;
  SDL_PixelFormatEnum sdlFormat;
  core_t core;
  nfdchar_t* romFile;
  u8* framebuffer;
  SDL_GameController* gamepad;
  bool has_gamepad;
  bool running;
} emu_t;

extern emu_t* emu;

void destroy_emu();
void init_emu();
void emu_run();
void emu_present();
#ifdef __cplusplus
extern "C" {
#endif
void emu_poll_input();
#ifdef __cplusplus
};
#endif
