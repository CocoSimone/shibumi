#pragma once
#include <common.h>
#include <stdbool.h>

#define STATUS_MASK 0xFF77FFFF
typedef struct cpu_t cpu_t;
typedef struct mem_t mem_t;

typedef struct registers_t registers_t;

typedef union {
  u32 raw;
  struct {
    unsigned:8;
    unsigned interrupt_pending:8;
    unsigned:16;
  } PACKED;
  struct {
    unsigned:2;
    unsigned exc_code:5;
    unsigned:1;
    unsigned ip0:1;
    unsigned ip1:1;
    unsigned ip2:1;
    unsigned ip3:1;
    unsigned ip4:1;
    unsigned ip5:1;
    unsigned ip6:1;
    unsigned ip7:1;
    unsigned:12;
    unsigned cop_error:2;
    unsigned:1;
    unsigned branch_delay:1;
  } PACKED;
} cop0_cause_t;

ASSERTWORD(cop0_cause_t);

typedef union {
  struct {
    unsigned ie:1;
    unsigned exl:1;
    unsigned erl:1;
    unsigned ksu:2;
    unsigned ux:1;
    unsigned sx:1;
    unsigned kx:1;
    unsigned im:8;
    unsigned ds:9;
    unsigned re:1;
    unsigned fr:1;
    unsigned rp:1;
    unsigned cu0:1;
    unsigned cu1:1;
    unsigned cu2:1;
    unsigned cu3:1;
  } PACKED;
  struct {
    unsigned:16;
    unsigned de:1;
    unsigned ce:1;
    unsigned ch:1;
    unsigned:1;
    unsigned sr:1;
    unsigned ts:1;
    unsigned bev:1;
    unsigned:1;
    unsigned its:1;
    unsigned:7;
  } PACKED;
  u32 raw;
} PACKED cop0_status_t;

ASSERTWORD(cop0_status_t);

typedef union {
  u32 raw;
  struct {
    unsigned g:1;
    unsigned v:1;
    unsigned d:1;
    unsigned c:3;
    unsigned pfn:20;
    unsigned:6;
  };
} entry_lo_t;

typedef union {
  struct {
    u64 asid:8;
    u64:5;
    u64 vpn2:27;
    u64 fill:22;
    u64 r:2;
  } PACKED;
  u64 raw;
} entry_hi_t;

typedef union {
  u32 raw;
  struct {
    unsigned:13;
    unsigned mask:12;
    unsigned:7;
  };
} page_mask_t;

typedef struct {
  union {
    struct {
      unsigned:1;
      unsigned v:1;
      unsigned d:1;
      unsigned c:3;
      unsigned pfn:20;
      unsigned:6;
    };
    u32 raw;
  } EntryLo0, EntryLo1;

  entry_hi_t EntryHi;
  page_mask_t PageMask;

  bool global;
} tlb_entry_t;

typedef enum {
  NONE,
  MISS,
  INVALID,
  MODIFICATION,
  DISALLOWED_ADDRESS
} tlb_error_t;

typedef enum {
  LOAD, STORE
} tlb_access_type_t;

typedef union {
  struct {
    u64:4;
    u64 badvpn2:19;
    u64 ptebase:41;
  };
  u64 raw;
} context_t;

typedef union {
  struct {
    u64:4;
    u64 badvpn2:27;
    u64 r:2;
    u64 ptebase:31;
  } PACKED;
  u64 raw;
} xcontext_t;

typedef struct {
  page_mask_t PageMask;
  entry_hi_t EntryHi;
  entry_lo_t EntryLo0, EntryLo1;
  u32 Index, Random;
  context_t Context;
  u32 Wired, r7;
  u64 BadVaddr, Count;
  u32 Compare;
  cop0_status_t Status;
  cop0_cause_t Cause;
  u64 EPC;
  u32 PRId, Config, LLAddr, WatchLo, WatchHi;
  xcontext_t XContext;
  u32 r21, r22, r23, r24, r25, ParityError, CacheError, TagLo, TagHi;
  u64 ErrorEPC;
  u32 r31;
  tlb_entry_t tlb[32];
  tlb_error_t TlbError;
} cop0_t;

void init_cop0(cop0_t* cop0);
u32 get_cop0_reg_word(cop0_t* cop0, u8 index);
void set_cop0_reg_word(cop0_t* cop0, u8 index, s32 value);
u64 get_cop0_reg_dword(cop0_t* cop0, u8 index);
void set_cop0_reg_dword(cop0_t* cop0, u8 index, u64 value);
tlb_entry_t* tlb_try_match(registers_t* regs, u32 vaddr, int* match);
bool probe_tlb(registers_t* regs, tlb_access_type_t access_type, u32 vaddr, u32* paddr, int* match);
void handle_tlb_exception(registers_t* regs, u64 vaddr);
exception_code_t get_tlb_exception_code(tlb_error_t error, tlb_access_type_t access_type);