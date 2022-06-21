#include <mem.h>
#include <log.h>
#include <string.h>
#include <stdio.h>
#include <utils/swap.h>
#include <utils/bit.h>
#include <utils/access.h>
#include <cpu.h>

void init_mem(mem_t* mem) {
  mem->rdram = calloc(RDRAM_SIZE, 1);
  mem->sram = calloc(SRAM_SIZE, 1);
  memset(mem->pif_ram, 0, PIF_RAM_SIZE);
  memset(mem->pif_bootrom, 0, PIF_BOOTROM_SIZE);
  memset(mem->isviewer, 0, ISVIEWER_SIZE);
  init_mmio(&mem->mmio);
}

void destroy_mem(mem_t* mem) {
  free(mem->rdram);
  free(mem->sram);
}

bool load_rom(mem_t* mem, const char* path) {
  FILE* fp = fopen(path, "rb");
  if(fp == NULL) {
    printf("Failed to open rom %s\n", path);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t rom_file_size = ftell(fp);
  size_t rom_size = next_pow_2(rom_file_size);
  mem->rom_mask = rom_size - 1; 
  fseek(fp, 0, SEEK_SET);
  
  mem->cart = calloc(rom_size, 1);
  
  if(fread(mem->cart, 1, rom_file_size, fp) != rom_file_size) {
    printf("Failed to load rom\n");
    return false;
  }

  fclose(fp);
  swap(rom_size, mem->cart);
  memcpy(mem->mmio.rsp.dmem, mem->cart, 0x1000);
  return true;
}

const char* regions_str(u32 paddr) {
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: return "RDRAM";
    case 0x00800000 ... 0x03EFFFFF:
    case 0x04002000 ... 0x0403FFFF:
    case 0x04900000 ...	0x04FFFFFF: return "Unused";
    case 0x03F00000 ... 0x03FFFFFF: return "RDRAM Registers";
    case 0x04000000 ... 0x04000FFF: return "DMEM";
    case 0x04001000 ... 0x04001FFF: return "IMEM";
    case 0x04040000 ...	0x040FFFFF: return "SP Registers";
    case 0x04100000 ...	0x041FFFFF: return "DP Command Registers";
    case 0x04200000 ...	0x042FFFFF: return "DP Span Registers";
    case 0x04300000 ...	0x043FFFFF: return "MIPS Interface";
    case 0x04400000 ...	0x044FFFFF: return "Video Interface";
    case 0x04500000 ...	0x045FFFFF: return "Audio Interface";
    case 0x04600000 ...	0x046FFFFF: return "Peripheral Interface";
    case 0x04700000 ...	0x047FFFFF: return "RDRAM Interface";
    case 0x04800000 ...	0x048FFFFF: return "Serial Interface";
    case 0x05000000 ...	0x05FFFFFF: return "Cartridge Domain 2 Address 1";
    case 0x06000000 ...	0x07FFFFFF: return "Cartridge Domain 1 Address 1";
    case 0x08000000 ...	0x0FFFFFFF: return "Cartridge Domain 2 Address 2";
    case 0x10000000 ...	0x1FBFFFFF: return "Cartridge Domain 1 Address 2";
    case 0x1FC00000 ...	0x1FC007BF: return "PIF Bootrom";
    case 0x1FC007C0 ...	0x1FC007FF: return "PIF RAM";
    case 0x1FC00800 ...	0x1FCFFFFF: return "Reserved";
    case 0x1FD00000 ...	0x7FFFFFFF: return "Cartridge Domain 1 Address 3";
    case 0x80000000 ... 0xFFFFFFFF: return "Unknown";
    default: logfatal("Unhandled region string (%08X)\n", paddr);
  }
}

