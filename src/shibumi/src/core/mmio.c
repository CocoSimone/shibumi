#include <mmio.h>
#include <mem.h>
#include <registers.h>
#include <log.h>

void init_mmio(mmio_t *mmio) {
  init_mi(&mmio->mi);
  init_pi(&mmio->pi);
  init_ri(&mmio->ri);
  init_ai(&mmio->ai);
  init_vi(&mmio->vi);
  init_si(&mmio->si);
  init_rsp(&mmio->rsp);
  init_rdp(&mmio->rdp);
}

u32 read_mmio(mmio_t* mmio, u32 addr) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: return sp_read(&mmio->rsp, addr);
    case 0x04100000 ... 0x041FFFFF: return dp_read(&mmio->rdp, addr);
    case 0x04300000 ... 0x043FFFFF: return mi_read(&mmio->mi, addr);
    case 0x04400000 ...	0x044FFFFF: return vi_read(&mmio->vi, addr);
    case 0x04500000 ... 0x045FFFFF: return ai_read(&mmio->ai, addr);
    case 0x04600000 ... 0x046FFFFF: return pi_read(&mmio->mi, &mmio->pi, addr);
    case 0x04700000 ... 0x047FFFFF: return ri_read(&mmio->ri, addr);
    case 0x04800000 ... 0x048FFFFF: return si_read(&mmio->si, &mmio->mi, addr);
    default: logfatal("Unhandled mmio read at addr %08X\n", addr);
  }
}

void write_mmio(mem_t* mem, registers_t* regs, si_t* si, u32 addr, u32 val) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: sp_write(&mem->mmio.rsp, mem, regs, addr, val); break;
    case 0x04100000 ... 0x041FFFFF: dp_write(&mem->mmio.rdp, addr, val); break;
    case 0x04300000 ... 0x043FFFFF: mi_write(&mem->mmio.mi, regs, addr, val); break;
    case 0x04400000 ...	0x044FFFFF: vi_write(&mem->mmio.mi, regs, &mem->mmio.vi, addr, val); break;
    case 0x04500000 ... 0x045FFFFF: ai_write(mem, regs, addr, val); break;
    case 0x04600000 ... 0x046FFFFF: pi_write(mem, regs, addr, val); break;
    case 0x04700000 ... 0x047FFFFF: ri_write(&mem->mmio.ri, addr, val); break;
    case 0x04800000 ... 0x048FFFFF: si_write(&mem->mmio.si, mem, regs, addr, val); break;
    default: logfatal("Unhandled mmio write at addr %08X with val %08X\n", addr, val);
  }
}
