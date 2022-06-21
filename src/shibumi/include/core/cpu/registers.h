#pragma once
#include <cop0.h>
#include <cop1.h>
#include <stdbool.h>

typedef struct registers_t {
  s64 gpr[32];
  cop1_t cp1;
  cop0_t cp0;
  s64 old_pc, pc, next_pc;
  s64 hi, lo;
  bool LLBit;
  bool prev_delay_slot, delay_slot;
} registers_t;

void init_registers(registers_t* regs);
void set_pc(registers_t* regs, s64 value);
