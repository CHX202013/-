#ifndef __MW_H_MW
#define __MW_H_MW

#include <linux/types.h>
#include <linux/cred.h>
#include <linux/workqueue.h>

#define KERNEL_SU_VERSION MW_VERSION

extern struct cred *mw_cred;
extern bool mw_late_loaded;
extern bool allow_shell;
#ifdef MODULE
extern bool mw_bundled;
#endif
extern struct selinux_policy *backup_sepolicy;
extern bool mw_no_custom_rc;

static inline int startswith(char *s, char *prefix)
{
    return strncmp(s, prefix, strlen(prefix));
}

static inline int endswith(const char *s, const char *t)
{
    size_t slen = strlen(s);
    size_t tlen = strlen(t);
    if (tlen > slen)
        return 1;
    return strcmp(s + slen - tlen, t);
}

#endif