INLINE bool map_vaddr(registers_t* regs, tlb_access_type_t access_type, u32 vaddr, u32* paddr, bool tlb) {
  switch(vaddr >> 29) {
    case 0 ... 3: case 7:
      return probe_tlb(regs, access_type, (s64)((s32)vaddr), paddr, NULL);
    case 4: case 5:
      *paddr = vaddr & 0x1FFFFFFF;
      return true;
    case 6: logfatal("Unimplemented virtual mapping in KSSEG! (%08X)\n", vaddr);
    default:
      logfatal("Should never end up in default case in map_vaddr! (%08X)\n", vaddr);
  }
}

u8 read8_(mem_t* mem, registers_t* regs, u32 vaddr, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, LOAD, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, LOAD), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: return mem->rdram[paddr];
    case 0x04000000 ... 0x04000FFF: return mmio->rsp.dmem[paddr & DMEM_DSIZE];
    case 0x04001000 ... 0x04001FFF: return mmio->rsp.imem[paddr & IMEM_DSIZE];
    case 0x10000000 ... 0x1FBFFFFF: return mem->cart[paddr & mem->rom_mask];
    case 0x1FC00000 ... 0x1FC007BF: return mem->pif_bootrom[paddr & PIF_BOOTROM_DSIZE];
    case 0x1FC007C0 ... 0x1FC007FF: return mem->pif_ram[paddr & PIF_RAM_DSIZE];
    default: logfatal("Unimplemented %s[%08X] 8-bit read\n", regions_str(paddr), paddr);
  }
}

u16 read16_(mem_t* mem, registers_t* regs, u32 vaddr, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, LOAD, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, LOAD), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: return raccess(16, mem->rdram, paddr);
    case 0x04000000 ... 0x04000FFF: return raccess(16, mmio->rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04001000 ... 0x04001FFF: return raccess(16, mmio->rsp.imem, paddr & IMEM_DSIZE);
    case 0x10000000 ... 0x1FBFFFFF: return raccess(16, mem->cart, paddr & mem->rom_mask);
    case 0x1FC00000 ... 0x1FC007BF: return raccess(16, mem->pif_bootrom, paddr & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF: return raccess(16, mem->pif_ram, paddr & PIF_RAM_DSIZE);
    default: logfatal("Unimplemented %s[%08X] 16-bit read\n", regions_str(paddr), paddr);
  }
}

u32 read32_(mem_t* mem, registers_t* regs, u32 vaddr, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, LOAD, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, LOAD), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: return raccess(32, mem->rdram, paddr & RDRAM_DSIZE);
    case 0x04000000 ... 0x04000FFF: return raccess(32, mmio->rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04001000 ... 0x04001FFF: return raccess(32, mmio->rsp.imem, paddr & IMEM_DSIZE);
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      return read_mmio(mmio, paddr);
    case 0x10000000 ... 0x1FBFFFFF: return raccess(32, mem->cart, paddr & mem->rom_mask);
    case 0x1FC00000 ... 0x1FC007BF: return raccess(32, mem->pif_bootrom, paddr & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF: return raccess(32, mem->pif_ram, paddr & PIF_RAM_DSIZE);
    case 0x00800000 ... 0x03FFFFFF: case 0x04002000 ... 0x0403FFFF:
    case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: logfatal("Unimplemented %s[%08X] 32-bit read (PC = %016lX)\n", regions_str(paddr), paddr, regs->pc);
  }
}

u64 read64_(mem_t* mem, registers_t* regs, u32 vaddr, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, LOAD, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, LOAD), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: return raccess(64, mem->rdram, paddr);
    case 0x04000000 ... 0x04000FFF: return raccess(64, mmio->rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04001000 ... 0x04001FFF: return raccess(64, mmio->rsp.imem, paddr & IMEM_DSIZE);
    case 0x10000000 ... 0x1FBFFFFF: return raccess(64, mem->cart, paddr & mem->rom_mask);
    case 0x1FC00000 ... 0x1FC007BF: return raccess(64, mem->pif_bootrom, paddr & PIF_BOOTROM_DSIZE);
    default: logfatal("Unimplemented %s[%08X] 64-bit read\n", regions_str(paddr), paddr);
  }
}

