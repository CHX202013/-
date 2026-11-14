#ifndef __MW_H_KLOG
#define __MW_H_KLOG

#include <linux/printk.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#endif
