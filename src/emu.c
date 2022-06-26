#include <emu.h>
#include <mem.h>
#include <audio.h>
#include <log.h>
#include <volk.h>
#include <parallelRDP_wrapper.h>

emu_t* emu = NULL;

void destroy_emu() {
  SDL_DestroyTexture(emu->texture);
  SDL_DestroyRenderer(emu->renderer);
  SDL_DestroyWindow(emu->window);
  SDL_Quit();
  NFD_Quit();
  destroy_core(&emu->core);
}

void init_emu() {
  init_core(&emu->core);
  emu->currentW = 320;
  emu->currentH = 240;
  emu->sdlFormat = SDL_PIXELFORMAT_RGBA5551;
  emu->currentFormat = f5553;

  SDL_Init(SDL_INIT_EVERYTHING);
  init_audio();
  emu->gamepad = NULL;
  emu->has_gamepad = false;

  emu->window = SDL_CreateWindow(
    "shibumi",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    W,
    H,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
  );

  if(volkInitialize() != VK_SUCCESS) {
    logfatal("Failed to initialize volk\n");
  }

  NFD_Init();
}

#define GET_BUTTON(gamepad, i) SDL_GameControllerGetButton(gamepad, i)
#define GET_AXIS(gamepad, axis) SDL_GameControllerGetAxis(gamepad, axis)

INLINE int clamp(int val, int min, int max) {
  if(val > max) {
    return max;
  } else if (val < min) {
    return min;
  }
  return val;
}

void poll_controller_gamepad(SDL_GameController* gamepad, controller_t* controller) {
  controller->b1 = 0;
  controller->b2 = 0;
  controller->b3 = 0;
  controller->b4 = 0;

  bool A = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_A);
  bool B = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_X);
  bool Z = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) == 32767;
  bool START = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_START);
  bool DUP = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP);
  bool DDOWN = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
  bool DLEFT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
  bool DRIGHT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
  bool L = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  bool R = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  bool CUP = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) == 32767;
  bool CDOWN = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) == -32768;
  bool CLEFT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) == -32768;
  bool CRIGHT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) == 32767;

  controller->b1 = (A << 7) | (B << 6) | (Z << 5) | (START << 4) |
                  (DUP << 3) | (DDOWN << 2) | (DLEFT << 1) | DRIGHT;

  controller->b2 = ((START && L && R) << 7) | (0 << 6) | (L << 5) | (R << 4) |
                  (CUP << 3) | (CDOWN << 2) | (CLEFT << 1) | CRIGHT;

  s8 xaxis = (s8)clamp((GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTX) >> 8), -127, 127);
  s8 yaxis = (s8)clamp(-(GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTY) >> 8), -127, 127);

  controller->b3 = xaxis;
  controller->b4 = yaxis;

  if((controller->b2 >> 7) & 1) {
    controller->b1 &= ~0x10;
    controller->b3 = 0;
    controller->b4 = 0;
  }
}

void poll_controller_kbm(controller_t* controller, const u8* state) {
  controller->b1 = 0;
  controller->b2 = 0;
  controller->b3 = 0;
  controller->b4 = 0;
  controller->b1 =
    (state[SDL_SCANCODE_X] << 7) |
    (state[SDL_SCANCODE_C] << 6) |
    (state[SDL_SCANCODE_Z] << 5) |
    (state[SDL_SCANCODE_RETURN] << 4) |
    (state[SDL_SCANCODE_KP_8] << 3) |
    (state[SDL_SCANCODE_KP_5] << 2) |
    (state[SDL_SCANCODE_KP_4] << 1) |
    (state[SDL_SCANCODE_KP_6]);
  controller->b2 =
    ((state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S]) << 7) |
    (0 << 6) |
    (state[SDL_SCANCODE_A] << 5) |
    (state[SDL_SCANCODE_S] << 4) |
    (state[SDL_SCANCODE_I] << 3) |
    (state[SDL_SCANCODE_J] << 2) |
    (state[SDL_SCANCODE_K] << 1) |
    (state[SDL_SCANCODE_L]);

  s8 xaxis = state[SDL_SCANCODE_LEFT] ? -128 : (state[SDL_SCANCODE_RIGHT] ? 127 : 0);
  s8 yaxis = state[SDL_SCANCODE_DOWN] ? -128 : (state[SDL_SCANCODE_UP] ? 127 : 0);

  controller->b3 = xaxis;
  controller->b4 = yaxis;

  if((controller->b2 >> 7) & 1) {
    controller->b1 &= ~0x10;
    controller->b3 = 0;
    controller->b4 = 0;
  }
}

void emu_poll_input() {
  SDL_Event e;
  core_t* core = &emu->core;
  controller_t* controller = &core->mem.mmio.si.controller;
  const u8* state = SDL_GetKeyboardState(NULL);
  while(SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_CONTROLLERDEVICEADDED: {
        const int index = e.cdevice.which;
        emu->gamepad = SDL_GameControllerOpen(index);
        emu->has_gamepad = true;
      } break;
      case SDL_CONTROLLERDEVICEREMOVED:
        SDL_GameControllerClose(emu->gamepad);
        emu->has_gamepad = false;
        break;
      case SDL_QUIT:
        emu->running = false;
        break;
      case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
          case SDLK_o: {
            nfdfilteritem_t filter = {"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
            nfdresult_t result = NFD_OpenDialog(&emu->romFile, &filter, 1, ".");
            if (result == NFD_OKAY) {
              core->running = false;
              init_core(core);
              core->running = load_rom(&core->mem, emu->romFile);
            }
          } break;
        } break;
    }
  }

  if(emu->has_gamepad) {
    poll_controller_gamepad(emu->gamepad, controller);
  } else {
    poll_controller_kbm(controller, state);
  }
}

void emu_run() {
  core_t* core = &emu->core;

  while(!emu->running) {
    update_screen_parallel_rdp_no_game();
    emu_poll_input();
  }

  while(emu->running) {
    if(core->running) {
      run_frame(core);
      update_screen_parallel_rdp();
    }

    emu_poll_input();
  }
}
