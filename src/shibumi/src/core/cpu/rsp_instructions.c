#include <rsp_instructions.h>
#include <access.h>
#include <log.h>

#define ELEMENT_INDEX(i) (7 - (i))
#define BYTE_INDEX(i) (15 - (i))

INLINE bool acquire_semaphore(rsp_t* rsp) {
  if(rsp->semaphore) {
    return true;
  } else {
    rsp->semaphore = true;
    return false;
  }
}

INLINE void release_semaphore(rsp_t* rsp) {
  rsp->semaphore = false;
}

INLINE u32 get_cop0_register(rsp_t* rsp, rdp_t* rdp, u8 index) {
  switch(index) {
    case 4: return rsp->sp_status.raw;
    case 5: return rsp->sp_status.dma_full;
    case 6: return 0;
    case 7: return acquire_semaphore(rsp);
    case 11: return rdp->dpc.status.raw;
    default: logfatal("Unhandled RSP COP0 register read at index %d\n", index);
  }
}

INLINE void set_cop0_register(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp, u8 index, u32 val) {
  switch(index) {
    case 0: rsp->sp_dma_sp_addr.raw = val; break;
    case 1: rsp->sp_dma_dram_addr.raw = val; break;
    case 2: rsp->sp_dma_rdlen.raw = val; break;
    case 3: rsp->sp_dma_wrlen.raw = val; break;
    case 4: rsp->sp_status.raw = val; break;
    case 7:
      if(val == 0) {
        release_semaphore(rsp);
      }
      break;
    case 8:
      rdp->dpc.start = val & 0xFFFFF8;
      rdp->dpc.current = rdp->dpc.start;
      break;
    case 9:
      rdp->dpc.end = val & 0xFFFFF8;
      rdp_run_command(mi, regs, rdp, rsp);
      break;
    case 11: dp_status_write(&rdp->dpc.status, val); break;
    default: logfatal("Unhandled RSP COP0 register write at index %d\n", index);
  }
}

INLINE vpr_t broadcast(vpr_t vt, int l0, int l1, int l2, int l3, int l4, int l5, int l6, int l7) {
  vpr_t vte;
  vte.element[ELEMENT_INDEX(0)] = vt.element[l0];
  vte.element[ELEMENT_INDEX(1)] = vt.element[l1];
  vte.element[ELEMENT_INDEX(2)] = vt.element[l2];
  vte.element[ELEMENT_INDEX(3)] = vt.element[l3];
  vte.element[ELEMENT_INDEX(4)] = vt.element[l4];
  vte.element[ELEMENT_INDEX(5)] = vt.element[l5];
  vte.element[ELEMENT_INDEX(6)] = vt.element[l6];
  vte.element[ELEMENT_INDEX(7)] = vt.element[l7];
  return vte;
}

INLINE vpr_t get_vte(vpr_t vt, u8 e) {
  vpr_t vte;
  switch(e & 0xf) {
    case 0 ... 1: return vt;
    case 2 ... 3:
      vte = broadcast(vt, e - 2, e - 2, e, e, e + 2, e + 2, e + 4, e + 4);
      break;
    case 4 ... 7:
      vte = broadcast(vt, e - 4, e - 4, e - 4, e - 4, e, e, e, e);
      break;
    case 8 ... 15: {
      int index = e - 8;
      for (int i = 0; i < 8; i++) {
        vte.element[i] = vt.element[index];
      }
    } break;
  }
  return vte;
}

RSP_INSTR(and_) {
  rsp->gpr[RD(instr)] = rsp->gpr[RS(instr)] & rsp->gpr[RT(instr)];
}

RSP_INSTR(andi) {
  rsp->gpr[RT(instr)] = rsp->gpr[RS(instr)] & (instr & 0xFFFF);
}

RSP_INSTR(add) {
  rsp->gpr[RD(instr)] = rsp->gpr[RS(instr)] + rsp->gpr[RT(instr)];
}

RSP_INSTR(addi) {
  s32 imm = (s32)((s16)(instr & 0xFFFF));
  rsp->gpr[RT(instr)] = (s32)(rsp->gpr[RS(instr)] + imm);
}

RSP_INSTR(vmov) {
  rsp->vpr[VD(instr)].element[DE(instr)] = rsp->vpr[VT(instr)].element[E(instr)];
}

