#include <rsp_decode_instr.h>
#include <log.h>

INLINE void rsp_special(rsp_t* rsp, u32 instr) {
  u8 mask = instr & 0x3f;
  switch(mask) {
    case 0x00: rsp_sll(rsp, instr); break;
    case 0x04: rsp_sllv(rsp, instr); break;
    case 0x08: rsp_jr(rsp, instr); break;
    case 0x0C:
    case 0x0D:
      rsp->sp_status.halt = true;
      rsp->sp_status.broke = true;
      break;
    case 0x20: case 0x21:
      rsp_add(rsp, instr);
      break;
    case 0x24: rsp_and_(rsp, instr); break;
    case 0x25: rsp_or_(rsp, instr); break;
    case 0x27: rsp_nor(rsp, instr); break;
    default: logfatal("Unhandled RSP special instruction %d %d\n", (mask >> 3) & 7, mask & 7);
  }
}

INLINE void rsp_regimm(rsp_t* rsp, u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  switch(mask) {
    case 0x00: rsp_b(rsp, instr, (s32)rsp->gpr[RS(instr)] < 0); break;
    case 0x01: rsp_b(rsp, instr, (s32)rsp->gpr[RS(instr)] >= 0); break;
    default: logfatal("Unhandled RSP regimm instruction %d %d\n", (mask >> 3) & 3, mask & 7);
  }
}

INLINE void lwc2(rsp_t* rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    case 0x04: rsp_lqv(rsp, instr); break;
    default: logfatal("Unhandled RSP LWC2 %d %d\n", (mask >> 3) & 3, mask & 7);
  }
}

INLINE void swc2(rsp_t* rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    case 0x04: rsp_sqv(rsp, instr); break;
    default: logfatal("Unhandled RSP SWC2 %d %d\n", (mask >> 3) & 3, mask & 7);
  }
}

INLINE void cop2(rsp_t* rsp, u32 instr) {
  u8 mask = instr & 0x3F;
  u8 mask_sub = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00:
      switch(mask_sub) {
        case 0x02: rsp_cfc2(rsp, instr); break;
        default: logfatal("Unhandled RSP COP2 sub %d %d\n", (mask_sub >> 3) & 3, mask_sub & 3);
      }
      break;
    case 0x13: rsp_vabs(rsp, instr); break;
    case 0x1D: rsp_vsar(rsp, instr); break;
    case 0x21: rsp_veq(rsp, instr); break;
    case 0x22: rsp_vne(rsp, instr); break;
    case 0x33: rsp_vmov(rsp, instr); break;
    default: logfatal("Unhandled RSP COP2 %d %d\n", (mask >> 3) & 7, mask & 7);
  }
}

INLINE void cop0(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp, u32 instr) {
  u8 mask = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00: rsp_mfc0(rsp, rdp, instr); break;
    case 0x04: rsp_mtc0(mi, regs, rsp, rdp, instr); break;
    default: logfatal("Unhandled RSP COP0 %d %d\n", (mask >> 3) & 3, mask & 7);
  }
}

void rsp_exec(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp, u32 instr) {
  u8 mask = (instr >> 26) & 0x3F;
  switch(mask) {
    case 0x00: rsp_special(rsp, instr); break;
    case 0x01: rsp_regimm(rsp, instr); break;
    case 0x02: rsp_j(rsp, instr); break;
    case 0x03: rsp_jal(rsp, instr); break;
    case 0x04: rsp_b(rsp, instr, rsp->gpr[RT(instr)] == rsp->gpr[RS(instr)]); break;
    case 0x05: rsp_b(rsp, instr, rsp->gpr[RT(instr)] != rsp->gpr[RS(instr)]); break;
    case 0x07: rsp_b(rsp, instr, rsp->gpr[RS(instr)] > 0); break;
    case 0x08: case 0x09:
      rsp_addi(rsp, instr);
      break;
    case 0x0C: rsp_andi(rsp, instr); break;
    case 0x0D: rsp_ori(rsp, instr); break;
    case 0x0F: rsp_lui(rsp, instr); break;
    case 0x10: cop0(mi, regs, rsp, rdp, instr); break;
    case 0x12: cop2(rsp, instr); break;
    case 0x21: rsp_lh(rsp, instr); break;
    case 0x23: rsp_lw(rsp, instr); break;
    case 0x28: rsp_sb(rsp, instr); break;
    case 0x29: rsp_sh(rsp, instr); break;
    case 0x2B: rsp_sw(rsp, instr); break;
    case 0x32: lwc2(rsp, instr); break;
    case 0x3A: swc2(rsp, instr); break;
    default: logfatal("Unhandled RSP instruction %d %d\n", (mask >> 3) & 7, mask & 7);
  }
}