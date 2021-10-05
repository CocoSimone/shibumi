#pragma once
#include <core/cpu.h>
#include <core/mem.h>
#include <disasm.h>
#include <stdatomic.h>

typedef struct {
  cpu_t cpu;
  mem_t mem;
	bool running;
} core_t;

void init_core(core_t* core);
void run_frame(core_t* core);