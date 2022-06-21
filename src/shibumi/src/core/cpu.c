#include <cpu.h>
#include <instruction/decode_instr.h>
#include <utils.h>

INLINE bool should_service_interrupt(registers_t* regs) {
  bool interrupts_pending = (regs->cp0.Status.im & regs->cp0.Cause.interrupt_pending) != 0;
  bool interrupts_enabled = regs->cp0.Status.ie == 1;
  bool currently_handling_exception = regs->cp0.Status.exl == 1;
  bool currently_handling_error = regs->cp0.Status.erl == 1;

  return interrupts_pending && interrupts_enabled &&
         !currently_handling_exception && !currently_handling_error;
}

INLINE void check_compare_interrupt(mi_t* mi, registers_t* regs) {
  regs->cp0.Count++;
  regs->cp0.Count &= 0x1FFFFFFFF;
  if(regs->cp0.Count == (u64)regs->cp0.Compare << 1) {
    regs->cp0.Cause.ip7 = 1;
    update_interrupt(mi, regs);
  }
}

INLINE bool is_64bit_addressing(cop0_t cp0, u64 addr) {
  u8 region = (addr >> 62) & 3;
  switch(region) {
    case 0b00: return cp0.Status.ux;
    case 0b01: return cp0.Status.sx;
    case 0b11: return cp0.Status.kx;
    default: return false;
  }
}

void fire_exception(registers_t* regs, exception_code_t code, int cop, s64 pc) {
  bool old_exl = regs->cp0.Status.exl;

  if(!regs->cp0.Status.exl) {
    if(regs->prev_delay_slot) { // TODO: cached value of delay_slot should be used, but Namco Museum breaks!
      regs->cp0.Cause.branch_delay = true;
      pc -= 4;
    } else {
      regs->cp0.Cause.branch_delay = false;
    }

    regs->cp0.EPC = pc;
  }

  regs->cp0.Status.exl = true;
  regs->cp0.Cause.cop_error = cop;
  regs->cp0.Cause.exc_code = code;

  if(regs->cp0.Status.bev) {
    logfatal("BEV bit set!\n");
  } else {
    switch(code) {
      case Interrupt: case TLBModification:
      case AddressErrorLoad: case AddressErrorStore:
      case InstructionBusError: case DataBusError:
      case Syscall: case Breakpoint:
      case ReservedInstruction: case CoprocessorUnusable:
      case Overflow: case Trap:
      case FloatingPointError: case Watch:
        set_pc(regs, (s64)((s32)0x80000180));
        break;
      case TLBLoad: case TLBStore:
        if(old_exl || regs->cp0.TlbError == INVALID) {
          set_pc(regs, (s64)((s32)0x80000180));
        } else if(is_64bit_addressing(regs->cp0, regs->cp0.BadVaddr)) {
          set_pc(regs, (s64)((s32)0x80000080));
        } else {
          set_pc(regs, (s64)((s32)0x80000000));
        }
        break;
      default: logfatal("Unhandled exception! %d\n", code);
    }
  }
}

INLINE void handle_interrupt(cpu_t* cpu) {
  if(should_service_interrupt(&cpu->regs)) {
    fire_exception(&cpu->regs, Interrupt, 0, cpu->regs.pc);
  }
}

void init_cpu(cpu_t *cpu) {
  init_registers(&cpu->regs);
}

void step(cpu_t *cpu, mem_t *mem) {
  registers_t* regs = &cpu->regs;
  regs->gpr[0] = 0;

  regs->prev_delay_slot = regs->delay_slot;
  regs->delay_slot = false;

  check_compare_interrupt(&mem->mmio.mi, regs);

  u32 instruction = read32(mem, regs, regs->pc, regs->pc);
  handle_interrupt(cpu);

  regs->old_pc = regs->pc;
  regs->pc = regs->next_pc;
  regs->next_pc += 4;

  exec(cpu, mem, instruction);
}