#!/usr/bin/env python3
"""马尾源码树自检：不编译，只查结构性错误（改名残留 / 资源缺失 / 包路径不一致）。"""
import os, re, sys
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
errors = []

# 1. XML
for dp, dns, fns in os.walk("."):
    dns[:] = [d for d in dns if d not in (".git", "build", "target")]
    for f in fns:
        if f.endswith(".xml"):
            try:
                ET.parse(os.path.join(dp, f))
            except Exception as e:
                errors.append(f"XML broken: {dp}/{f}: {e}")

# 2. Kotlin package vs path
for dp, dns, fns in os.walk("manager/app/src/main/java"):
    for f in fns:
        if not f.endswith(".kt"):
            continue
        p = os.path.join(dp, f)
        m = re.search(r"^package\s+([\w.]+)", open(p, encoding="utf-8").read(), re.M)
        if not m:
            errors.append(f"no package: {p}")
            continue
        expect = os.path.relpath(dp, "manager/app/src/main/java").replace(os.sep, ".")
        if m.group(1) != expect:
            errors.append(f"package mismatch: {p}: {m.group(1)} != {expect}")

# 3. R.string references
res = {e.get("name") for e in ET.parse("manager/app/src/main/res/values/strings.xml").getroot()}
allowed_missing = {"app_name", "ok", "cancel", "dialog_alert_title"}  # gradle resValue / 依赖库提供
used = set()
for dp, dns, fns in os.walk("manager/app/src/main/java"):
    for f in fns:
        if f.endswith(".kt"):
            used |= set(re.findall(r"R\.string\.(\w+)",
                                   open(os.path.join(dp, f), encoding="utf-8").read()))
missing = used - res - allowed_missing
if missing:
    errors.append(f"missing strings: {sorted(missing)}")

# 4. AIDL filename vs interface
for dp, dns, fns in os.walk("manager/app/src/main/aidl"):
    for f in fns:
        if f.endswith(".aidl"):
            m = re.search(r"interface\s+(\w+)", open(os.path.join(dp, f), encoding="utf-8").read())
            if m and m.group(1) + ".aidl" != f:
                errors.append(f"aidl name mismatch: {f}")

# 5. leftover upstream identifiers (docs/NOTICE 允许)
for dp, dns, fns in os.walk("."):
    dns[:] = [d for d in dns if d not in (".git", "build", "target")]
    for f in fns:
        if f.endswith((".png", ".webp", ".so", ".jar", ".ttf")):
            continue
        p = os.path.join(dp, f)
        if p.startswith(("./README", "./docs/", "./NOTICE", "./LICENSE", "./scripts/check_tree")):
            continue
        try:
            s = open(p, encoding="utf-8").read()
        except Exception:
            continue
        for token in ("me.weishu", "0xDEADBEEF", "0xCAFEBABE", "patreon.com/weishu"):
            if token in s:
                errors.append(f"leftover {token}: {p}")

if errors:
    print("\n".join(f"[!] {e}" for e in errors))
    sys.exit(1)
print(f"[+] tree ok: {len(res)} strings, {len(used)} references checked")
