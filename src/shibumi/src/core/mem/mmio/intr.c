#include <intr.h>
#include <registers.h>
#include <mi.h>

void update_interrupt(mi_t* mi, registers_t* regs) {
  bool interrupt = mi->mi_intr.raw & mi->mi_intr_mask.raw;
  regs->cp0.Cause.ip2 = interrupt;
}

void interrupt_raise(mi_t* mi, registers_t* regs, interrupt_t intr) {
  switch(intr) {
    case VI:
      mi->mi_intr.vi = true;
      break;
    case SI:
      mi->mi_intr.si = true;
      break;
    case PI:
      mi->mi_intr.pi = true;
      break;
    case AI:
      mi->mi_intr.ai = true;
      break;
    case DP:
      mi->mi_intr.dp = true;
      break;
    case SP:
      mi->mi_intr.sp = true;
      break;
  }

  update_interrupt(mi, regs);
}

void interrupt_lower(mi_t* mi, registers_t* regs, interrupt_t intr) {
  switch(intr) {
    case VI:
      mi->mi_intr.vi = false;
      break;
    case SI:
      mi->mi_intr.si = false;
      break;
    case PI:
      mi->mi_intr.pi = false;
      break;
    case AI:
      mi->mi_intr.ai = false;
      break;
    case DP:
      mi->mi_intr.dp = false;
      break;
    case SP:
      mi->mi_intr.sp = false;
      break;
  }

  update_interrupt(mi, regs);
}
