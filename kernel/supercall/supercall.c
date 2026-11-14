#include <linux/anon_inodes.h>
#include <linux/err.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kprobes.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/task_work.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include "uapi/supercall.h"
#include "supercall/internal.h"
#include "arch.h"
#include "util.h"
#include "klog.h" // IWYU pragma: keep

#define MW_DRIVER_PERMISSION_SU_SESSION (1UL << 0)

struct mw_driver_context {
    unsigned long permissions;
};

struct mw_install_fd_tw {
    struct callback_head cb;
    int __user *outp;
};

static int anon_mw_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    pr_info("mw fd released\n");
    return 0;
}

static long anon_mw_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return mw_supercall_handle_ioctl(filp, cmd, (void __user *)arg);
}

static const struct file_operations anon_mw_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = anon_mw_ioctl,
    .compat_ioctl = anon_mw_ioctl,
    .release = anon_mw_release,
};

static int mw_install_fd_with_permissions(unsigned int fd_flags, unsigned long permissions)
{
    struct mw_driver_context *context;
    struct file *filp;
    const char *name;
    int fd;

    context = kzalloc(sizeof(*context), GFP_KERNEL);
    if (!context)
        return -ENOMEM;

    context->permissions = permissions;
    name = permissions & MW_DRIVER_PERMISSION_SU_SESSION ? "[mw_driver_su]" : "[mw_driver]";

    fd = get_unused_fd_flags(fd_flags);
    if (fd < 0) {
        pr_err("mw_install_fd: failed to get unused fd\n");
        kfree(context);
        return fd;
    }

    filp = anon_inode_getfile(name, &anon_mw_fops, context, O_RDWR);
    if (IS_ERR(filp)) {
        pr_err("mw_install_fd: failed to create anon inode file\n");
        put_unused_fd(fd);
        kfree(context);
        return PTR_ERR(filp);
    }

    fd_install(fd, filp);
    pr_info("mw fd installed: %d for pid %d\n", fd, current->pid);
    return fd;
}

int mw_install_fd(void)
{
    return mw_install_fd_with_permissions(O_CLOEXEC, 0);
}

int mw_install_su_fd(void)
{
    // This descriptor must be installed after the exec into mwd.
    return mw_install_fd_with_permissions(O_CLOEXEC, MW_DRIVER_PERMISSION_SU_SESSION);
}

bool mw_is_su_session_fd(const struct file *filp)
{
    const struct mw_driver_context *context = filp->private_data;

    return context && (context->permissions & MW_DRIVER_PERMISSION_SU_SESSION);
}

static void mw_install_fd_tw_func(struct callback_head *cb)
{
    struct mw_install_fd_tw *tw = container_of(cb, struct mw_install_fd_tw, cb);
    int fd = mw_install_fd();

    pr_info("[%d] install mw fd: %d\n", current->pid, fd);
    if (copy_to_user(tw->outp, &fd, sizeof(fd))) {
        pr_err("install mw fd reply err\n");
        mw_close_fd(fd);
    }

    kfree(tw);
}

static int reboot_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    struct pt_regs *real_regs = PT_REAL_REGS(regs);
    int magic1 = (int)PT_REGS_SYSCALL_PARM1(real_regs);
    int magic2 = (int)PT_REGS_PARM2(real_regs);

    if (magic1 == MW_INSTALL_MAGIC1 && magic2 == MW_INSTALL_MAGIC2) {
        struct mw_install_fd_tw *tw;
        unsigned long arg4 = (unsigned long)PT_REGS_SYSCALL_PARM4(real_regs);

        tw = kzalloc(sizeof(*tw), GFP_ATOMIC);
        if (!tw)
            return 0;

        tw->outp = (int __user *)arg4;
        tw->cb.func = mw_install_fd_tw_func;

        if (task_work_add(current, &tw->cb, TWA_RESUME)) {
            kfree(tw);
            pr_warn("install fd add task_work failed\n");
        }
    }

    return 0;
}

static struct kprobe reboot_kp = {
    .symbol_name = REBOOT_SYMBOL,
    .pre_handler = reboot_handler_pre,
};

void __init mw_supercalls_init(void)
{
    int rc;

    mw_supercall_dump_commands();

    rc = register_kprobe(&reboot_kp);
    if (rc) {
        pr_err("reboot kprobe failed: %d\n", rc);
    } else {
        pr_info("reboot kprobe registered successfully\n");
    }
}

void __exit mw_supercalls_exit(void)
{
    unregister_kprobe(&reboot_kp);
    mw_supercall_cleanup_state();
}
