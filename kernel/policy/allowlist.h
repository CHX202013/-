#ifndef __MW_H_ALLOWLIST
#define __MW_H_ALLOWLIST

#include <linux/types.h>
#include <linux/uidgid.h>
#include "uapi/app_profile.h"

#define PER_USER_RANGE 100000
#define WEBVIEW_ZYGOTE_UID 1053
#define FIRST_APPLICATION_UID 10000
#define LAST_APPLICATION_UID 19999
#define FIRST_ISOLATED_UID 99000
#define LAST_ISOLATED_UID 99999

void mw_allowlist_init(void);

void mw_allowlist_exit(void);

void mw_load_allow_list(void);

// Check if the uid is in allow list
bool __mw_is_allow_uid(uid_t uid);
#define mw_is_allow_uid(uid) unlikely(__mw_is_allow_uid(uid))

// Check if the uid is in allow list, or current is mw domain root.
// Only reflects the grant state, use mw_is_allow_uid_for_current() to
// authorize current.
bool mw_is_uid_granted_root(uid_t uid);

// mw_is_uid_granted_root() plus extra restrictions on current, e.g. shell
// may have to descend from adbd or a mw domain process
bool __mw_is_allow_uid_for_current(uid_t uid);
#define mw_is_allow_uid_for_current(uid) unlikely(__mw_is_allow_uid_for_current(uid))

bool mw_get_allow_list(int *array, u16 length, u16 *out_length, u16 *out_total, bool allow);

void mw_prune_allowlist(bool (*is_uid_exist)(uid_t, char *, void *), void *data);
void mw_persistent_allow_list();

// should be called with rcu read lock
struct app_profile *mw_get_app_profile(uid_t uid);
// only used to put the app_profile returned by mw_get_app_profile
void mw_put_app_profile(struct app_profile *);
int mw_set_app_profile(struct app_profile *);

bool mw_uid_should_umount(uid_t uid);
struct root_profile *mw_get_root_profile(uid_t uid);
// only used to put the root_profile returned by mw_get_root_profile
void mw_put_root_profile(struct root_profile *);

static inline bool is_appuid(uid_t uid)
{
    uid_t appid = uid % PER_USER_RANGE;
    return appid >= FIRST_APPLICATION_UID && appid <= LAST_APPLICATION_UID;
}

static inline bool is_isolated_process(uid_t uid)
{
    uid_t appid = uid % PER_USER_RANGE;
    return appid >= FIRST_ISOLATED_UID && appid <= LAST_ISOLATED_UID;
}
#endif
