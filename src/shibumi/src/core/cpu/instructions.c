#include <instructions.h>
#include <utils.h>
#include <cpu.h>

#define se_imm(x) ((s16)((x) & 0xFFFF))

void add(registers_t* regs, u32 instr) {
  s32 rs = (s32)regs->gpr[RS(instr)];
  s32 rt = (s32)regs->gpr[RT(instr)];
  s32 result = rs + rt;
  regs->gpr[RD(instr)] = result;
}

void addu(registers_t* regs, u32 instr) {
  s32 rs = (s32)regs->gpr[RS(instr)];
  s32 rt = (s32)regs->gpr[RT(instr)];
  s32 result = rs + rt;
  regs->gpr[RD(instr)] = result;
}

void addi(registers_t* regs, u32 instr) {
  s32 rs = (s32)regs->gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs->gpr[RT(instr)] = result;
}

void addiu(registers_t* regs, u32 instr) {
  s32 rs = (s32)regs->gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs->gpr[RT(instr)] = result;
}

void dadd(registers_t* regs, u32 instr) {
  s64 rs = regs->gpr[RS(instr)];
  s64 rt = regs->gpr[RT(instr)];
  regs->gpr[RD(instr)] = rs + rt;
}

void daddu(registers_t* regs, u32 instr) {
  s64 rs = regs->gpr[RS(instr)];
  s64 rt = regs->gpr[RT(instr)];
  regs->gpr[RD(instr)] = rs + rt;
}

void daddi(registers_t* regs, u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs->gpr[RS(instr)];
  regs->gpr[RT(instr)] = rs + imm;
}

void daddiu(registers_t* regs, u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs->gpr[RS(instr)];
  regs->gpr[RT(instr)] = rs + imm;
}

void div_(registers_t* regs, u32 instr) {
  s64 dividend = (s32)regs->gpr[RS(instr)];
  s64 divisor = (s32)regs->gpr[RT(instr)];
  
  if(divisor == 0) {
    regs->hi = dividend;
    if(dividend >= 0) {
      regs->lo = -1;
    } else {
      regs->lo = 1;
    }
  } else {
    s64 quotient = dividend / divisor;
    s64 remainder = dividend % divisor;
    regs->lo = quotient;
    regs->hi = remainder;
  }
}

void divu(registers_t* regs, u32 instr) {
  u32 dividend = regs->gpr[RS(instr)];
  u32 divisor = regs->gpr[RT(instr)];
  if(divisor == 0) {
    regs->lo = -1;
    regs->hi = (s32)dividend;
  } else {
    s32 quotient = (s32)(dividend / divisor);
    s32 remainder = (s32)(dividend % divisor);
    regs->lo = quotient;
    regs->hi = remainder;
  }
}

void ddiv(registers_t* regs, u32 instr) {
  s64 dividend = regs->gpr[RS(instr)];
  s64 divisor = regs->gpr[RT(instr)];
  if (dividend == 0x8000000000000000 && divisor == 0xFFFFFFFFFFFFFFFF) {
    regs->lo = dividend;
    regs->hi = 0;
  } else if(divisor == 0) {
    regs->hi = dividend;
    if(dividend >= 0) {
      regs->lo = -1;
    } else {
      regs->lo = 1;
    }
  } else {
    s64 quotient = dividend / divisor;
    s64 remainder = dividend % divisor;
    regs->lo = quotient;
    regs->hi = remainder;
  }
}

void ddivu(registers_t* regs, u32 instr) {
  u64 dividend = regs->gpr[RS(instr)];
  u64 divisor = regs->gpr[RT(instr)];
  if(divisor == 0) {
    regs->lo = -1;
    regs->hi = (s64)dividend;
  } else {
    u64 quotient = dividend / divisor;
    u64 remainder = dividend % divisor;
    regs->lo = (s64)quotient;
    regs->hi = (s64)remainder;
  }
}

void branch(cpu_t* cpu, bool cond, s64 address) {
  registers_t* regs = &cpu->regs;
  regs->delay_slot = true;
  if (cond) {
    regs->next_pc = address;
  }
}

