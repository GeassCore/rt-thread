/* SPDX-License-Identifier: MIT
 * RV32E frame: 14 integer registers, PC, mstatus, four reserved words.
 * CLIC is non-nested; IRQ switches are deferred until trap return.
 */
.section .text
.macro save_frame
 addi sp,sp,-80
 sw ra,0(sp)
 sw gp,4(sp)
 sw tp,8(sp)
 sw t0,12(sp)
 sw t1,16(sp)
 sw t2,20(sp)
 sw s0,24(sp)
 sw s1,28(sp)
 sw a0,32(sp)
 sw a1,36(sp)
 sw a2,40(sp)
 sw a3,44(sp)
 sw a4,48(sp)
 sw a5,52(sp)
.endm
.globl rt_hw_context_switch
rt_hw_context_switch:
 save_frame
 sw ra,56(sp)
 csrr t0,mstatus
 andi t1,t0,8
 slli t1,t1,4
 li t2,-137
 and t0,t0,t2
 or t0,t0,t1
 li t1,0x1800
 or t0,t0,t1
 sw t0,60(sp)
 sw sp,0(a0)
 lw sp,0(a1)
 j restore_frame
.globl rt_hw_context_switch_to
rt_hw_context_switch_to:
 lw sp,0(a0)
 j restore_frame
.balign 64
.globl rt_hw_trap_entry
rt_hw_trap_entry:
 save_frame
 csrr t0,mepc
 sw t0,56(sp)
 csrr t0,mstatus
 sw t0,60(sp)
 csrr a0,mcause
 mv a1,sp
 call rt_hw_trap_handler
 la t0,rt_thread_switch_interrupt_flag
 lw t1,0(t0)
 beqz t1,restore_frame
 sw zero,0(t0)
 la t0,rt_interrupt_from_thread
 lw t0,0(t0)
 sw sp,0(t0)
 la t0,rt_interrupt_to_thread
 lw t0,0(t0)
 lw sp,0(t0)
restore_frame:
 lw t0,56(sp)
 csrw mepc,t0
 lw t0,60(sp)
 csrw mstatus,t0
 lw ra,0(sp)
 lw gp,4(sp)
 lw tp,8(sp)
 lw t0,12(sp)
 lw t1,16(sp)
 lw t2,20(sp)
 lw s0,24(sp)
 lw s1,28(sp)
 lw a0,32(sp)
 lw a1,36(sp)
 lw a2,40(sp)
 lw a3,44(sp)
 lw a4,48(sp)
 lw a5,52(sp)
 addi sp,sp,80
 mret
