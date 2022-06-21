#pragma once
#include <sp_status.h>
#include <mi.h>
#include <rdp.h>

typedef struct mem_t mem_t;
typedef struct registers_t registers_t;

typedef union {
  struct {
    unsigned len:12;
    unsigned count:8;
    unsigned skip:12;
  };
  u32 raw;
} sp_dma_len_t;

typedef union {
  struct {
    unsigned address:12;
    unsigned bank:1;
    unsigned: 19;
  };
  u32 raw;
} sp_dma_sp_addr_t;

typedef union {
  struct {
    unsigned address:24;
    unsigned:8;
  };
  u32 raw;
} sp_dma_dram_addr_t;

typedef union {
  s16 selement[8];
  u16 element[8];
  u8 byte[8];
  u32 word[4];
} vpr_t;

typedef union {

} dpc_status_t;

typedef struct rsp_t {
  sp_status_t sp_status;
  u16 old_pc, pc, next_pc;
  sp_dma_sp_addr_t sp_dma_sp_addr;
  sp_dma_dram_addr_t sp_dma_dram_addr;
  sp_dma_len_t sp_dma_rdlen, sp_dma_wrlen;
  u8 dmem[DMEM_SIZE], imem[IMEM_SIZE];
  vpr_t vpr[32];
  u32 gpr[32];
  u8 vce;
  struct {
    vpr_t h, m, l;
  } acc;

  struct {
    vpr_t l, h;
  } vcc, vco;

  bool semaphore;
} rsp_t;

void init_rsp(rsp_t* rsp);
void step_rsp(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp);
u32 sp_read(rsp_t* rsp, u32 addr);
void sp_write(rsp_t* rsp, mem_t* mem, registers_t* regs, u32 addr, u32 value);

INLINE void rsp_set_pc(rsp_t* rsp, u16 val) {
  rsp->pc = val;
  rsp->next_pc = val += 4;
}

INLINE u16 vco_as_u16(rsp_t* rsp) {
  u16 val = 0;
  for(int i = 0; i < 8; i++) {
    bool h = rsp->vco.h.element[7 - i] != 0;
    bool l = rsp->vco.l.element[7 - i] != 0;
    u32 mask = (l << i) | (h << (i + 8));
    val |= mask;
  }
  return val;
}

INLINE u16 vcc_as_u16(rsp_t* rsp) {
  u16 val = 0;
  for(int i = 0; i < 8; i++) {
    bool h = rsp->vcc.h.element[7 - i] != 0;
    bool l = rsp->vcc.l.element[7 - i] != 0;
    u32 mask = (l << i) | (h << (i + 8));
    val |= mask;
  }
  return val;
}