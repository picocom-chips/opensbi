#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Andes Technology Corporation
# Copyright (c) 2022 Picocom
#
# Authors:
#   Zong Li <zong@andestech.com>
#   Nylon Chen <nylon7@andestech.com>
#   Byron Bradley <byron.bradley@picocom.com>
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Objects to build
platform-objs-y += cache.o platform.o plmt.o

PLATFORM_RISCV_XLEN = 32
PLATFORM_RISCV_ABI = ilp32d
PLATFORM_RISCV_ISA = rv32gc
PLATFORM_RISCV_CODE_MODEL = medany

# Blobs to build
FW_TEXT_START=0x80F00000

FW_DYNAMIC=y

FW_OPTIONS=0x02

FW_JUMP=y
FW_JUMP_ADDR=0x80000000
FW_JUMP_FDT_ADDR=0x80EF0000

FW_PAYLOAD=y
FW_PAYLOAD_OFFSET=0x40000000
FW_PAYLOAD_FDT_ADDR=0x40EF0000