RSP_INSTR(veq) {
  vpr_t* vs = &rsp->vpr[VS(instr)];
  vpr_t* vt = &rsp->vpr[VT(instr)];
  vpr_t* vd = &rsp->vpr[VD(instr)];
  vpr_t vte = get_vte(*vt, E(instr));
  for(int i = 0; i < 8; i++) {
    rsp->vcc.l.element[i] = ((rsp->vco.h.element[i] == 0) && vs->element[i] == vte.element[i]) ? 0xFFFF : 0;
    rsp->acc.l.element[i] = rsp->vcc.l.element[i] != 0 ? vs->element[i] : vte.element[i];
    vd->element[i] = rsp->acc.l.element[i];

    rsp->vcc.h.element[i] = 0;
    rsp->vco.h.element[i] = 0;
    rsp->vco.l.element[i] = 0;
  }
}

RSP_INSTR(vne) {
  vpr_t* vs = &rsp->vpr[VS(instr)];
  vpr_t* vt = &rsp->vpr[VT(instr)];
  vpr_t* vd = &rsp->vpr[VD(instr)];
  vpr_t vte = get_vte(*vt, E(instr));
  for(int i = 0; i < 8; i++) {
    rsp->vcc.l.element[i] = ((rsp->vco.h.element[i] != 0) || vs->element[i] != vte.element[i]) ? 0xFFFF : 0;
    rsp->acc.l.element[i] = rsp->vcc.l.element[i] != 0 ? vs->element[i] : vte.element[i];
    vd->element[i] = rsp->acc.l.element[i];

    rsp->vcc.h.element[i] = 0;
    rsp->vco.h.element[i] = 0;
    rsp->vco.l.element[i] = 0;
  }
}

RSP_INSTR(cfc2) {
  s16 val = 0;
  switch(RD(instr) & 3) {
    case 0:
      val = vco_as_u16(rsp);
      break;
    case 1:
      val = vcc_as_u16(rsp);
      break;
    case 2 ... 3:
      val = (s8)rsp->vce;
      break;
  }
  rsp->gpr[RT(instr)] = val;
}

RSP_INSTR(lh) {
  s16 offset = instr & 0xFFFF;
  u32 address = rsp->gpr[RS(instr)] + offset;
  rsp->gpr[RT(instr)] = (s16)raccess(16, rsp->dmem, address & 0xFFF);
}

RSP_INSTR(lw) {
  s32 offset = (s32)((s16)(instr & 0xFFFF));
  u32 address = rsp->gpr[RS(instr)] + offset;
  rsp->gpr[RT(instr)] = raccess(32, rsp->dmem, address & 0xFFF);
}

RSP_INSTR(sb) {
  s32 offset = (s32)((s16)(instr & 0xFFFF));
  u32 address = rsp->gpr[RS(instr)] + offset;
  rsp->dmem[address & 0xFFF] = rsp->gpr[RT(instr)];
}

RSP_INSTR(sh) {
  u16 imm = instr & 0xFFFF;
  s32 seimm = (s32)((s16)imm);
  u32 address = rsp->gpr[RS(instr)] + seimm;
  waccess(16, rsp->dmem, address & 0xFFF, rsp->gpr[RT(instr)]);
}

RSP_INSTR(sw) {
  u16 imm = instr & 0xFFFF;
  s32 seimm = (s32)((s16)imm);
  u32 address = rsp->gpr[RS(instr)] + seimm;
  waccess(32, rsp->dmem, address & 0xFFF, rsp->gpr[RT(instr)]);
}

RSP_INSTR(lui) {
  u16 imm = instr & 0xFFFF;
  s32 seimm = (s32)((s16)imm);
  rsp->gpr[RT(instr)] = seimm << 16;
}

RSP_INSTR(lqv) {
  u8 offset = instr & 0x7F;
  u32 address = (s32)((s8)((offset << 1) >> 1)) + rsp->gpr[RS(instr)];
  u8 e = E(instr);
  u32 end = (address & ~15) + 15;
  for(int i = 0; address + i <= end; i++) {
    rsp->vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 15)] = rsp->dmem[address + i];
  }
}

RSP_INSTR(sqv) {
  u8 offset = instr & 0x7F;
  u32 address = (s32)((s8)((offset << 1) >> 1)) + rsp->gpr[RS(instr)];
  u8 e = E(instr);
  u32 end = (address & ~15) + 15;
  for(int i = 0; address + i <= end; i++) {
    rsp->dmem[address + i] = rsp->vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 15)];
  }
}

