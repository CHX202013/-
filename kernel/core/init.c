#include <linux/export.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/moduleparam.h>

#include "policy/allowlist.h"
#include "policy/app_profile.h"
#include "policy/feature.h"
#include "klog.h" // IWYU pragma: keep
#include "manager/manager_observer.h"
#include "manager/throne_tracker.h"
#include "hook/syscall_hook_manager.h"
#include "hook/lsm_hook.h"
#include "runtime/mwd.h"
#include "runtime/mwd_boot.h"
#include "feature/sulog.h"
#include "supercall/supercall.h"
#include "mw.h"
#include "infra/file_wrapper.h"
#include "selinux/selinux.h"
#include "hook/syscall_hook.h"
#include "feature/adb_root.h"
#include "feature/selinux_hide.h"
#include "feature/module_blacklist.h"
#include "feature/shell_su_adb_only.h"
#include "infra/symbol_resolver.h"
#include "preinit/preinit.h"

#if defined(__x86_64__) && !defined(CONFIG_MW_X86_PATCH_SYSCALL_DISPATCHER)
#include <asm/cpufeature.h>
#include <linux/version.h>
#ifndef X86_FEATURE_INDIRECT_SAFE
#error "FATAL: Your kernel is missing the indirect syscall bypass patches!"
#endif
#endif

// workaround for A12-5.10 kernel
// Some third-party kernel (e.g. linegaeOS) uses wrong toolchain, which supports
// CC_HAVE_STACKPROTECTOR_SYSREG while gki's toolchain doesn't.
// Therefore, mw lkm, which uses gki toolchain, requires this __stack_chk_guard,
// while those third-party kernel can't provide.
// Thus, we manually provide it instead of using kernel's
#if defined(CONFIG_STACKPROTECTOR) &&                                                                                  \
    (defined(CONFIG_ARM64) && defined(MODULE) && !defined(CONFIG_STACKPROTECTOR_PER_TASK))
#include <linux/stackprotector.h>
#include <linux/random.h>
unsigned long __stack_chk_guard __ro_after_init __attribute__((visibility("hidden")));

__attribute__((no_stack_protector)) void __init mw_setup_stack_chk_guard()
{
    unsigned long canary;

    /* Try to get a semi random initial value. */
    get_random_bytes(&canary, sizeof(canary));
    canary ^= LINUX_VERSION_CODE;
    canary &= CANARY_MASK;
    __stack_chk_guard = canary;
}

__attribute__((naked)) int __init mawei_init_early(void)
{
    asm("mov x19, x30;\n"
        "bl mw_setup_stack_chk_guard;\n"
        "mov x30, x19;\n"
        "b mawei_init;\n");
}
#define NEED_OWN_STACKPROTECTOR 1
#else
#define NEED_OWN_STACKPROTECTOR 0
#endif

struct cred *mw_cred;
bool mw_late_loaded;

#ifdef CONFIG_MW_DEBUG
bool allow_shell = true;
#else
bool allow_shell = false;
#endif
module_param(allow_shell, bool, 0);

bool mw_no_custom_rc = false;
module_param_named(norc, mw_no_custom_rc, bool, 0);

#ifdef MODULE
bool mw_bundled = false;
module_param_named(bundled, mw_bundled, bool, 0);
#endif

int __init mawei_init(void)
{
    int ret;
#if defined(__x86_64__) && !defined(CONFIG_MW_X86_PATCH_SYSCALL_DISPATCHER)
    // If the kernel has the hardening patch, X86_FEATURE_INDIRECT_SAFE must be set
    if (!boot_cpu_has(X86_FEATURE_INDIRECT_SAFE)) {
        pr_alert("*************************************************************");
        pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
        pr_alert("**                                                         **");
        pr_alert("**        X86_FEATURE_INDIRECT_SAFE is not enabled!        **");
        pr_alert("**      Mawei will abort initialization to prevent      **");
        pr_alert("**                     kernel panic.                       **");
        pr_alert("**                                                         **");
        pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
        pr_alert("*************************************************************");
        return -ENOSYS;
    }
#endif

#ifdef MODULE
    mw_late_loaded = (current->pid != 1);
#else
    mw_late_loaded = false;
#endif

#ifdef CONFIG_MW_DEBUG
    pr_alert("*************************************************************");
    pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
    pr_alert("**                                                         **");
    pr_alert("**         You are running Mawei in DEBUG mode          **");
    pr_alert("**                                                         **");
    pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
    pr_alert("*************************************************************");
#endif
    if (allow_shell) {
        pr_alert("shell is allowed at init!");
    }

    mw_cred = prepare_creds();
    if (!mw_cred) {
        pr_err("prepare cred failed!\n");
        return -ENOSYS;
    }

    pr_info("initializing symbol resolver\n");
    mw_init_symbol_resolver();

    ret = mw_selinux_inode_init();
    if (ret) {
        put_cred(mw_cred);
        mw_cred = NULL;
        return ret;
    }

    mw_syscall_hook_init();

    // Block modules early, before vendor modules are loaded in second stage.
    mw_module_blacklist_init();

    mw_feature_init();
    mw_sulog_init();
    mw_adb_root_init();
    mw_lsm_hook_init();
    mw_selinux_hide_init();
    mw_shell_su_adb_only_init();

    mw_supercalls_init();
    mw_app_profile_init();

    if (mw_late_loaded) {
        pr_info("late load mode, skipping kprobe hooks\n");

        apply_mawei_rules();
        cache_sid();
        setup_mw_cred();

        // Grant current process (mwd late-load) root
        // with MW SELinux domain before enforcing SELinux, so it
        // can continue to access /data/app etc. after enforcement.
        escape_to_root_for_init();

        mw_allowlist_init();
        mw_load_allow_list();

        mw_syscall_hook_manager_init();

        mw_throne_tracker_init();
        mw_observer_init();
        mw_file_wrapper_init();

        mw_boot_completed = true;
        track_throne(false);

        mw_feature_apply_forced();

        if (!getenforce()) {
            pr_info("Permissive SELinux, enforcing\n");
            setenforce(true);
        }

    } else {
        mw_syscall_hook_manager_init();

        mw_allowlist_init();

        mw_throne_tracker_init();

        mw_mwd_init();

        mw_preinit_init();

        mw_file_wrapper_init();
    }

#ifdef MODULE
#ifndef CONFIG_MW_DEBUG
    kobject_del(&THIS_MODULE->mkobj.kobj);
#endif
#endif
    return 0;
}

void __exit mawei_exit(void)
{
    // Phase 1: Stop all hooks first to prevent new callbacks
    mw_syscall_hook_manager_exit();

    mw_supercalls_exit();

    if (!mw_late_loaded) {
        mw_mwd_exit();
        mw_preinit_exit();
    }

    // Wait for any in-flight RCU readers (e.g. handler traversing allow_list)
    synchronize_rcu();

    // Phase 2: Now safe to release data structures
    mw_observer_exit();

    mw_throne_tracker_exit();

    mw_allowlist_exit();

    mw_shell_su_adb_only_exit();
    mw_selinux_hide_exit();
    mw_lsm_hook_exit();
    mw_adb_root_exit();
    mw_sulog_exit();
    mw_feature_exit();
    mw_module_blacklist_exit();

    put_cred(mw_cred);
}

#if NEED_OWN_STACKPROTECTOR
module_init(mawei_init_early);
#else
module_init(mawei_init);
#endif
module_exit(mawei_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Android kernel extension");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#else
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
