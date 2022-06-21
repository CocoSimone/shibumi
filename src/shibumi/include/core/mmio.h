#pragma once
#include <mi.h>
#include <ai.h>
#include <pi.h>
#include <ri.h>
#include <vi.h>
#include <si.h>
#include <rsp.h>
#include <rdp.h>

typedef struct mem_t mem_t;
typedef struct registers_t registers_t;

typedef struct {
  mi_t mi;
  pi_t pi;
  ri_t ri;
  vi_t vi;
  si_t si;
  ai_t ai;
  rsp_t rsp;
  rdp_t rdp;
} mmio_t;

void init_mmio(mmio_t* mmio);
u32 read_mmio(mmio_t* mmio, u32 addr);
void write_mmio(mem_t* mem, registers_t* regs, si_t* si, u32 addr, u32 val);
