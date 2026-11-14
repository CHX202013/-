#ifndef __MW_H_FEATURE
#define __MW_H_FEATURE

#include <linux/types.h>
#include "uapi/feature.h" // IWYU pragma: keep

typedef int (*mw_feature_get_t)(u64 *value);
typedef int (*mw_feature_set_t)(u64 value);

struct mw_feature_handler {
    u32 feature_id;
    const char *name;
    mw_feature_get_t get_handler;
    mw_feature_set_t set_handler;
};

int mw_register_feature_handler(const struct mw_feature_handler *handler);

int mw_unregister_feature_handler(u32 feature_id);

int mw_get_feature(u32 feature_id, u64 *value, bool *supported);

int mw_set_feature(u32 feature_id, u64 value);

// Whether the feature is forced by the force_feature module param, in which
// case mw_set_feature() refuses to change it.
bool mw_feature_is_forced(u32 feature_id);

// Apply forced feature values not applied yet. Called at several boot stages
// since some handlers can only succeed late (e.g. selinux_hide needs the
// sepolicy backup).
void mw_feature_apply_forced(void);

void mw_feature_init(void);

void mw_feature_exit(void);

#endif // __MW_H_FEATURE
