#ifndef __MW_H_SELINUX_HIDE
#define __MW_H_SELINUX_HIDE

void mw_selinux_hide_init();
void mw_selinux_hide_exit();
void mw_selinux_hide_drop_backup_if_unused();
void mw_selinux_hide_handle_second_stage();
void mw_selinux_hide_handle_post_fs_data();

#endif
