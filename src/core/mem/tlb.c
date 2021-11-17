#include <tlb.h>
#include <log.h>

u32 vtp(u32 addr) {
  switch(addr) {
    case 0x80000000 ... 0xBFFFFFFF:  return addr & 0x1FFFFFFF;
    default:
      log_(FATAL, "Unimplemented TLB region %08X!", addr);
      return 0;
  }
}