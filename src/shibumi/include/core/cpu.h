#pragma once
#include <registers.h>
#include <mem.h>

typedef struct cpu_t {
  registers_t regs;
} cpu_t;

typedef enum exception_code_t {
  Interrupt,
  TLBModification,
  TLBLoad,
  TLBStore,
  AddressErrorLoad,
  AddressErrorStore,
  InstructionBusError,
  DataBusError,
  Syscall,
  Breakpoint,
  ReservedInstruction,
  CoprocessorUnusable,
  Overflow,
  Trap,
  FloatingPointError = 15,
  Watch = 23
} exception_code_t;

void init_cpu(cpu_t* cpu);
void step(cpu_t* cpu, mem_t* mem);
void fire_exception(registers_t* regs, exception_code_t code, int cop, s64 pc);
