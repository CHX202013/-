#include <linux/compiler.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/task_work.h>
#include <linux/thread_info.h>
#include <linux/seccomp.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>

#include "policy/allowlist.h"
#include "hook/setuid_hook.h"
#include "klog.h" // IWYU pragma: keep
#include "manager/manager_identity.h"
#include "infra/seccomp_cache.h"
#include "supercall/supercall.h"
#include "hook/tp_marker.h"
#include "feature/kernel_umount.h"

int mw_handle_setresuid(uid_t old_uid, uid_t new_uid)
{
    // we rely on the fact that zygote always call setresuid(3) with same uids

    if (unlikely(is_uid_manager(new_uid))) {
        spin_lock_irq(&current->sighand->siglock);
        mw_seccomp_allow_cache(current->seccomp.filter, __NR_reboot);
        mw_set_task_tracepoint_flag(current);
        spin_unlock_irq(&current->sighand->siglock);

        mw_install_fd();
        return 0;
    }

    if (mw_is_allow_uid_for_current(new_uid)) {
        if (current->seccomp.mode == SECCOMP_MODE_FILTER && current->seccomp.filter) {
            spin_lock_irq(&current->sighand->siglock);
            mw_seccomp_allow_cache(current->seccomp.filter, __NR_reboot);
            spin_unlock_irq(&current->sighand->siglock);
        }
        mw_set_task_tracepoint_flag(current);
    } else {
        mw_clear_task_tracepoint_flag_if_needed(current);
    }

    // Handle kernel umount
    mw_handle_umount(old_uid, new_uid);

    return 0;
}

void __init mw_setuid_hook_init(void)
{
    mw_kernel_umount_init();
}

void __exit mw_setuid_hook_exit(void)
{
    pr_info("mw_core_exit\n");
    mw_kernel_umount_exit();
}
