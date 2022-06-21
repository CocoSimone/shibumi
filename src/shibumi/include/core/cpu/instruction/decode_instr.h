#pragma once
#include <instructions.h>

typedef struct cpu_t cpu_t;

void exec(cpu_t* cpu, mem_t* mem, u32 instr);