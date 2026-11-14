#ifndef __MW_H_MANAGER_OBSERVER
#define __MW_H_MANAGER_OBSERVER

#ifdef CONFIG_MW_DISABLE_MANAGER
static inline int mw_observer_init(void)
{
    return 0;
}

static inline void mw_observer_exit(void)
{
}
#else
int mw_observer_init(void);
void mw_observer_exit(void);
#endif

#endif // __MW_H_MANAGER_OBSERVER
