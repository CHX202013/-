#ifndef __MW_H_MW_CORE
#define __MW_H_MW_CORE

#include <linux/init.h>
#include <linux/types.h>

void mw_setuid_hook_init(void);
void mw_setuid_hook_exit(void);

// Handler functions for hook_manager
int mw_handle_setresuid(uid_t old_uid, uid_t new_uid);

#endif
