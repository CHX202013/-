/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Embed the preinit runner. It is only needed until mw_preinit_init() copies
 * it into shmem, so keep it in an init section that is freed after load.
 */
	.section .init.rodata, "a"
	.balign 16
	.global mw_preinit_runner_start
mw_preinit_runner_start:
	.incbin MW_PREINIT_RUNNER_BIN
	.global mw_preinit_runner_end
mw_preinit_runner_end:
