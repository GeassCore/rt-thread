/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#define R32(a) (*(volatile rt_uint32_t *)(a))
#define R8(a) (*(volatile rt_uint8_t *)(a))
static void assert_hook(const char *expression, const char *function, rt_size_t line)
{
    (void)expression;
    (void)function;
    a733_e902_fault(0xdead3001u, line);
}

void rt_hw_board_init(void)
{
    a733_e902_early_init();
#ifdef BSP_USING_LINUX_TELEMETRY
    e902_debug->abi = 2;
    e902_debug->os_id = 4;
#endif
    rt_assert_set_hook(assert_hook);
    rt_hw_interrupt_init();
    a733_e902_timer_init();
#ifdef BSP_USING_MSGBOX
    a733_msgbox_init();
#endif
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
