#ifndef __MW_H_UID_OBSERVER
#define __MW_H_UID_OBSERVER

#include <linux/types.h>
#ifdef CONFIG_MW_DISABLE_MANAGER
static inline void mw_throne_tracker_init()
{
}

static inline void mw_throne_tracker_exit()
{
}

static inline void track_throne(bool prune_only)
{
    (void)prune_only;
}
#else
void mw_throne_tracker_init();

void mw_throne_tracker_exit();

void track_throne(bool prune_only);
#endif

#endif