void branch_likely(cpu_t* cpu, bool cond, s64 address) {
  registers_t* regs = &cpu->regs;
  regs->delay_slot = true;
  if (cond) {
    regs->next_pc = address;
  } else {
    set_pc(regs, regs->next_pc);
  }
}

void b(cpu_t* cpu, u32 instr, bool cond) {
  registers_t* regs = &cpu->regs;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs->pc + offset;
  branch(cpu, cond, address);
}

void blink(cpu_t* cpu, u32 instr, bool cond) {
  registers_t* regs = &cpu->regs;
  regs->gpr[31] = regs->next_pc;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs->pc + offset;
  branch(cpu, cond, address);
}

void bl(cpu_t* cpu, u32 instr, bool cond) {
  registers_t* regs = &cpu->regs;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs->pc + offset;
  branch_likely(cpu, cond, address);
}

void bllink(cpu_t* cpu, u32 instr, bool cond) {
  registers_t* regs = &cpu->regs;
  regs->gpr[31] = regs->next_pc;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs->pc + offset;
  branch_likely(cpu, cond, address);
}

void lui(registers_t* regs, u32 instr) {
  s64 val = (s16)instr;
  val *= 65536;
  regs->gpr[RT(instr)] = val;
}

void lb(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  regs->gpr[RT(instr)] = (s64)(s8)read8(mem, regs, address, regs->old_pc);
}

void lh(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 1) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  regs->gpr[RT(instr)] = (s64)(s16)read16(mem, regs, address, regs->old_pc);
}

void lw(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  s32 value = read32(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void ll(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  regs->LLBit = true;
  regs->cp0.LLAddr = address;

  s32 value = read32(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void lwl(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 0) & 3);
  u32 mask = 0xFFFFFFFF << shift;
  u32 data = read32(mem, regs, address & ~3, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  s32 result = (s32)((rt & ~mask) | (data << shift));
  regs->gpr[RT(instr)] = result;
}

void lwr(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 3) & 3);
  u32 mask = 0xFFFFFFFF >> shift;
  u32 data = read32(mem, regs, address & ~3, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  s32 result = (s32)((rt & ~mask) | (data >> shift));
  regs->gpr[RT(instr)] = result;
}

void ld(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  s64 value = read64(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void lld(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  regs->LLBit = true;
  regs->cp0.LLAddr = address;

  s64 value = read64(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void ldl(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 0) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
  u64 data = read64(mem, regs, address & ~7, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  s64 result = (s64)((rt & ~mask) | (data << shift));
  regs->gpr[RT(instr)] = result;
}

void ldr(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 7) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
  u64 data = read64(mem, regs, address & ~7, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  s64 result = (s64)((rt & ~mask) | (data >> shift));
  regs->gpr[RT(instr)] = result;
}

void lbu(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  u8 value = read8(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void lhu(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 1) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  u16 value = read16(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void lwu(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorLoad, 0, regs->old_pc);
  }

  u32 value = read32(mem, regs, address, regs->old_pc);
  regs->gpr[RT(instr)] = value;
}

void sb(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  write8(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
}

void sc(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }
  
  if(regs->LLBit) {
    write32(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
  }

  regs->gpr[RT(instr)] = (s64)((u64)regs->LLBit);
  regs->LLBit = false;
}

void scd(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }
  
  if(regs->LLBit) {
    write64(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
  }

  regs->gpr[RT(instr)] = (s64)((u64)regs->LLBit);
  regs->LLBit = false;
}

void sh(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 1) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }

  write16(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
}

void sw(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }
  
  write32(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
}

void sd(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }
  
  write64(mem, regs, address, regs->gpr[RT(instr)], regs->old_pc);
}

void sdl(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 0) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
  u64 data = read64(mem, regs, address & ~7, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  write64(mem, regs, address & ~7, (data & ~mask) | (rt >> shift), regs->old_pc);
}

void sdr(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 7) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
  u64 data = read64(mem, regs, address & ~7, regs->old_pc);
  s64 rt = regs->gpr[RT(instr)];
  write64(mem, regs, address & ~7, (data & ~mask) | (rt << shift), regs->old_pc);
}

void swl(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 0) & 3);
  u32 mask = 0xFFFFFFFF >> shift;
  u32 data = read32(mem, regs, address & ~3, regs->old_pc);
  u32 rt = regs->gpr[RT(instr)];
  write32(mem, regs, address & ~3, (data & ~mask) | (rt >> shift), regs->old_pc);
}

void swr(mem_t* mem, registers_t* regs, u32 instr) {
  u32 address = regs->gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 3) & 3);
  u32 mask = 0xFFFFFFFF << shift;
  u32 data = read32(mem, regs, address & ~3, regs->old_pc);
  u32 rt = regs->gpr[RT(instr)];
  write32(mem, regs, address & ~3, (data & ~mask) | (rt << shift), regs->old_pc);
}

void ori(registers_t* regs, u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs->gpr[RS(instr)];
  regs->gpr[RT(instr)] = result;
}

void or_(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->gpr[RS(instr)] | regs->gpr[RT(instr)];
}

void nor(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = ~(regs->gpr[RS(instr)] | regs->gpr[RT(instr)]);
}

void j(cpu_t* cpu, u32 instr) {
  registers_t* regs = &cpu->regs;
  u32 target = (instr & 0x3ffffff) << 2;
  u32 address = (regs->old_pc & ~0xfffffff) | target;
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, DataBusError, 0, regs->old_pc);
  }

  branch(cpu, true, address);
}

