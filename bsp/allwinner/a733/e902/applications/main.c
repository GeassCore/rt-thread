/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#ifdef BSP_USING_E902_DEMO
static struct rt_thread threads[4];
static unsigned char stacks[4][2048] __attribute__((aligned(16)));
static struct rt_messagequeue messages;
static unsigned char pool[256] __attribute__((aligned(4)));
static struct rt_timer probe;
static void heartbeat(void *p)
{
    (void)p;
    for (;;)
    {
        e902_debug->heartbeat++;
        rt_thread_delay(RT_TICK_PER_SECOND);
    }
}
static void busy(void *p)
{
    (void)p;
    for (;;)
        e902_debug->busy_counter++;
}
static void producer(void *p)
{
    (void)p;
    uint32_t n = 0;
    for (;;)
    {
        if (rt_mq_send(&messages, &n, sizeof(n)) == RT_EOK)
        {
            e902_debug->queue_tx++;
            n++;
        }
        rt_thread_delay(rt_tick_from_millisecond(20));
    }
}
static void consumer(void *p)
{
    (void)p;
    uint32_t n;
    for (;;)
        if (rt_mq_recv(&messages, &n, sizeof(n), RT_WAITING_FOREVER) == sizeof(n))
            e902_debug->queue_rx++;
}
static void timer(void *p)
{
    (void)p;
    e902_debug->software_timer++;
}
static void snapshot(void)
{
    rt_base_t key = rt_hw_interrupt_disable();
    e902_debug->sequence++;
    __asm__ volatile("fence rw,rw" ::: "memory");
    e902_debug->task_count = 0;
    struct rt_object_information *info = rt_object_get_information(RT_Object_Class_Thread);
    rt_list_t *node;
    for (node = info->object_list.next; node != &info->object_list; node = node->next)
    {
        unsigned i = e902_debug->task_count;
        if (i >= E902_DEBUG_TASKS)
            break;
        struct rt_thread *thread = rt_list_entry(node, struct rt_thread, parent.list);
        volatile struct e902_task_info *t = &e902_debug->tasks[i];
        for (unsigned j = 0; j < 16; j++)
            t->name[j] = thread->parent.name[j];
        unsigned state = rt_sched_thread_get_stat(thread) & RT_THREAD_STAT_MASK;
        t->state = thread == rt_thread_self() ? 0
                   : state == RT_THREAD_READY ? 1
                   : state == RT_THREAD_CLOSE ? 4
                                              : 2;
        t->priority = rt_sched_thread_get_curr_prio(thread);
        t->number = i;
        unsigned free = 0;
        unsigned char *stack = thread->stack_addr;
        while (free < thread->stack_size && stack[free] == '#')
            free++;
        t->stack_words = free / 4;
        e902_debug->task_count = i + 1;
    }
    __asm__ volatile("fence rw,rw" ::: "memory");
    e902_debug->sequence++;
    rt_hw_interrupt_enable(key);
}
static void mailbox_echo(void)
{
    static rt_uint32_t reply;
    static int pending;
    rt_uint32_t word;
    if (!pending && a733_msgbox_recv(3, &word) == RT_EOK)
    {
        e902_debug->mailbox_rx++;
        e902_debug->mailbox_last_rx = word;
        reply = word ^ 0xe9020000u;
        pending = 1;
    }
    if (pending && a733_msgbox_send(3, reply) == RT_EOK)
    {
        e902_debug->mailbox_tx++;
        e902_debug->mailbox_last_tx = reply;
        pending = 0;
    }
}
static void monitor(void *p)
{
    (void)p;
    unsigned n = 0;
    for (;;)
    {
        mailbox_echo();
        if (++n == 100)
        {
            snapshot();
            n = 0;
        }
        rt_thread_delay(rt_tick_from_millisecond(10));
    }
}
int main(void)
{
    rt_mq_init(&messages, "ipc", pool, sizeof(uint32_t), sizeof(pool), RT_IPC_FLAG_PRIO);
    void (*entries[])(void *) = {heartbeat, producer, consumer, busy};
    const char *names[] = {"heartbeat", "producer", "consumer", "busy"};
    const unsigned prios[] = {3, 5, 3, 20};
    for (unsigned i = 0; i < 4; i++)
    {
        if (rt_thread_init(&threads[i], names[i], entries[i], 0, stacks[i], sizeof(stacks[i]),
                           prios[i], 10) != RT_EOK)
            a733_e902_fault(0xdead3003u, i);
        rt_thread_startup(&threads[i]);
    }
    rt_timer_init(&probe, "probe", timer, 0, rt_tick_from_millisecond(250),
                  RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    rt_timer_start(&probe);
    a733_e902_set_stage(4);
    monitor(RT_NULL);
    return 0;
}

#else
int main(void)
{
    for (;;)
        rt_thread_delay(RT_TICK_PER_SECOND);
    return 0;
}
#endif
