/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Andes Technology Corporation
 * Copyright (c) 2022 Picocom
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 *   Byron Bradley <byron.bradley@picocom.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_trap.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include "platform.h"
#include "plmt.h"
#include "cache.h"
#include "pma.h"

static struct plic_data plic = {
	.addr = PC805_PLIC_ADDR,
	.num_src = PC805_PLIC_NUM_SOURCES,
};

/* Platform final initialization. */
static int pc805_final_init(bool cold_boot)
{
	void *fdt;

	/* enable L1 cache */
	uintptr_t mcache_ctl_val = csr_read(CSR_MCACHECTL);

	if (!(mcache_ctl_val & V5_MCACHE_CTL_IC_EN))
		mcache_ctl_val |= V5_MCACHE_CTL_IC_EN;
	if (!(mcache_ctl_val & V5_MCACHE_CTL_DC_EN))
		mcache_ctl_val |= V5_MCACHE_CTL_DC_EN;
	if (!(mcache_ctl_val & V5_MCACHE_CTL_CCTL_SUEN))
		mcache_ctl_val |= V5_MCACHE_CTL_CCTL_SUEN;
	csr_write(CSR_MCACHECTL, mcache_ctl_val);

	if (!cold_boot)
		return 0;

	fdt = fdt_get_address();
	fdt_fixups(fdt);

	return 0;
}

/* Initialize the platform console. */
static int pc805_console_init(void)
{
	return uart8250_init(PC805_UART_ADDR,
			     PC805_UART_FREQUENCY,
			     PC805_UART_BAUDRATE,
			     PC805_UART_REG_SHIFT,
			     PC805_UART_REG_WIDTH,
			     PC805_UART_REG_OFFSET);
}

/* Initialize the platform interrupt controller for current HART. */
static int pc805_irqchip_init(bool cold_boot)
{
	u32 hartid = current_hartid();
	int ret;

	if (cold_boot) {
		ret = plic_cold_irqchip_init(&plic);
		if (ret)
			return ret;
	}

	return plic_warm_irqchip_init(&plic, 2 * hartid, 2 * hartid + 1);
}

/* Initialize platform timer for current HART. */
static int pc805_timer_init(bool cold_boot)
{
	int ret;

	if (cold_boot) {
		ret = plmt_cold_timer_init(PC805_PLMT_ADDR,
					   PC805_HART_COUNT);
		if (ret)
			return ret;
	}

	return plmt_warm_timer_init();
}

/* Vendor-Specific SBI handler */
static int pc805_vendor_ext_provider(long extid, long funcid,
	const struct sbi_trap_regs *regs, unsigned long *out_value,
	struct sbi_trap_info *out_trap)
{
	int ret = 0;
	switch (funcid) {
	case SBI_EXT_ANDES_GET_MCACHE_CTL_STATUS:
		*out_value = csr_read(CSR_MCACHECTL);
		break;
	case SBI_EXT_ANDES_GET_MMISC_CTL_STATUS:
		*out_value = csr_read(CSR_MMISCCTL);
		break;
	case SBI_EXT_ANDES_SET_MCACHE_CTL:
		ret = mcall_set_mcache_ctl(regs->a0);
		break;
	case SBI_EXT_ANDES_SET_MMISC_CTL:
		ret = mcall_set_mmisc_ctl(regs->a0);
		break;
	case SBI_EXT_ANDES_ICACHE_OP:
		ret = mcall_icache_op(regs->a0);
		break;
	case SBI_EXT_ANDES_DCACHE_OP:
		ret = mcall_dcache_op(regs->a0);
		break;
	case SBI_EXT_ANDES_L1CACHE_I_PREFETCH:
		ret = mcall_l1_cache_i_prefetch_op(regs->a0);
		break;
	case SBI_EXT_ANDES_L1CACHE_D_PREFETCH:
		ret = mcall_l1_cache_d_prefetch_op(regs->a0);
		break;
	case SBI_EXT_ANDES_NON_BLOCKING_LOAD_STORE:
		ret = mcall_non_blocking_load_store(regs->a0);
		break;
	case SBI_EXT_ANDES_WRITE_AROUND:
		ret = mcall_write_around(regs->a0);
		break;

    //A27 non-standard
    case SBI_EXT_ANDES_READ_POWERBRAKE:
        //*out_value = csr_read(CSR_MPFTCTL);
        break;
    case SBI_EXT_ANDES_WRITE_POWERBRAKE:
        //csr_write(CSR_MPFTCTL, regs->a0);
        break;
    case SBI_EXT_ANDES_SET_PMA:
        mcall_set_pma(regs->a0, regs->a1, regs->a2, regs->a3);
        break;
    case SBI_EXT_ANDES_FREE_PMA:
        mcall_free_pma(regs->a0);
        break;
    case SBI_EXT_ANDES_PROBE_PMA:
        *out_value = ((csr_read(CSR_MMSC_CFG) & 0x40000000) != 0);
        break;

	default:
		sbi_printf("Unsupported vendor sbi call : 0x%lx - %ld\n", extid, funcid);
		asm volatile("ebreak");
	}
	return ret;
}

/* Platform descriptor. */
const struct sbi_platform_operations platform_ops = {
	.final_init = pc805_final_init,

	.console_init = pc805_console_init,

	.irqchip_init = pc805_irqchip_init,

	.timer_init	   = pc805_timer_init,

	.vendor_ext_provider = pc805_vendor_ext_provider
};

const struct sbi_platform platform = {
	.opensbi_version = OPENSBI_VERSION,
	.platform_version = SBI_PLATFORM_VERSION(0x0, 0x01),
	.name = "Picocom PC805",
	.features = SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count = PC805_HART_COUNT,
	.hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr = (unsigned long)&platform_ops
};
