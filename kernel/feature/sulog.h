#ifndef __MW_H_SULOG
#define __MW_H_SULOG

#include <linux/types.h>

bool mw_sulog_is_enabled(void);
void mw_sulog_init(void);
void mw_sulog_exit(void);

#endif
