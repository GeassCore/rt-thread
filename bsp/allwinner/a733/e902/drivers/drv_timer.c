/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#define R32(a) (*(volatile rt_uint32_t *)(a))
#define R8(a) (*(volatile rt_uint8_t *)(a))
#define TIMER 0x07091000u
#define CLIC 0xe0800000u
static void timer_isr(int vector, void *parameter)
{
    (void)vector;
    (void)parameter;
    a733_e902_ack_tick();
#ifdef BSP_USING_LINUX_TELEMETRY
    e902_debug->timer_irq++;
#endif
    rt_tick_increase();
#ifdef BSP_USING_LINUX_TELEMETRY
    e902_debug->tick = rt_tick_get();
#endif
}

void a733_e902_timer_init(void)
{
    rt_hw_interrupt_install(20, timer_isr, RT_NULL, "cpus-timer0");
    R32(0x0701011c) |= 0x10001;
    R32(0x07010100) = 1;
    /* RTC DCXO mux matches Linux ccu-sun60i-a733-rtc: 24M, 19.2M, 26M, 24M. */
    static const uint32_t dcxo_hz[] = {24000000u, 19200000u, 26000000u, 24000000u};
    uint32_t source_hz = dcxo_hz[(R32(0x07090160) >> 14) & 3];
    R32(TIMER + 0x20) = 0;
    R32(TIMER + 0x24) = source_hz / RT_TICK_PER_SECOND;
    R32(TIMER + 0x2c) = 0;
    R32(TIMER + 0x20) = 2;
    unsigned n = 100000;
    while ((R32(TIMER + 0x20) & 2) && --n)
        ;
    if (!n)
        a733_e902_fault(0xdead0020, R32(TIMER + 0x20));
    R32(TIMER + 4) = 1;
    R32(TIMER) |= 1;
    R32(CLIC + 8) = 0;
    R8(CLIC + 0x1002 + 4 * 20) = 0;
    R8(CLIC + 0x1003 + 4 * 20) = 0xff;
    rt_hw_interrupt_umask(20);
    R32(TIMER + 0x20) = 1;
}
void a733_e902_ack_tick(void)
{
    R32(TIMER + 4) = 1;
}
