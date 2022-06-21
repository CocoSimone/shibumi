#include <stdio.h>
#include <cop0.h>
#include <cpu.h>
#include <log.h>

void init_cop0(cop0_t* cop0) {
  cop0->Cause.raw = 0xB000007C;
  cop0->Random = 0x0000001F;
  cop0->Status.raw = 0x241000E0;
  cop0->Wired = 64;
  cop0->Index = 64;
  cop0->PRId = 0x00000B00;
  cop0->Config = 0x7006E463;
  cop0->EPC = 0xFFFFFFFFFFFFFFFF;
  cop0->ErrorEPC = 0xFFFFFFFFFFFFFFFF;
}

u32 get_cop0_reg_word(cop0_t* cop0, u8 index) {
  switch(index) {
    case 0: return cop0->Index & 0x8000001F;
    case 1: return cop0->Random & 0x1F;
    case 2: return cop0->EntryLo0.raw & 0x3FFFFFFF;
    case 3: return cop0->EntryLo1.raw & 0x3FFFFFFF;
    case 4: return cop0->Context.raw;
    case 5: return cop0->PageMask.raw;
    case 6: return cop0->Wired & 0x3F;
    case 7: return cop0->r7;
    case 8: return cop0->BadVaddr;
    case 9: return cop0->Count >> 1;
    case 10: return cop0->EntryHi.raw & 0xFFFFFFFFFFFFE0FF;
    case 11: return cop0->Compare;
    case 12: return cop0->Status.raw & STATUS_MASK;
    case 13: return cop0->Cause.raw;
    case 14: return cop0->EPC;
    case 15: return cop0->PRId & 0xFFFF;
    case 16: return cop0->Config;
    case 17: return cop0->LLAddr;
    case 18: return cop0->WatchLo;
    case 19: return cop0->WatchHi;
    case 20: return cop0->XContext.raw & 0xFFFFFFF0;
    case 21: return cop0->r21;
    case 22: return cop0->r22;
    case 23: return cop0->r23;
    case 24: return cop0->r24;
    case 25: return cop0->r25;
    case 26: return cop0->ParityError;
    case 27: return cop0->CacheError;
    case 28: return cop0->TagLo & 0xFFFFFFF;
    case 29: return cop0->TagHi;
    case 30: return cop0->ErrorEPC;
    case 31: return cop0->r31;
    default:
      logfatal("Unsupported word read from COP0 register %d\n", index);
  }
}

void set_cop0_reg_word(cop0_t* cop0, u8 index, s32 value) {
  switch(index) {
    case 0: cop0->Index = value & 0x8000001F; break;
    case 1: cop0->Random = value & 0x1F; break;
    case 2: cop0->EntryLo0.raw = value & 0x3FFFFFFF; break;
    case 3: cop0->EntryLo1.raw = value & 0x3FFFFFFF; break;
    case 4: cop0->Context.raw = value; break;
    case 5: cop0->PageMask.raw = value; break;
    case 6: cop0->Wired = value & 0x3F; break;
    case 7: cop0->r7 = value; break;
    case 9: cop0->Count = value << 1; break;
    case 10: cop0->EntryHi.raw = value & 0xFFFFFFFFFFFFE0FF; break;
    case 11: {
      cop0->Cause.ip7 = 0;
      cop0->Compare = value;
    } break;
    case 12: cop0->Status.raw = value & STATUS_MASK; break;
    case 13: {
      cop0_cause_t tmp;
      tmp.raw = value;
      cop0->Cause.ip0 = tmp.ip0;
      cop0->Cause.ip1 = tmp.ip1;
    } break;
    case 14: cop0->EPC = value; break;
    case 15: cop0->PRId = value & 0xFFFF; break;
    case 16: cop0->Config = value; break;
    case 17: cop0->LLAddr = value; break;
    case 18: cop0->WatchLo = value; break;
    case 19: cop0->WatchHi = value; break;
    case 21: cop0->r21 = value; break;
    case 22: cop0->r22 = value; break;
    case 23: cop0->r23 = value; break;
    case 24: cop0->r24 = value; break;
    case 25: cop0->r25 = value; break;
    case 26: cop0->ParityError = value; break;
    case 27: cop0->CacheError = value; break;
    case 28: cop0->TagLo = value & 0xFFFFFFF; break;
    case 29: cop0->TagHi = value; break;
    case 30: cop0->ErrorEPC = value; break;
    case 31: cop0->r31 = value; break;
    default:
      logfatal("Unsupported word write to COP0 register %d\n", index);
  }
}

