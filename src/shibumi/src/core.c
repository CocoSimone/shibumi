#include <core.h>
#include <utils.h>

void init_core(core_t* core) {
  init_cpu(&core->cpu);
  init_mem(&core->mem);
  core->running = false;
}

void destroy_core(core_t* core) {
  destroy_mem(&core->mem);
}

void run_frame(core_t* core) {
  mmio_t* mmio = &core->mem.mmio;
  for (mmio->vi.current = 0; mmio->vi.current < 262; mmio->vi.current++) {
    for(int i = 0; i < 6000; i++) {
      step(&core->cpu, &core->mem);
      step_rsp(&mmio->mi, &core->cpu.regs, &mmio->rsp, &mmio->rdp);
      step_rsp(&mmio->mi, &core->cpu.regs, &mmio->rsp, &mmio->rdp);
      step_rsp(&mmio->mi, &core->cpu.regs, &mmio->rsp, &mmio->rdp);
      step_rsp(&mmio->mi, &core->cpu.regs, &mmio->rsp, &mmio->rdp);
    }

    if((mmio->vi.current & 0x3FE) == mmio->vi.intr) {
      interrupt_raise(&mmio->mi, &core->cpu.regs, VI);
    }
  }
}