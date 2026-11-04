/* SPDX-License-Identifier: MIT */
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#define R32(a) (*(volatile rt_uint32_t *)(a))
#define R8(a) (*(volatile rt_uint8_t *)(a))
#define CPUX_BOX 0x03004000u
#define CPUS_BOX 0x07094000u
void a733_msgbox_init(void)
{
    R32(0x0701017c) |= 0x10001;
    R32(0x02002744) |= 0x10001;
}
rt_err_t a733_msgbox_recv(unsigned channel, rt_uint32_t *word)
{
    if (channel >= 4 || word == RT_NULL)
        return -RT_EINVAL;
    if (!(R32(CPUS_BOX + 0x60 + 4 * channel) & 15))
        return -RT_EEMPTY;
    *word = R32(CPUS_BOX + 0x70 + 4 * channel);
    R32(CPUS_BOX + 0x24) = 1u << (channel * 2);
    return RT_EOK;
}
rt_err_t a733_msgbox_send(unsigned channel, rt_uint32_t word)
{
    if (channel >= 4)
        return -RT_EINVAL;
    if ((R32(CPUX_BOX + 0x60 + 4 * channel) & 15) >= 8)
        return -RT_EFULL;
    R32(CPUX_BOX + 0x70 + 4 * channel) = word;
    return RT_EOK;
}
