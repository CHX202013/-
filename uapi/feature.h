#ifndef __MW_UAPI_FEATURE_H
#define __MW_UAPI_FEATURE_H

enum mw_feature_id {
    MW_FEATURE_SU_COMPAT = 0,
    MW_FEATURE_KERNEL_UMOUNT = 1,
    MW_FEATURE_SULOG = 2,
    MW_FEATURE_ADB_ROOT = 3,
    MW_FEATURE_SELINUX_HIDE = 4,
    // 5 was webview_zygote_umount (v3.3.0), keep it unused so stale configs don't apply
    MW_FEATURE_SHELL_SU_ADB_ONLY = 6,

    MW_FEATURE_MAX
};

#endif
