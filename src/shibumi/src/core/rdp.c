#include <rdp.h>
#include <log.h>
#include <stdbool.h>
#include <rsp.h>
#include <access.h>
#include <intr.h>

static const int cmd_lens[64] = {
  2, 2, 2, 2, 2, 2, 2, 2, 8, 12, 24, 28, 24, 28, 40, 44,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 4, 4, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2
};

void init_rdp(rdp_t* rdp) {
  rdp->dpc.status.raw = 0x80;
}

u32 dp_read(rdp_t* rdp, u32 addr) {
  switch(addr) {
    case 0x0410000C: return rdp->dpc.status.raw;
    default: logfatal("Unhandled DP Command Registers read (addr: %08X)\n", addr);
  }
}

void dp_status_write(dpc_status_read_t* status, u32 val) {
  dpc_status_write_t temp;
  temp.raw = val;
  CLEAR_SET(status->xbus_dmem_dma, temp.clear_xbus_dmem_dma, temp.set_xbus_dmem_dma);
  CLEAR_SET(status->freeze, temp.clear_freeze, false); // Setting it seems to break games? Avoid for now (TODO)
  CLEAR_SET(status->flush, temp.clear_flush, temp.set_flush);
}

void dp_write(rdp_t* rdp, u32 addr, u32 val) {
  switch(addr) {
    case 0x0410000C: dp_status_write(&rdp->dpc.status, val); break;
    default: logfatal("Unhandled DP Command Registers read (addr: %08X, val: %08X)\n", addr, val);
  }
}

void rdp_run_command(mi_t* mi, registers_t* regs, rdp_t* rdp, rsp_t* rsp) {
  static int remaining_cmds = 0;
  dpc_t* dpc = &rdp->dpc;
  dpc->status.freeze = true;

  const u32 current = dpc->current & 0xFFFFF8;
  const u32 end = dpc->end & 0xFFFFF8;

  int len = end - current;
  if(len <= 0) return;

  if(len + (remaining_cmds * 4) <= 0xFFFFF) {
    if(dpc->status.xbus_dmem_dma) {
      for(int i = 0; i < len; i += 4) {
        u32 cmd = raccess(32, rsp->dmem, current + i);
        rdp->cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    } else {
      if(end > 0x7FFFFF || current > 0x7FFFFF) {
        return;
      }
      for(int i = 0; i < len; i += 4) {
        u32 cmd = raccess(32, rsp->dmem, current + i);
        rdp->cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    }

    int word_len = (len >> 2) + remaining_cmds;
    int buf_index = 0;

    bool processed_all = true;

    while(buf_index < word_len) {
      u8 cmd = (rdp->cmd_buf[buf_index] >> 24) & 0x3F;

      int cmd_len = cmd_lens[cmd];
      if((buf_index + cmd_len) * 4 > len + (remaining_cmds * 4)) {
        remaining_cmds = word_len - buf_index;

        u32 tmp[remaining_cmds];
        for(int i = 0; i < remaining_cmds; i++) {
          tmp[i] = rdp->cmd_buf[buf_index + i];
        }

        for(int i = 0; i < remaining_cmds; i++) {
          rdp->cmd_buf[buf_index + i] = tmp[i];
        }

        processed_all = false;
        break;
      }

      if(cmd >= 8) {

      }

      if (cmd == 0x29) {
        on_full_sync(rdp);
        interrupt_raise(mi, regs,DP);
      }

      buf_index += cmd_len;
    }

    if(processed_all) {
      remaining_cmds = 0;
    }

    dpc->current = end;
    dpc->status.freeze = false;
    dpc->status.cbuf_ready = true;
  }
}

void on_full_sync(rdp_t* rdp) {
  logfatal("On full sync!\n");
}