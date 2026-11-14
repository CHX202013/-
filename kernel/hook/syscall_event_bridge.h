#ifndef __MW_H_SYSCALL_EVENT_BRIDGE
#define __MW_H_SYSCALL_EVENT_BRIDGE

#include <asm/ptrace.h>

long mw_hook_newfstatat(int orig_nr, const struct pt_regs *regs);
long mw_hook_faccessat(int orig_nr, const struct pt_regs *regs);
long mw_hook_execve(int orig_nr, const struct pt_regs *regs);
long mw_hook_execveat(int orig_nr, const struct pt_regs *regs);
long mw_hook_setresuid(int orig_nr, const struct pt_regs *regs);

void mw_stop_mwd_execve_hook(void);

#endif // __MW_H_SYSCALL_EVENT_BRIDGE
