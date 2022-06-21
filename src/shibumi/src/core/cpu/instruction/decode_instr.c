#include <instruction/decode_instr.h>
#include <instruction/fpu_decode_instr.h>
#include <instruction/cop0_decode_instr.h>
#include <log.h>
#include <access.h>
#include <cpu.h>

INLINE void special(cpu_t* cpu, mem_t* mem, u32 instr) {
  registers_t* regs = &cpu->regs;
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        sll(regs, instr);
      }
      break;
    case 0x02: srl(regs, instr); break;
    case 0x03: sra(regs, instr); break;
    case 0x04: sllv(regs, instr); break;
    case 0x06: srlv(regs, instr); break;
    case 0x07: srav(regs, instr); break;
    case 0x08: jr(cpu, instr); break;
    case 0x09: jalr(cpu, instr); break;
    case 0x0C: fire_exception(regs, Syscall, 0, regs->old_pc); break;
    case 0x0D: fire_exception(regs, Breakpoint, 0, regs->old_pc); break;
    case 0x0F: break;
    case 0x10: mfhi(regs, instr); break;
    case 0x11: mthi(regs, instr); break;
    case 0x12: mflo(regs, instr); break;
    case 0x13: mtlo(regs, instr); break;
    case 0x14: dsllv(regs, instr); break;
    case 0x16: dsrlv(regs, instr); break;
    case 0x17: dsrav(regs, instr); break;
    case 0x18: mult(regs, instr); break;
    case 0x19: multu(regs, instr); break;
    case 0x1A: div_(regs, instr); break;
    case 0x1B: divu(regs, instr); break;
    case 0x1C: dmult(regs, instr); break;
    case 0x1D: dmultu(regs, instr); break;
    case 0x1E: ddiv(regs, instr); break;
    case 0x1F: ddivu(regs, instr); break;
    case 0x20: add(regs, instr); break;
    case 0x21: addu(regs, instr); break;
    case 0x22: sub(regs, instr); break;
    case 0x23: subu(regs, instr); break;
    case 0x24: and_(regs, instr); break;
    case 0x25: or_(regs, instr); break;
    case 0x26: xor_(regs, instr); break;
    case 0x27: nor(regs, instr); break;
    case 0x2A: slt(regs, instr); break;
    case 0x2B: sltu(regs, instr); break;
    case 0x2C: dadd(regs, instr); break;
    case 0x2D: daddu(regs, instr); break;
    case 0x2E: dsub(regs, instr); break;
    case 0x2F: dsubu(regs, instr); break;
    case 0x34: trap(regs, regs->gpr[RS(instr)] == regs->gpr[RT(instr)]); break;
    case 0x38: dsll(regs, instr); break;
    case 0x3A: dsrl(regs, instr); break;
    case 0x3B: dsra(regs, instr); break;
    case 0x3C: dsll32(regs, instr); break;
    case 0x3E: dsrl32(regs, instr); break;
    case 0x3F: dsra32(regs, instr); break;
    default:
      logfatal("Unimplemented special %d %d\n", (mask >> 3) & 7, mask & 7);
  }
}

INLINE void regimm(cpu_t* cpu, u32 instr) {
  registers_t* regs = &cpu->regs;
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: b(cpu, instr, regs->gpr[RS(instr)] < 0); break;
    case 0x01: b(cpu, instr, regs->gpr[RS(instr)] >= 0); break;
    case 0x02: bl(cpu, instr, regs->gpr[RS(instr)] < 0); break;
    case 0x03: bl(cpu, instr, regs->gpr[RS(instr)] >= 0); break;
    case 0x10: blink(cpu, instr, regs->gpr[RS(instr)] < 0); break;
    case 0x11: blink(cpu, instr, regs->gpr[RS(instr)] >= 0); break;
    case 0x12: bllink(cpu, instr, regs->gpr[RS(instr)] < 0); break;
    case 0x13: bllink(cpu, instr, regs->gpr[RS(instr)] >= 0); break;
    default:
      logfatal("Unimplemented regimm %d %d\n", (mask >> 3) & 3, mask & 7);
  }
}

void exec(cpu_t* cpu, mem_t* mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  registers_t* regs = &cpu->regs;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: special(cpu, mem, instr); break;
    case 0x01: regimm(cpu, instr); break;
    case 0x02: j(cpu, instr); break;
    case 0x03: jal(cpu, instr); break;
    case 0x04: b(cpu, instr, regs->gpr[RS(instr)] == regs->gpr[RT(instr)]); break;
    case 0x05: b(cpu, instr, regs->gpr[RS(instr)] != regs->gpr[RT(instr)]); break;
    case 0x06: b(cpu, instr, regs->gpr[RS(instr)] <= 0); break;
    case 0x07: b(cpu, instr, regs->gpr[RS(instr)] > 0); break;
    case 0x08: addi(regs, instr); break;
    case 0x09: addiu(regs, instr); break;
    case 0x0A: slti(regs, instr); break;
    case 0x0B: sltiu(regs, instr); break;
    case 0x0C: andi(regs, instr); break;
    case 0x0D: ori(regs, instr); break;
    case 0x0E: xori(regs, instr); break;
    case 0x0F: lui(regs, instr); break;
    case 0x10: cop0_decode(cpu, mem, instr); break;
    case 0x11: fpu_decode(cpu, instr); break;
    case 0x14: bl(cpu, instr, regs->gpr[RS(instr)] == regs->gpr[RT(instr)]); break;
    case 0x15: bl(cpu, instr, regs->gpr[RS(instr)] != regs->gpr[RT(instr)]); break;
    case 0x16: bl(cpu, instr, regs->gpr[RS(instr)] <= 0); break;
    case 0x17: bl(cpu, instr, regs->gpr[RS(instr)] > 0); break;
    case 0x18: daddi(regs, instr); break;
    case 0x19: daddiu(regs, instr); break;
    case 0x1A: ldl(mem, regs, instr); break;
    case 0x1B: ldr(mem, regs, instr); break;
    case 0x1F: fire_exception(regs, ReservedInstruction, 0, regs->old_pc); break;
    case 0x20: lb(mem, regs, instr); break;
    case 0x21: lh(mem, regs, instr); break;
    case 0x22: lwl(mem, regs, instr); break;
    case 0x23: lw(mem, regs, instr); break;
    case 0x24: lbu(mem, regs, instr); break;
    case 0x25: lhu(mem, regs, instr); break;
    case 0x26: lwr(mem, regs, instr); break;
    case 0x27: lwu(mem, regs, instr); break;
    case 0x28: sb(mem, regs, instr); break;
    case 0x29: sh(mem, regs, instr); break;
    case 0x2A: swl(mem, regs, instr); break;
    case 0x2B: sw(mem, regs, instr); break;
    case 0x2C: sdl(mem, regs, instr); break;
    case 0x2D: sdr(mem, regs, instr); break;
    case 0x2E: swr(mem, regs, instr); break;
    case 0x2F: break; // CACHE
    case 0x30: ll(mem, regs, instr); break;
    case 0x31: lwc1(regs, mem, instr); break;
    case 0x34: lld(mem, regs, instr); break;
    case 0x35: ldc1(regs, mem, instr); break;
    case 0x37: ld(mem, regs, instr); break;
    case 0x38: sc(mem, regs, instr); break;
    case 0x39: swc1(regs, mem, instr); break;
    case 0x3C: scd(mem, regs, instr); break;
    case 0x3D: sdc1(regs, mem, instr); break;
    case 0x3F: sd(mem, regs, instr); break;
    default:
      logfatal("Unimplemented instruction %d %d\n", (mask >> 3) & 7, mask & 7);
  }
}