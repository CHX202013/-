#ifndef __MW_H_SULOG_EVENT
#define __MW_H_SULOG_EVENT

#include <linux/compiler_types.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include "uapi/sulog.h" // IWYU pragma: keep

struct mw_event_queue;
struct mw_sulog_pending_event;

int mw_sulog_events_init(void);
void mw_sulog_events_exit(void);

struct mw_sulog_pending_event *mw_sulog_capture_root_execve(const char __user *filename_user,
                                                              const char __user *const __user *argv_user, gfp_t gfp);
struct mw_sulog_pending_event *mw_sulog_capture_sucompat(const char __user *filename_user,
                                                           const char __user *const __user *argv_user, gfp_t gfp);
void mw_sulog_emit_pending(struct mw_sulog_pending_event *pending, int retval, gfp_t gfp);
int mw_sulog_emit_grant_root(int retval, __u32 uid, __u32 euid, gfp_t gfp);

struct mw_event_queue *mw_sulog_get_queue(void);

#endif
