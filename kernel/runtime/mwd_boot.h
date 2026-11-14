#ifndef __MW_H_MWD_BOOT
#define __MW_H_MWD_BOOT

#include <linux/types.h>

void on_post_fs_data(void);
void on_module_mounted(void);
void on_boot_completed(void);

bool mw_is_safe_mode(void);

int nuke_ext4_sysfs(const char *mnt);

extern bool mw_module_mounted;
extern bool mw_boot_completed;

#endif // __MW_H_MWD_BOOT
