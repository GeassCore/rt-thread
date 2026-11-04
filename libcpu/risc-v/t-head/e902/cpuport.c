/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
volatile rt_ubase_t rt_interrupt_from_thread;
volatile rt_ubase_t rt_interrupt_to_thread;
volatile rt_uint32_t rt_thread_switch_interrupt_flag;
rt_base_t rt_hw_interrupt_disable(void)
{
    rt_base_t v;
    __asm__ volatile("csrrc %0,mstatus,8" : "=r"(v)::"memory");
    return v;
}
void rt_hw_interrupt_enable(rt_base_t level)
{
    if (level & 8)
        __asm__ volatile("csrsi mstatus,8" ::: "memory");
    else
        __asm__ volatile("csrci mstatus,8" ::: "memory");
}
int __rt_ffs(int value)
{
    return value ? __builtin_ffs(value) : 0;
}
rt_uint8_t *rt_hw_stack_init(void *entry, void *arg, rt_uint8_t *top, void *exit)
{
    rt_ubase_t *f = (rt_ubase_t *)(((rt_ubase_t)top + 4) & ~15u);
    f -= 20;
    for (unsigned i = 0; i < 20; i++)
        f[i] = 0;
    f[0] = (rt_ubase_t)exit;
    __asm__ volatile("mv %0,gp" : "=r"(f[1]));
    f[8] = (rt_ubase_t)arg;
    f[14] = (rt_ubase_t)entry;
    f[15] = 0x1880;
    return (rt_uint8_t *)f;
}
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread,
                                    rt_thread_t to_thread)
{
    (void)from_thread;
    (void)to_thread;
    if (!rt_thread_switch_interrupt_flag)
        rt_interrupt_from_thread = from;
    rt_interrupt_to_thread = to;
    rt_thread_switch_interrupt_flag = 1;
}
void rt_hw_cpu_shutdown(void)
{
    rt_hw_interrupt_disable();
    for (;;)
        __asm__ volatile("wfi");
}
rt_bool_t rt_hw_interrupt_is_disabled(void)
{
    rt_ubase_t status;
    __asm__ volatile("csrr %0,mstatus" : "=r"(status));
    return !(status & 8);
}
