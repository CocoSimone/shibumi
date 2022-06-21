#pragma once
#include <rsp.h>

typedef struct registers_t registers_t;

#define RSP_INSTR(name) void rsp_##name(rsp_t* rsp, u32 instr)
#define RSP_BRANCH(name) void rsp_##name(rsp_t* rsp, u32 instr, bool cond)
#define VT(x) (((x) >> 16) & 0x1F)
#define VS(x) (((x) >> 11) & 0x1F)
#define VD(x) (((x) >> 6) & 0x1F)
#define E(x) (((x) >> 21) & 0x1F)
#define DE(x) (((x) >> 11) & 0x1F)

RSP_INSTR(add);
RSP_INSTR(addi);
RSP_INSTR(and_);
RSP_INSTR(andi);
RSP_INSTR(cfc2);
RSP_BRANCH(b);
RSP_INSTR(lh);
RSP_INSTR(lw);
RSP_INSTR(lui);
RSP_INSTR(lqv);
RSP_INSTR(j);
RSP_INSTR(jal);
RSP_INSTR(jr);
RSP_INSTR(nor);
RSP_INSTR(or_);
RSP_INSTR(ori);
RSP_INSTR(sb);
RSP_INSTR(sh);
RSP_INSTR(sw);
RSP_INSTR(sqv);
RSP_INSTR(sllv);
RSP_INSTR(sll);
RSP_INSTR(vabs);
RSP_INSTR(vmov);
RSP_INSTR(veq);
RSP_INSTR(vne);
RSP_INSTR(vsar);
void rsp_mfc0(rsp_t* rsp, rdp_t* rdp, u32 instr);
void rsp_mtc0(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp, u32 instr);

INLINE void rsp_branch(rsp_t* rsp, u16 address, bool cond) {
  if(cond) {
    rsp->next_pc = address & 0xFFF;
  }
}