void jal(cpu_t* cpu, u32 instr) {
  registers_t* regs = &cpu->regs;
  regs->gpr[31] = regs->next_pc;
  j(cpu, instr);
}

void jalr(cpu_t* cpu, u32 instr) {
  registers_t* regs = &cpu->regs;
  branch(cpu, true, regs->gpr[RS(instr)]);
  regs->gpr[RD(instr)] = regs->pc + 4;
}

void slti(registers_t* regs, u32 instr) {
  regs->gpr[RT(instr)] = regs->gpr[RS(instr)] < se_imm((s64)instr);
}

void sltiu(registers_t* regs, u32 instr) {
  regs->gpr[RT(instr)] = (u64)regs->gpr[RS(instr)] < se_imm((s64)instr);
}

void slt(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->gpr[RS(instr)] < regs->gpr[RT(instr)];
}

void sltu(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = (u64)regs->gpr[RS(instr)] < (u64)regs->gpr[RT(instr)];
}

void xori(registers_t* regs, u32 instr) {
  s64 imm = (u16)instr;
  regs->gpr[RT(instr)] = regs->gpr[RS(instr)] ^ imm;
}

void xor_(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->gpr[RT(instr)] ^ regs->gpr[RS(instr)];
}

void andi(registers_t* regs, u32 instr) {
  s64 imm = (u16)instr;
  regs->gpr[RT(instr)] = regs->gpr[RS(instr)] & imm;
}

void and_(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->gpr[RS(instr)] & regs->gpr[RT(instr)];
}

void sll(registers_t* regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = regs->gpr[RT(instr)] << sa;
  regs->gpr[RD(instr)] = (s64)result;
}

void sllv(registers_t* regs, u32 instr) {
  u8 sa = (regs->gpr[RS(instr)]) & 0x1F;
  u32 rt = regs->gpr[RT(instr)];
  s32 result = rt << sa;
  regs->gpr[RD(instr)] = (s64)result;
}

void dsll32(registers_t* regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs->gpr[RT(instr)] << (sa + 32);
  regs->gpr[RD(instr)] = result;
}

void dsll(registers_t* regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs->gpr[RT(instr)] << sa;
  regs->gpr[RD(instr)] = result;
}

void dsllv(registers_t* regs, u32 instr) {
  s64 sa = regs->gpr[RS(instr)] & 63;
  s64 result = regs->gpr[RT(instr)] << sa;
  regs->gpr[RD(instr)] = result;
}

void srl(registers_t* regs, u32 instr) {
  u32 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u32 result = rt >> sa;
  regs->gpr[RD(instr)] = (s32)result;
}

