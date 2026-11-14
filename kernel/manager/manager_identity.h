#ifndef __MW_H_MANAGER_IDENTITY
#define __MW_H_MANAGER_IDENTITY

#include <linux/cred.h>
#include <linux/types.h>

#define MW_INVALID_APPID -1
#define MW_PER_USER_RANGE 100000

#ifdef CONFIG_MW_DISABLE_MANAGER
static inline bool mw_is_manager_appid_valid()
{
    return true;
}

static inline bool is_manager()
{
    return current_uid().val == 0;
}

static inline bool is_uid_manager(uid_t uid)
{
    return uid == 0;
}

static inline uid_t mw_get_manager_appid()
{
    return 0;
}

static inline void mw_set_manager_appid(uid_t appid)
{
    (void)appid;
}

static inline void mw_invalidate_manager_uid()
{
}
#else
extern uid_t mw_manager_appid; // DO NOT DIRECT USE

static inline bool mw_is_manager_appid_valid()
{
    return mw_manager_appid != MW_INVALID_APPID;
}

static inline bool is_manager()
{
    return unlikely(mw_manager_appid == current_uid().val % MW_PER_USER_RANGE);
}

static inline bool is_uid_manager(uid_t uid)
{
    return unlikely(mw_manager_appid == uid % MW_PER_USER_RANGE);
}

static inline uid_t mw_get_manager_appid()
{
    return mw_manager_appid;
}

static inline void mw_set_manager_appid(uid_t appid)
{
    mw_manager_appid = appid;
}

static inline void mw_invalidate_manager_uid()
{
    mw_manager_appid = MW_INVALID_APPID;
}
#endif

#endif // __MW_H_MANAGER_IDENTITY
