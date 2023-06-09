/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2009, Citrix Systems, Inc.
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation.
 * Copyright (c) 2018, Arm Ltd.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __UKARCH_LCPU_H__
#error Do not include this header directly
#endif

#include <uk/asm.h>

#define CACHE_LINE_SIZE	64

#ifdef __ASSEMBLY__
/*
 * Stack size to save general purpose registers and essential system
 * registers. 8 * (30 + lr + elr_el1 + spsr_el1 + esr_el1) = 272.
 * From exceptions come from EL0, we have to save sp_el0. So the
 * TRAP_STACK_SIZE should be 272 + 8 = 280. But we enable the stack
 * alignment check, we will force align the stack for EL1 exceptions,
 * so we add a sp to save original stack pointer: 280 + 8 = 288
 *
 * TODO: We'd better to calculate this size automatically later.
 */
#define __TRAP_STACK_SIZE	288
#define __SP_OFFSET		272
#define __SP_EL0_OFFSET		280

/*
 * In thread context switch, we will save the callee-saved registers
 * (x19 ~ x28) and Frame Point Register and Link Register to prev's
 * thread stack:
 * http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
 */
#define __CALLEE_SAVED_SIZE    96

#else

#include <stdint.h>

/*
 * Change this structure must update TRAP_STACK_SIZE at the same time.
 * This data structure must be 16-byte alignment.
 */
struct __regs {
	/* Generic Purpose registers, from x0 ~ x29 */
	uint64_t x[30];

	/* Link Register (x30) */
	uint64_t lr;

	/* Exception Link Register */
	uint64_t elr_el1;

	/* Processor State Register */
	uint64_t spsr_el1;

	/* Exception Status Register */
	uint64_t esr_el1;

	/* Stack Pointer */
	uint64_t sp;

	/* Stack Pointer from el0 */
	uint64_t sp_el0;
};

/*
 * Change this structure must update __CALLEE_SAVED_SIZE at the
 * same time.
 */
struct __callee_saved_regs {
	/* Callee-saved registers, from x19 ~ x28 */
	uint64_t callee[10];

	/* Frame Point Register (x29) */
	uint64_t fp;

	/* Link Register (x30) */
	uint64_t lr;
};

/*
 * Instruction Synchronization Barrier flushes the pipeline in the
 * processor, so that all instructions following the ISB are fetched
 * from cache or memory, after the instruction has been completed.
 */
#define isb()   __asm__ __volatile("isb" ::: "memory")

/*
 * Options for DMB and DSB:
 *	oshld	Outer Shareable, load
 *	oshst	Outer Shareable, store
 *	osh	Outer Shareable, all
 *	nshld	Non-shareable, load
 *	nshst	Non-shareable, store
 *	nsh	Non-shareable, all
 *	ishld	Inner Shareable, load
 *	ishst	Inner Shareable, store
 *	ish	Inner Shareable, all
 *	ld	Full system, load
 *	st	Full system, store
 *	sy	Full system, all
 */
#define dmb(opt)    __asm__ __volatile("dmb " #opt ::: "memory")
#define dsb(opt)    __asm__ __volatile("dsb " #opt ::: "memory")

/* We probably only need "dmb" here, but we'll start by being paranoid. */
#ifndef mb
#define mb()    dsb(sy) /* Full system memory barrier all */
#endif

#ifndef rmb
#define rmb()   dsb(ld) /* Full system memory barrier load */
#endif

#ifndef wmb
#define wmb()   dsb(st) /* Full system memory barrier store */
#endif

static inline unsigned long ukarch_read_sp(void)
{
	unsigned long sp;

	__asm__ __volatile("mov %0, sp": "=&r"(sp));

	return sp;
}

static inline void ukarch_spinwait(void)
{
	/* Intelligent busy wait not supported on arm64. */
}

/******************************************************************
 * System Register Definitions
 ******************************************************************/

/* SCTLR_EL1: System Control Register */
#define SCTLR_EL1_M_BIT             (_AC(1, ULL) << 0)
#define SCTLR_EL1_A_BIT             (_AC(1, ULL) << 1)
#define SCTLR_EL1_C_BIT             (_AC(1, ULL) << 2)
#define SCTLR_EL1_SA_BIT            (_AC(1, ULL) << 3)
#define SCTLR_EL1_SA0_BIT           (_AC(1, ULL) << 4)
#define SCTLR_EL1_CP15BEN_BIT       (_AC(1, ULL) << 5)
#define SCTLR_EL1_ITD_BIT           (_AC(1, ULL) << 7)
#define SCTLR_EL1_SED_BIT           (_AC(1, ULL) << 8)
#define SCTLR_EL1_UMA_BIT           (_AC(1, ULL) << 9)
#define SCTLR_EL1_I_BIT             (_AC(1, ULL) << 12)
#define SCTLR_EL1_EnDB_BIT          (_AC(1, ULL) << 13)
#define SCTLR_EL1_DZE_BIT           (_AC(1, ULL) << 14)
#define SCTLR_EL1_UCT_BIT           (_AC(1, ULL) << 15)
#define SCTLR_EL1_NTWI_BIT          (_AC(1, ULL) << 16)
#define SCTLR_EL1_NTWE_BIT          (_AC(1, ULL) << 18)
#define SCTLR_EL1_WXN_BIT           (_AC(1, ULL) << 19)
#define SCTLR_EL1_UWXN_BIT          (_AC(1, ULL) << 20)
#define SCTLR_EL1_IESB_BIT          (_AC(1, ULL) << 21)
#define SCTLR_EL1_E0E_BIT           (_AC(1, ULL) << 24)
#define SCTLR_EL1_EE_BIT            (_AC(1, ULL) << 25)
#define SCTLR_EL1_UCI_BIT           (_AC(1, ULL) << 26)
#define SCTLR_EL1_EnDA_BIT          (_AC(1, ULL) << 27)
#define SCTLR_EL1_EnIB_BIT          (_AC(1, ULL) << 30)
#define SCTLR_EL1_EnIA_BIT          (_AC(1, ULL) << 31)
#define SCTLR_EL1_BT0_BIT           (_AC(1, ULL) << 35)
#define SCTLR_EL1_BT1_BIT           (_AC(1, ULL) << 36)
#define SCTLR_EL1_BT_BIT            (_AC(1, ULL) << 36)
#define SCTLR_EL1_DSSBS_BIT         (_AC(1, ULL) << 44)

/* ID_AA64_ISAR_EL1: AArch64 Instruction Set Attributes Register 1 */
#define ID_AA64ISAR1_EL1_GPI_SHIFT  28
#define ID_AA64ISAR1_EL1_GPI_MASK   0xf
#define ID_AA64ISAR1_EL1_GPA_SHIFT  24
#define ID_AA64ISAR1_EL1_GPA_MASK   0xf
#define ID_AA64ISAR1_EL1_API_SHIFT  8
#define ID_AA64ISAR1_EL1_API_MASK   0xf
#define ID_AA64ISAR1_EL1_APA_SHIFT  4
#define ID_AA64ISAR1_EL1_APA_MASK   0xf

#endif /* __ASSEMBLY__ */
