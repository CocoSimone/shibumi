#pragma once
#include <common.h>

typedef struct rsp_t rsp_t;
typedef struct mi_t mi_t;
typedef struct registers_t registers_t;

typedef union {
  struct {
    unsigned clear_xbus_dmem_dma:1;
    unsigned set_xbus_dmem_dma:1;
    unsigned clear_freeze:1;
    unsigned set_freeze:1;
    unsigned clear_flush:1;
    unsigned set_flush:1;
    unsigned clear_tmem:1;
    unsigned clear_pipe:1;
    unsigned clear_cmd:1;
    unsigned clear_clock:1;
  };
  u32 raw;
} dpc_status_write_t;

typedef union {
  struct {
    unsigned xbus_dmem_dma;
    unsigned freeze;
    unsigned flush;
    unsigned start_gclk;
    unsigned tmem_busy;
    unsigned pipe_busy;
    unsigned cmd_busy;
    unsigned cbuf_ready;
    unsigned dma_busy;
    unsigned end_valid;
    unsigned start_valid;
  };
  u32 raw;
} dpc_status_read_t;

typedef struct {
  dpc_status_read_t status;
  u32 start;
  u32 current;
  u32 end;
} dpc_t;

typedef struct {
  dpc_t dpc;
  u32 cmd_buf[0xFFFFF];
} rdp_t;

void init_rdp(rdp_t* rdp);
u32 dp_read(rdp_t* rdp, u32 addr);
void dp_write(rdp_t* rdp, u32 addr, u32 val);
void dp_status_write(dpc_status_read_t* status, u32 val);
void rdp_run_command(mi_t* mi, registers_t* regs, rdp_t* rdp, rsp_t* rsp);
void on_full_sync(rdp_t* rdp);