u64 get_cop0_reg_dword(cop0_t* cop0, u8 index) {
  switch(index) {
    case 2: return cop0->EntryLo0.raw & 0x1FFFFFF;
    case 3: return cop0->EntryLo1.raw & 0x1FFFFFF;
    case 4: return cop0->Context.raw;
    case 8: return cop0->BadVaddr;
    case 10: return cop0->EntryHi.raw & 0xFFFFFFFFFFFFE0FF;
    case 12: return cop0->Status.raw & STATUS_MASK;
    case 14: return cop0->EPC;
    case 17: return cop0->LLAddr;
    case 20: return cop0->XContext.raw;
    case 30: return cop0->ErrorEPC;
    default:
      logfatal("Unsupported dword read from COP0 register %d\n", index);
  }
}

void set_cop0_reg_dword(cop0_t* cop0, u8 index, u64 value) {
  switch(index) {
    case 2: cop0->EntryLo0.raw = value & 0x1FFFFFF; break;
    case 3: cop0->EntryLo1.raw = value & 0x1FFFFFF; break;
    case 4: cop0->Context.raw = value; break;
    case 8: break;
    case 10: cop0->EntryHi.raw = value & 0xFFFFFFFFFFFFE0FF; break;
    case 12: cop0->Status.raw = value & STATUS_MASK; break;
    case 14: cop0->EPC = value; break;
    case 17: cop0->LLAddr = value; break;
    case 20: break;
    case 30: cop0->ErrorEPC = value; break;
    default:
      logfatal("Unsupported dword write to COP0 register %d\n", index);
  }
}

#define vpn(addr, PageMask) (((((addr) & 0xFFFFFFFFFF) | (((addr) >> 22) & 0x30000000000)) & ~((PageMask) | 0x1FFF)))

tlb_entry_t* tlb_try_match(registers_t* regs, u32 vaddr, int* match) {
  for(int i = 0; i < 32; i++) {
    tlb_entry_t *entry = &regs->cp0.tlb[i];
    u64 entry_vpn = vpn(entry->EntryHi.raw, entry->PageMask.raw);
    u64 vaddr_vpn = vpn(vaddr, entry->PageMask.raw);

    bool vpn_match = entry_vpn == vaddr_vpn;
    bool asid_match = entry->global || (regs->cp0.EntryHi.asid == entry->EntryHi.asid);

    if(vpn_match && asid_match) {
      if(match) {
        *match = i;
      }
      return entry;
    }
  }

  return NULL;
}

bool probe_tlb(registers_t* regs, tlb_access_type_t access_type, u32 vaddr, u32* paddr, int* match) {
  tlb_entry_t* entry = tlb_try_match(regs, vaddr, match);
  if(!entry) {
    regs->cp0.TlbError = MISS;
    return false;
  }

  u32 mask = (entry->PageMask.mask << 12) | 0xFFF;
  u32 odd = vaddr & (mask + 1);
  u32 pfn;

  if(!odd) {
    if(!(entry->EntryLo0.v)) {
      regs->cp0.TlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !(entry->EntryLo0.d)) {
      regs->cp0.TlbError = MODIFICATION;
      return false;
    }

    pfn = entry->EntryLo0.pfn;
  } else {
    if(!(entry->EntryLo1.v)) {
      regs->cp0.TlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !(entry->EntryLo1.d)) {
      regs->cp0.TlbError = MODIFICATION;
      return false;
    }

    pfn = entry->EntryLo1.pfn;
  }

  if(paddr != NULL) {
    *paddr = (pfn << 12) | (vaddr & mask);
  }

  return true;
}

void handle_tlb_exception(registers_t* regs, u64 vaddr) {
  u64 vpn2 = (vaddr >> 13) & 0x7FFFF;
  u64 xvpn2 = (vaddr >> 13) & 0x7FFFFFF;
  regs->cp0.BadVaddr = vaddr;
  regs->cp0.Context.badvpn2 = vpn2;
  regs->cp0.XContext.badvpn2 = xvpn2;
  regs->cp0.XContext.r = (vaddr >> 62) & 3;
  regs->cp0.EntryHi.vpn2 = xvpn2;
  regs->cp0.EntryHi.r = (vaddr >> 62) & 3;
}

exception_code_t get_tlb_exception_code(tlb_error_t error, tlb_access_type_t access_type) {
  switch(error) {
    case NONE: logfatal("Getting TLB exception with error NONE\n");
    case INVALID: case MISS:
      return access_type == LOAD ? TLBLoad : TLBStore;
    case MODIFICATION:
      return TLBModification;
    case DISALLOWED_ADDRESS:
      return access_type == LOAD ? AddressErrorLoad : AddressErrorStore;
    default:
      logfatal("Getting TLB exception for unknown error code! (%d)\n", error);
  }
}