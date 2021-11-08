#pragma once
#include <utils/log.h>
#include <utils/swap.h>
#include <time.h>

#define RD(x) (((x) >> 11) & 0x1F)
#define RT(x) (((x) >> 16) & 0x1F)
#define RS(x) (((x) >> 21) & 0x1F)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define clock_to_ms(ticks) (ticks/(double)CLOCKS_PER_SEC)*1000.0