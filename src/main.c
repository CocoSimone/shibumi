#include <emu.h>
#include <parallelRDP_wrapper.h>

int main(int argc, char* argv[]) {
  emu = (emu_t*)calloc(1, sizeof(emu_t));
  init_emu();

  if(argc > 1) {
    emu->core.running = load_rom(&emu->core.mem, argv[1]);
  }

  load_parallel_rdp();

  emu_run();
  destroy_emu();
  free(emu);

  return 0;
}
