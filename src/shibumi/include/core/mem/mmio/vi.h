#pragma once
#include <common.h>
#include <stdbool.h>

typedef struct mi_t mi_t;
typedef struct registers_t registers_t;

typedef union {
  struct {
    unsigned hsync_w:8;
    unsigned burst_w:8;
    unsigned vsync_w:4;
    unsigned burst_start:10;
    unsigned:2;
  };
  u32 raw;
} vi_burst_t;

typedef union {
  struct {
    unsigned leap_b:10;
    unsigned:6;
    unsigned leap_a:10;
    unsigned:6;
  };
  u32 raw;
} vi_hsync_leap_t;

typedef union {
  struct {
    unsigned end:10;
    unsigned:6;
    unsigned start:10;
    unsigned:6;
  };
  u32 raw;
} vi_video_t;

typedef union {
  struct {
    unsigned scale:12;
    unsigned:4;
    unsigned offset:12;
    unsigned:4;
  };
  u32 raw;
} vi_scale_t;

enum VI_FORMAT {
  blank = 0,
  reserved = 1,
  f5553 = 2,
  f8888 = 3
};

typedef union {
  struct {
    u8 format:2;
    unsigned serrate:1;
    unsigned:29;
  };

  u32 raw;
} status_t;

typedef struct {
  vi_scale_t xscale, yscale;
  vi_video_t hvideo, vvideo;
  vi_hsync_leap_t hsync_leap;
  status_t status;
  vi_burst_t burst, vburst;
  u32 origin, width, current;
  u32 vsync, hsync, intr;
  u32 hstart, vstart;
  int swaps;
  int num_halflines;
  int num_fields;
  int cycles_per_halfline;
} vi_t;

void init_vi(vi_t* vi);
u32 vi_read(vi_t* vi, u32 paddr);
void vi_write(mi_t* mi, registers_t* regs, vi_t* vi, u32 paddr, u32 val);
