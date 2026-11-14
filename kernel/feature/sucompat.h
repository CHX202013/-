#ifndef __MW_H_SUCOMPAT
#define __MW_H_SUCOMPAT
#include <asm/ptrace.h>
#include <linux/types.h>

extern bool mw_su_compat_enabled;

void mw_sucompat_init(void);
void mw_sucompat_exit(void);

// Handler functions exported for hook_manager
long mw_handle_faccessat_sucompat(int orig_nr, struct pt_regs *regs);
long mw_handle_stat_sucompat(int orig_nr, struct pt_regs *regs);
long mw_handle_execve_sucompat(const char __user **filename_user, int orig_nr, struct pt_regs *regs);
long mw_handle_execveat_sucompat(const char __user **filename_user, int orig_nr, struct pt_regs *regs);

#endif
