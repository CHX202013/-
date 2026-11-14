#ifndef __MW_SU_MOUNT_NS_H
#define __MW_SU_MOUNT_NS_H

#include <linux/types.h>

#define MW_NS_INHERITED 0
#define MW_NS_GLOBAL 1
#define MW_NS_INDIVIDUAL 2

struct mw_mns_tw {
    struct callback_head cb;
    int32_t ns_mode;
};

void setup_mount_ns(int32_t ns_mode);

#endif
