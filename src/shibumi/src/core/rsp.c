#include <rsp.h>
#include <log.h>
#include <intr.h>
#include <mem.h>
#include <string.h>
#include <access.h>
#include <rsp_decode_instr.h>

void init_rsp(rsp_t* rsp) {
  rsp->sp_status.raw = 1;
  rsp->old_pc = rsp->pc = 0;
  rsp->next_pc = 1;
  rsp->semaphore = false;
  memset(rsp->dmem, 0, DMEM_SIZE);
  memset(rsp->imem, 0, IMEM_SIZE);
}

void step_rsp(mi_t* mi, registers_t* regs, rsp_t* rsp, rdp_t* rdp) {
  if(!rsp->sp_status.halt) {
    rsp->gpr[0] = 0;
    u32 instr = raccess(32, rsp->imem, rsp->pc & IMEM_DSIZE);
    rsp->old_pc = rsp->pc & 0xFFF;
    rsp->pc = rsp->next_pc & 0xFFF;
    rsp->next_pc += 4;
    rsp_exec(mi, regs, rsp, rdp, instr);
  }
}

u32 sp_read(rsp_t* rsp, u32 addr) {
  switch (addr) {
    case 0x04040000: return rsp->sp_dma_sp_addr.raw & 0xFFFFF8;
    case 0x04040004: return rsp->sp_dma_dram_addr.raw & 0x1FF8;
    case 0x04040008: return rsp->sp_dma_rdlen.raw;
    case 0x0404000C: return rsp->sp_dma_wrlen.raw;
    case 0x04040010: return rsp->sp_status.raw;
    case 0x04040018: return 0;
    case 0x04080000: return rsp->pc & 0xFFF;
    default: logfatal("Unimplemented SP register read %08X\n", addr);
  }
}

INLINE void sp_dma(sp_dma_len_t len, rsp_t* rsp, u8* dst, u8* src, bool is_rdram_dest) {
  u32 length = len.len + 1;

  length = (length + 0x7) & ~0x7;

  u32 last_addr = rsp->sp_dma_sp_addr.address + length;
  if (last_addr > 0x1000) {
    u32 overshoot = last_addr - 0x1000;
    length -= overshoot;
  }

  u32 dram_address = rsp->sp_dma_dram_addr.address & 0xFFFFF8;
  u32 mem_address = rsp->sp_dma_sp_addr.address & 0x1FF8;

  for (int i = 0; i < len.count + 1; i++) {
    if(is_rdram_dest) {
      memcpy(&dst[dram_address], &src[mem_address], length);
    } else {
      memcpy(&dst[mem_address], &src[dram_address], length);
    }

    int skip = i == len.count ? 0 : len.skip;

    dram_address += (length + skip) & 0xFFFFF8;
    mem_address += length;
  }
}

void sp_write(rsp_t* rsp, mem_t* mem, registers_t* regs, u32 addr, u32 value) {
  mi_t* mi = &mem->mmio.mi;
  switch (addr) {
    case 0x04040000: rsp->sp_dma_sp_addr.raw = value & 0x1FF8; break;
    case 0x04040004: rsp->sp_dma_dram_addr.raw = value & 0xFFFFF8; break;
    case 0x04040008: {
      rsp->sp_dma_rdlen.raw = value;
      sp_dma(rsp->sp_dma_rdlen, rsp, rsp->sp_dma_sp_addr.bank ? rsp->imem : rsp->dmem, mem->rdram, false);
      rsp->sp_dma_rdlen.raw = 0xFF8 | (rsp->sp_dma_rdlen.skip << 20);
    } break;
    case 0x0404000C: {
      rsp->sp_dma_wrlen.raw = value;
      sp_dma(rsp->sp_dma_wrlen, rsp, mem->rdram, rsp->sp_dma_sp_addr.bank ? rsp->imem : rsp->dmem, true);
      rsp->sp_dma_wrlen.raw = 0xFF8 | (rsp->sp_dma_wrlen.skip << 20);
    } break;
    case 0x04040010: {
      sp_status_write_t write;
      write.raw = value;
      CLEAR_SET(rsp->sp_status.halt, write.clear_halt, write.set_halt);
      CLEAR_SET(rsp->sp_status.broke, write.clear_broke, false);
      if(write.clear_intr) interrupt_lower(mi, regs, SP);
      if(write.set_intr) interrupt_raise(mi, regs, SP);
      CLEAR_SET(rsp->sp_status.single_step, write.clear_sstep, write.set_sstep);
      CLEAR_SET(rsp->sp_status.interrupt_on_break, write.clear_intr_on_break, write.set_intr_on_break);
      CLEAR_SET(rsp->sp_status.signal_0_set, write.clear_signal_0, write.set_signal_0);
      CLEAR_SET(rsp->sp_status.signal_1_set, write.clear_signal_1, write.set_signal_1);
      CLEAR_SET(rsp->sp_status.signal_2_set, write.clear_signal_2, write.set_signal_2);
      CLEAR_SET(rsp->sp_status.signal_3_set, write.clear_signal_3, write.set_signal_3);
      CLEAR_SET(rsp->sp_status.signal_4_set, write.clear_signal_4, write.set_signal_4);
      CLEAR_SET(rsp->sp_status.signal_5_set, write.clear_signal_5, write.set_signal_5);
      CLEAR_SET(rsp->sp_status.signal_6_set, write.clear_signal_6, write.set_signal_6);
      CLEAR_SET(rsp->sp_status.signal_7_set, write.clear_signal_7, write.set_signal_7);
    } break;
    case 0x04080000:
      if(rsp->sp_status.halt) {
        rsp->old_pc = rsp->pc;
        rsp->pc = rsp->next_pc;
        rsp->next_pc = value & 0xFFF;
      } break;
    default: logfatal("Unimplemented SP register write %08X, val: %08X\n\n", addr, value);
  }
}