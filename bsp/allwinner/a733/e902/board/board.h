/* SPDX-License-Identifier: MIT */
#ifndef A733_E902_BOARD_H
#define A733_E902_BOARD_H
#include <rtthread.h>
#include <rthw.h>
#include "e902_linux_telemetry.h"
void rt_hw_board_init(void);
void a733_e902_early_init(void);
void a733_e902_set_stage(rt_uint32_t stage);
void a733_e902_fault(rt_uint32_t cause, rt_uint32_t pc) __attribute__((noreturn));
void a733_e902_timer_init(void);
void a733_e902_ack_tick(void);
void a733_msgbox_init(void);
rt_err_t a733_msgbox_recv(unsigned channel, rt_uint32_t *word);
rt_err_t a733_msgbox_send(unsigned channel, rt_uint32_t word);
#endif
