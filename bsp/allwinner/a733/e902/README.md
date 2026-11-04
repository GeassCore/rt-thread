# Allwinner A733 CPUS E902

The BSP hierarchy is Allwinner vendor → A733 SoC → E902 coprocessor.
A733-specific startup, interrupts, clocks and peripherals are maintained here.

This BSP runs RT-Thread Nano on the A733 CPUS E902 (RV32EMC, ILP32E).
It uses the normal RT-Thread SCons/Kconfig build and kernel startup.
No source or build script from the surrounding rtos-e902 collection is needed.

## Build

Use a RISC-V bare-metal GCC toolchain with RV32E/ILP32E libgcc support and
Python 3 with SCons, Kconfiglib and the dependencies of RT-Thread's tools.
For example, a toolchain named riscv64-unknown-elf-gcc can build RV32 code
when its multilib includes rv32emc/ilp32e.

```sh
cd bsp/allwinner/a733/e902
export CROSS_COMPILE=riscv64-unknown-elf-
scons -j4
# Optional configuration through the upstream RT-Thread tools:
scons --menuconfig
```

RTT_EXEC_PATH can select the compiler directory. The default configuration
is tracked as .config/rtconfig.h; defconfig contains its minimal Kconfig input.
Outputs are rtthread.elf, rtthread.bin and rtthread.map in this BSP directory.

## Source organization

- libcpu/risc-v/t-head/e902: generic RV32E context switching and IRQ state.
- board: reset entry, linker layout, CLIC dispatch, remoteproc resource table,
  and optional Linux telemetry.
- drivers: CPUS S_TIMER0 tick and polling hardware message-box interfaces.
- applications/main.c: optional multi-thread example using RT-Thread APIs.

The reset entry calls rtthread_startup(). The kernel calls rt_hw_board_init(),
creates main/timer/idle threads and starts the scheduler. The application runs
in the standard main thread. Board interrupts use rt_hw_interrupt_install(),
rt_interrupt_enter()/leave() and deferred switching on trap return. This port
uses a non-nested CLIC handler and an 80-byte RV32E context frame.

## Host loading and clocks

The Linux host's A733 E902 remoteproc driver owns core clocks/reset and loads
the ELF. Entry is 0x4000a000 (ARM PA 0x4a000). Firmware, static stacks and the
boot stack occupy a 92 KiB SRAM window ending at ARM PA 0x60fff. The next
4 KiB page at ARM PA 0x61000 is reserved for optional Linux telemetry.
The firmware enables the CPUS timer clock/reset and derives the timer input
from the RTC DCXO selector (24/19.2/26/24 MHz), then uses IRQ20 for the configured
RT_TICK_PER_SECOND rate (1 kHz by default).

This profile requires host ownership of CPUS; it does not run concurrently with
the stock ARISC/SCP firmware or implement its standby and power-management ABI.
FPU, ISA A atomics, SMP and protected userspace are not enabled.

## Example and observation

The optional example uses static thread/IPC objects, heartbeat, a non-yielding
low-priority busy thread, queue producer/consumer, a 250 ms soft timer and
mailbox channel 3 echo. Together with main/timer/idle there are seven threads.
The echo response is request XOR 0xe9020000 and belongs to the example rather
than the hardware driver. Linux tools can read execution, thread stack space
and fault state without wiring a UART. The wire layout is documented in
board/e902_linux_telemetry.h; the host observer belongs to the Linux project.

BSP_USING_E902_DEMO, BSP_USING_LINUX_TELEMETRY and BSP_USING_MSGBOX are separate
Kconfig options. Turning off Linux observation and the example still builds a
standalone Nano BSP. Physical UART console support is not included.
