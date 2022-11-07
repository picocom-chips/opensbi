/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Andes Technology Corporation
 * Copyright (c) 2022 Picocom
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Byron Bradley <byron.bradley@picocom.com>
 */

#ifndef _PC805_PLMT_H_
#define _PC805_PLMT_H_

int plmt_warm_timer_init(void);

int plmt_cold_timer_init(unsigned long base, u32 hart_count);

#endif /* _PC805_PLMT_H_ */