void srlv(registers_t* regs, u32 instr) {
  u8 sa = (regs->gpr[RS(instr)] & 0x1F);
  u32 rt = regs->gpr[RT(instr)];
  s32 result = rt >> sa;
  regs->gpr[RD(instr)] = (s64)result;
}

void dsrl(registers_t* regs, u32 instr) {
  u64 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> sa;
  regs->gpr[RD(instr)] = result;
}

void dsrlv(registers_t* regs, u32 instr) {
  u8 amount = (regs->gpr[RS(instr)] & 63);
  u64 rt = regs->gpr[RT(instr)];
  u64 result = rt >> amount;
  regs->gpr[RD(instr)] = result;
}

void dsrl32(registers_t* regs, u32 instr) {
  u64 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> (sa + 32);
  regs->gpr[RD(instr)] = result;
}

void sra(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = rt >> sa;
  regs->gpr[RD(instr)] = result;
}

void srav(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  s64 rs = regs->gpr[RS(instr)];
  u8 sa = rs & 0x1f;
  s32 result = rt >> sa;
  regs->gpr[RD(instr)] = result;
}

void dsra(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> sa;
  regs->gpr[RD(instr)] = result;
}

void dsrav(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  s64 rs = regs->gpr[RS(instr)];
  s64 sa = rs & 63;
  s64 result = rt >> sa;
  regs->gpr[RD(instr)] = result;
}

void dsra32(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> (sa + 32);
  regs->gpr[RD(instr)] = result;
}



void jr(cpu_t* cpu, u32 instr) {
  registers_t* regs = &cpu->regs;
  u32 address = regs->gpr[RS(instr)];
  if ((address & 3) != 0) {
    handle_tlb_exception(regs, (s64)((s32)address));
    fire_exception(regs, AddressErrorStore, 0, regs->old_pc);
  }
  
  branch(cpu, true, address);
}

void dsub(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  s64 rs = regs->gpr[RS(instr)];
  s64 result = rs - rt;
  regs->gpr[RD(instr)] = result;
}

void dsubu(registers_t* regs, u32 instr) {
  u64 rt = regs->gpr[RT(instr)];
  u64 rs = regs->gpr[RS(instr)];
  u64 result = rs - rt;
  regs->gpr[RD(instr)] = result;
}

void sub(registers_t* regs, u32 instr) {
  s32 rt = regs->gpr[RT(instr)];
  s32 rs = regs->gpr[RS(instr)];
  s32 result = rs - rt;
  regs->gpr[RD(instr)] = (s64)result;
}

void subu(registers_t* regs, u32 instr) {
  u32 rt = regs->gpr[RT(instr)];
  u32 rs = regs->gpr[RS(instr)];
  u32 result = rs - rt;
  regs->gpr[RD(instr)] = (s64)((s32)result);
}

void dmultu(registers_t* regs, u32 instr) {
  u64 rt = regs->gpr[RT(instr)];
  u64 rs = regs->gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs->lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs->hi = (s64)(result >> 64);
}

void dmult(registers_t* regs, u32 instr) {
  s64 rt = regs->gpr[RT(instr)];
  s64 rs = regs->gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs->lo = result & 0xFFFFFFFFFFFFFFFF;
  regs->hi = result >> 64;
}

void multu(registers_t* regs, u32 instr) {
  u32 rt = regs->gpr[RT(instr)];
  u32 rs = regs->gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs->lo = (s64)((s32)result);
  regs->hi = (s64)((s32)(result >> 32));
}

void mult(registers_t* regs, u32 instr) {
  s32 rt = regs->gpr[RT(instr)];
  s32 rs = regs->gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs->lo = (s64)((s32)result);
  regs->hi = (s64)((s32)(result >> 32));
}

void mflo(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->lo;
}

void mfhi(registers_t* regs, u32 instr) {
  regs->gpr[RD(instr)] = regs->hi;
}

void mtlo(registers_t* regs, u32 instr) {
  regs->lo = regs->gpr[RS(instr)];
}

void mthi(registers_t* regs, u32 instr) {
  regs->hi = regs->gpr[RS(instr)];
}

void trap(registers_t* regs, bool cond) {
  if(cond) {
    fire_exception(regs, Trap, 0, regs->old_pc);
  }
}
