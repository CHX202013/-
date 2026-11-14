#ifndef __MW_H_MWD
#define __MW_H_MWD

#include <asm/syscall.h>

#define MWD_PATH "/data/adb/mwd"

void mw_mwd_init();
void mw_mwd_exit();

void mw_execve_hook_mwd(const struct pt_regs *regs);
void mw_execveat_hook_mwd(const struct pt_regs *regs);
void mw_stop_input_hook_runtime(void);

#endif
