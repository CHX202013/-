#ifndef __MW_H_HOOK_MANAGER
#define __MW_H_HOOK_MANAGER

#include <asm/ptrace.h>

// Hook manager initialization and cleanup
void mw_syscall_hook_manager_init(void);
void mw_syscall_hook_manager_exit(void);

#endif
