#ifndef __MW_H_KERNEL_COMPAT
#define __MW_H_KERNEL_COMPAT

#include <linux/fs.h>
#include <linux/version.h>

extern void mw_seccomp_clear_cache(struct seccomp_filter *filter, int nr);
extern void mw_seccomp_allow_cache(struct seccomp_filter *filter, int nr);

#endif
