/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#define R32(a) (*(volatile rt_uint32_t *)(a))
#define R8(a) (*(volatile rt_uint8_t *)(a))
volatile rt_uint32_t e902_boot_stage;
void a733_e902_set_stage(rt_uint32_t stage)
{
    e902_boot_stage = stage;
#ifdef BSP_USING_LINUX_TELEMETRY
    e902_debug->stage = stage;
#endif
}
void a733_e902_fault(rt_uint32_t cause, rt_uint32_t pc)
{
    __asm__ volatile("csrci mstatus, 8" ::: "memory");

#ifdef BSP_USING_LINUX_TELEMETRY
    e902_debug->fault = cause;
    e902_debug->fault_pc = pc;
#else
    (void)cause;
    (void)pc;
#endif
    a733_e902_set_stage(0xff);
    for (;;)
        __asm__ volatile("nop");
}
void a733_e902_early_init(void)
{
#ifdef BSP_USING_LINUX_TELEMETRY
    volatile rt_uint32_t *v = (volatile rt_uint32_t *)e902_debug;
    for (unsigned i = 0; i < sizeof(*e902_debug) / 4; i++)
        v[i] = 0;
    e902_debug->abi = 1;
    e902_debug->magic = E902_DEBUG_MAGIC;
    a733_e902_set_stage(1);
#endif
}
