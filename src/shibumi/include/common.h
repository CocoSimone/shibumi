#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef __uint128_t u128;
typedef __int128_t s128;

#define bits(x) ((x) + 8)
#define bswap_16(x) (((x) << 8) | ((x) >> 8))
#define bswap_32(x) ((((x) << 24) | ((x) >> 24)) | ((((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8)))
#define PACKED __attribute__((__packed__))
#define ASSERTWORD(type) static_assert(sizeof(type) == 4, #type " must be 32 bits")
#define ASSERTDWORD(type) static_assert(sizeof(type) == 8, #type " must be 64 bits")
#define INLINE static inline __attribute__((always_inline))

#define CPU_FREQ 93750000
#define CYCLES_PER_FRAME (CPU_FREQ / 60)
#define RDRAM_SIZE 0x800000
#define RDRAM_DSIZE (RDRAM_SIZE - 1)
#define SRAM_SIZE 0x8000000
#define SRAM_DSIZE (SRAM_SIZE - 1)
#define DMEM_SIZE 0x1000
#define DMEM_DSIZE (DMEM_SIZE - 1)
#define IMEM_SIZE 0x1000
#define IMEM_DSIZE (IMEM_SIZE - 1)
#define PIF_RAM_SIZE 0x40
#define PIF_RAM_DSIZE (PIF_RAM_SIZE - 1)
#define PIF_BOOTROM_SIZE 0x7C0
#define PIF_BOOTROM_DSIZE (PIF_BOOTROM_SIZE - 1)
#define ISVIEWER_SIZE 0xFFDF
#define ISVIEWER_DSIZE (ISVIEWER_SIZE - 1)

#define RD(x) (((x) >> 11) & 0x1F)
#define RT(x) (((x) >> 16) & 0x1F)
#define RS(x) (((x) >> 21) & 0x1F)
#define FD(x) (((x) >>  6) & 0x1F)
#define FT(x) RT(x)
#define FS(x) RD(x)
#define BASE(x) RS(x)

#define CLEAR_SET(val, clear, set) do { \
  if(clear) (val) = 0; \
  if(set) (val) = 1; \
} while(0)

typedef enum exception_code_t {
  Interrupt,
  TLBModification,
  TLBLoad,
  TLBStore,
  AddressErrorLoad,
  AddressErrorStore,
  InstructionBusError,
  DataBusError,
  Syscall,
  Breakpoint,
  ReservedInstruction,
  CoprocessorUnusable,
  Overflow,
  Trap,
  FloatingPointError = 15,
  Watch = 23
} exception_code_t;