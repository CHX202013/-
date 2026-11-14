#include <linux/cache.h>
#include <linux/compiler_types.h>

#include "feature/sulog.h"
#include "klog.h" // IWYU pragma: keep
#include "policy/feature.h"
#include "sulog/event.h"
#include "sulog/fd.h"

static bool mw_sulog_enabled __read_mostly = false;

static int sulog_feature_get(u64 *value)
{
    *value = mw_sulog_enabled ? 1 : 0;
    return 0;
}

static int sulog_feature_set(u64 value)
{
    bool enable = value != 0;

    mw_sulog_enabled = enable;
    pr_info("sulog: set to %d\n", enable);
    return 0;
}

static const struct mw_feature_handler sulog_handler = {
    .feature_id = MW_FEATURE_SULOG,
    .name = "sulog",
    .get_handler = sulog_feature_get,
    .set_handler = sulog_feature_set,
};

bool mw_sulog_is_enabled(void)
{
    return mw_sulog_enabled;
}

void __init mw_sulog_init(void)
{
    int ret;

    mw_sulog_enabled = false;

    ret = mw_register_feature_handler(&sulog_handler);
    if (ret) {
        pr_err("Failed to register sulog feature handler\n");
        return;
    }

    ret = mw_sulog_events_init();
    if (ret) {
        pr_err("Failed to initialize sulog events: %d\n", ret);
        mw_unregister_feature_handler(MW_FEATURE_SULOG);
        return;
    }

    mw_sulog_fd_init();
}

void __exit mw_sulog_exit(void)
{
    mw_sulog_fd_exit();
    mw_sulog_events_exit();
    mw_unregister_feature_handler(MW_FEATURE_SULOG);
}
