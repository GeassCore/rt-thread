/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#define R32(a) (*(volatile rt_uint32_t *)(a))
#define R8(a) (*(volatile rt_uint8_t *)(a))
#define E902_IRQ_COUNT 208
static struct rt_irq_desc irq_table[E902_IRQ_COUNT];
extern void rt_hw_trap_entry(void);
void rt_hw_interrupt_mask(int vector)
{
    if ((unsigned)vector < E902_IRQ_COUNT)
        R8(0xe0801001u + 4 * vector) = 0;
}
void rt_hw_interrupt_umask(int vector)
{
    if ((unsigned)vector < E902_IRQ_COUNT)
        R8(0xe0801001u + 4 * vector) = 1;
}
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler, void *parameter,
                                         const char *name)
{
    rt_isr_handler_t previous;
    rt_base_t level;
    if ((unsigned)vector >= E902_IRQ_COUNT)
        return RT_NULL;
    level = rt_hw_interrupt_disable();
    previous = irq_table[vector].handler;
    irq_table[vector].handler = handler;
    irq_table[vector].param = parameter;
#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(irq_table[vector].name, name ? name : "", RT_NAME_MAX - 1);
#else
    (void)name;
#endif
    rt_hw_interrupt_enable(level);
    return previous;
}
void rt_hw_interrupt_init(void)
{
    unsigned i;
    for (i = 0; i < E902_IRQ_COUNT; i++)
        rt_hw_interrupt_mask(i);
    __asm__ volatile("csrw mtvec,%0" ::"r"((rt_ubase_t)rt_hw_trap_entry | 3u) : "memory");
}
void rt_hw_trap_handler(rt_ubase_t cause, rt_ubase_t *frame)
{
    unsigned vector = cause & 0xfff;
    if (!(cause & 0x80000000u) || vector >= E902_IRQ_COUNT || irq_table[vector].handler == RT_NULL)
        a733_e902_fault(cause, frame[14]);
    rt_interrupt_enter();
#ifdef RT_USING_INTERRUPT_INFO
    irq_table[vector].counter++;
#endif
    irq_table[vector].handler(vector, irq_table[vector].param);
    rt_interrupt_leave();
}
