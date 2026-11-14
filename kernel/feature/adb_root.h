#ifndef __MW_H_ADB_ROOT
#define __MW_H_ADB_ROOT
#include <asm/ptrace.h>

long mw_adb_root_handle_execve(struct pt_regs *regs);
long mw_adb_root_handle_execveat(struct pt_regs *regs);

void mw_adb_root_init(void);

void mw_adb_root_exit(void);

#endif
