#ifndef __MW_H_SEPOLICY
#define __MW_H_SEPOLICY

#include <linux/types.h>

#include "ss/policydb.h"

struct selinux_policy *mw_dup_sepolicy(struct selinux_policy *old_pol);

void mw_destroy_sepolicy(struct selinux_policy *orig);

// Operation on types
bool mw_type(struct policydb *db, const char *name, const char *attr);
bool mw_attribute(struct policydb *db, const char *name);
bool mw_permissive(struct policydb *db, const char *type);
bool mw_enforce(struct policydb *db, const char *type);
bool mw_typeattribute(struct policydb *db, const char *type, const char *attr);
bool mw_exists(struct policydb *db, const char *type);

// Return 1 when the type value is from the initial policy, 0 when it is not, and -1 when metadata is unavailable
int mw_type_is_original(struct policydb *db, u32 policy_generation, u32 type_value);
bool mw_set_original_types(struct policydb *db, struct policydb *orig_db, u32 policy_generation);
bool mw_advance_original_types(struct policydb *db, struct policydb *old_db, u32 old_policy_generation,
                                u32 policy_generation);

// Access vector rules
bool mw_allow(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool mw_deny(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool mw_auditallow(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool mw_dontaudit(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);

// Extended permissions access vector rules
bool mw_allowxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);
bool mw_auditallowxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);
bool mw_dontauditxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);

// Type rules
bool mw_type_transition(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def,
                         const char *obj);
bool mw_type_change(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def);
bool mw_type_member(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def);

// File system labeling
bool mw_genfscon(struct policydb *db, const char *fs_name, const char *path, const char *ctx);

#endif