RSP_INSTR(vabs) {
  vpr_t* vs = &rsp->vpr[VS(instr)];
  vpr_t* vt = &rsp->vpr[VT(instr)];
  vpr_t* vd = &rsp->vpr[VD(instr)];
  vpr_t vte = get_vte(*vt, E(instr));

  for(int i = 0; i < 8; i++) {
    if((s16)vt->element[ELEMENT_INDEX(i)] < 0) {
      if (vte.element[ELEMENT_INDEX(i)] == 0x8000) {
        vs->element[ELEMENT_INDEX(i)] = 0x7FFF;
        rsp->acc.l.element[ELEMENT_INDEX(i)] = 0x8000;
      } else {
        vd->element[ELEMENT_INDEX(i)] = -vte.selement[ELEMENT_INDEX(i)];
        rsp->acc.l.element[ELEMENT_INDEX(i)] = -vte.selement[ELEMENT_INDEX(i)];
      }
    } else if (vs->element[ELEMENT_INDEX(i)] == 0) {
      vd->element[ELEMENT_INDEX(i)] = 0x0000;
      rsp->acc.l.element[ELEMENT_INDEX(i)] = 0x0000;
    } else {
      vd->element[ELEMENT_INDEX(i)] = vte.element[ELEMENT_INDEX(i)];
      rsp->acc.l.element[ELEMENT_INDEX(i)] = vte.element[ELEMENT_INDEX(i)];
    }
  }
}

RSP_INSTR(vsar) {
  u8 e = E(instr) & 0xf;
  vpr_t* vd = &rsp->vpr[VD(instr)];
  for(int i = 0; i < 8; i++) {
    switch(e) {
      case 0:
        vd->element[ELEMENT_INDEX(i)] = rsp->acc.h.element[ELEMENT_INDEX(i)];
        rsp->acc.h.element[ELEMENT_INDEX(i)] = vd->element[ELEMENT_INDEX(i)];
        break;
      case 1:
        vd->element[ELEMENT_INDEX(i)] = rsp->acc.m.element[ELEMENT_INDEX(i)];
        rsp->acc.m.element[ELEMENT_INDEX(i)] = vd->element[ELEMENT_INDEX(i)];
        break;
      default:
        vd->element[ELEMENT_INDEX(i)] = rsp->acc.l.element[ELEMENT_INDEX(i)];
        rsp->acc.l.element[ELEMENT_INDEX(i)] = vd->element[ELEMENT_INDEX(i)];
        break;
    }
  }
}

RSP_INSTR(j) {
  u32 target = (s32)((instr & 0x3FFFFFF) << 2);
  u32 address = (rsp->old_pc & ~0xFFFFFFF) | target;
  rsp_branch(rsp, address, true);
}

RSP_INSTR(jal) {
  rsp->gpr[31] = rsp->next_pc;
  rsp_j(rsp, instr);
}

RSP_INSTR(jr) {
  rsp_branch(rsp, rsp->gpr[RS(instr)], true);
}

RSP_INSTR(nor) {
  rsp->gpr[RD(instr)] = ~(rsp->gpr[RS(instr)] | rsp->gpr[RT(instr)]);
}

RSP_INSTR(or_) {
  rsp->gpr[RD(instr)] = rsp->gpr[RS(instr)] | rsp->gpr[RT(instr)];
}

RSP_INSTR(ori) {
  u16 imm = instr & 0xFFFF;
  rsp->gpr[RT(instr)] = rsp->gpr[RS(instr)] | imm;
}

RSP_INSTR(sll) {
  u8 sa = (instr >> 6) & 0x1F;
  rsp->gpr[RD(instr)] = rsp->gpr[RT(instr)] << sa;
}

RSP_INSTR(sllv) {
  u8 sa = rsp->gpr[RS(instr)] & 0x1F;
  rsp->gpr[RD(instr)] = rsp->gpr[RT(instr)] << sa;
}

RSP_BRANCH(b) {
  s32 address = ((s32)((s16)(instr & 0xFFFF) << 2)) + rsp->pc;
  rsp_branch(rsp, address, cond);
}

void rsp_mfc0(rsp_t* rsp, rdp_t* rdp, u32 instr) {
  rsp->gpr[RT(instr)] = get_cop0_register(rsp, rdp, RD(instr));
}

void rsp_mtc0(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp, u32 instr) {
  set_cop0_register(mi, regs, rsp, rdp, RD(instr), rsp->gpr[RT(instr)]);
}