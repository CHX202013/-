#ifndef __MW_H_SUPERCALL_INTERNAL
#define __MW_H_SUPERCALL_INTERNAL

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>

bool only_manager(void);
bool only_root(void);
bool manager_or_root(void);
bool always_allow(void);
bool allowed_for_su(void);

long mw_supercall_handle_ioctl(const struct file *filp, unsigned int cmd, void __user *argp);
void mw_supercall_dump_commands(void);
void mw_supercall_cleanup_state(void);

#endif // __MW_H_SUPERCALL_INTERNAL