void write8_(mem_t* mem, registers_t* regs, u32 vaddr, u8 val, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, STORE, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, STORE), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: mem->rdram[paddr] = val; break;
    case 0x04000000 ... 0x04000FFF: mmio->rsp.dmem[paddr & DMEM_DSIZE] = val; break;
    case 0x04001000 ... 0x04001FFF: mmio->rsp.imem[paddr & IMEM_DSIZE] = val; break;
    case 0x1FC007C0 ... 0x1FC007FF: mem->pif_ram[paddr & PIF_RAM_DSIZE] = val; break;
    default: logfatal("Unimplemented %s[%08X] 8-bit write (%02X)\n", regions_str(paddr), paddr, val);
  }
}

void write16_(mem_t* mem, registers_t* regs, u32 vaddr, u16 val, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, STORE, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, STORE), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: waccess(16, mem->rdram, paddr, val); break;
    case 0x04000000 ... 0x04000FFF: waccess(16, mmio->rsp.dmem, paddr & DMEM_DSIZE, val); break;
    case 0x04001000 ... 0x04001FFF: waccess(16, mmio->rsp.imem, paddr & IMEM_DSIZE, val); break;
    case 0x1FC007C0 ... 0x1FC007FF: waccess(16, mem->pif_ram, paddr & PIF_RAM_DSIZE, val); break;
    default: logfatal("Unimplemented %s[%08X] 16-bit write (%04X)\n", regions_str(paddr), paddr, val);
  }
}

void write32_(mem_t* mem, registers_t* regs, u32 vaddr, u32 val, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, STORE, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, STORE), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: waccess(32, mem->rdram, paddr, val); break;
    case 0x04000000 ... 0x04000FFF: waccess(32, mmio->rsp.dmem, paddr & DMEM_DSIZE, val); break;
    case 0x04001000 ... 0x04001FFF: waccess(32, mmio->rsp.imem, paddr & IMEM_DSIZE, val); break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      write_mmio(mem, regs, &mem->mmio.si, paddr, val);
      break;
    case 0x10000000 ... 0x13FF0013: break;
    case 0x13FF0014: {
      if(val < ISVIEWER_SIZE) {
        char* message = calloc(val + 1, 1);
        memcpy(message, mem->isviewer, val);
        printf("%s", message);
        free(message);
      }
    } break;
    case 0x13FF0020 ... 0x13FFFFFF: waccess(32, mem->isviewer, paddr & ISVIEWER_DSIZE, val); break;
    case 0x1FC007C0 ... 0x1FC007FF: waccess(32, mem->pif_ram, paddr & PIF_RAM_DSIZE, val); break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04002000 ... 0x0403FFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: break;
    default: logfatal("Unimplemented %s[%08X] 32-bit write (%08X)\n", regions_str(paddr), paddr, val);
  }
}

void write64_(mem_t* mem, registers_t* regs, u32 vaddr, u64 val, s64 pc, bool tlb) {
  u32 paddr = vaddr;
  if(!map_vaddr(regs, STORE, vaddr, &paddr, tlb)) {
    handle_tlb_exception(regs, vaddr);
    fire_exception(regs, get_tlb_exception_code(regs->cp0.TlbError, STORE), 0, pc);
  }

  mmio_t* mmio = &mem->mmio;
  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: waccess(64, mem->rdram, paddr, val); break;
    case 0x04000000 ... 0x04000FFF: waccess(64, mmio->rsp.dmem, paddr & DMEM_DSIZE, val); break;
    case 0x04001000 ... 0x04001FFF: waccess(64, mmio->rsp.imem, paddr & IMEM_DSIZE, val); break;
    default: logfatal("Unimplemented %s[%08X] 64-bit write (%16lX)\n", regions_str(paddr), paddr, val);
  }
}
