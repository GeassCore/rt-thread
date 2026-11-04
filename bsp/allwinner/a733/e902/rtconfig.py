import os
import shutil
BSP_ROOT = os.path.dirname(os.path.abspath(__file__))
RTT_ROOT = os.path.abspath(os.getenv('RTT_ROOT', os.path.join(BSP_ROOT, '../../../..')))
ARCH = 'risc-v'
CPU = 'e902'
VENDOR = 't-head'
CROSS_TOOL = PLATFORM = 'gcc'
PREFIX = os.getenv('CROSS_COMPILE', 'riscv64-unknown-elf-')
EXEC_PATH = os.getenv('RTT_EXEC_PATH', os.path.dirname(shutil.which(PREFIX + 'gcc') or ''))
CC = PREFIX + 'gcc'
CXX = PREFIX + 'g++'
AS = CC
AR = PREFIX + 'ar'
LINK = CC
TARGET_EXT = 'elf'
SIZE = PREFIX + 'size'
OBJCPY = PREFIX + 'objcopy'
DEVICE = '-march=rv32emc -mabi=ilp32e -mcmodel=medany'
CFLAGS = DEVICE + ' -Os -g -ffreestanding -fno-builtin -fno-common -ffunction-sections -fdata-sections -Wall -Wextra -Wno-unused-parameter'
AFLAGS = DEVICE + ' -g -x assembler-with-cpp -c'
LFLAGS = DEVICE + ' -nostdlib -nostartfiles -Wl,--gc-sections,-Map=rtthread.map -T' + os.path.join(BSP_ROOT, 'board/link.lds')
CPATH = LPATH = ''
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET\n'

# Export canonical Kconfig paths for both mconf and Kconfiglib front ends.
os.environ["RTT_ROOT"] = RTT_ROOT
os.environ["RTT_DIR"] = RTT_ROOT
os.environ["BSP_ROOT"] = BSP_ROOT
os.environ["BSP_DIR"] = BSP_ROOT
