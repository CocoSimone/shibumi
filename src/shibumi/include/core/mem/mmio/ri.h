#pragma once
#include <common.h>
#include <stdbool.h>

typedef struct {
  u32 mode, config, select, refresh;
} ri_t;

void init_ri(ri_t* ri);
u32 ri_read(ri_t* ri, u32 paddr);
void ri_write(ri_t* ri, u32 paddr, u32 value);
