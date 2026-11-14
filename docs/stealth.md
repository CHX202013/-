# 马尾 · 隐藏性设计与威胁模型

「隐藏性拉满」不是一句口号，得先定义清楚**谁在检测、检测什么**。这份文档分三块：
检测面、已实现、未实现（以及为什么没做）。

## 1. 检测面

| # | 层面 | 典型检测手段 | 马尾对应策略 |
| --- | --- | --- | --- |
| 1 | 安装包 | `pm list packages` 匹配 `me.weishu.kernelsu` / 图标 / 应用名 | 包名与名称构建期可变，默认 `com.mawei.manager`/马尾 |
| 2 | 桌面 | 启动器图标、最近任务缩略图 | 图标可隐藏、`FLAG_SECURE`+`excludeFromRecents` |
| 3 | 文件 | `/data/adb/ksu`、`/data/adb/ksud`、`/system/bin/su` | 已改名 `/data/adb/mw`、`/data/adb/mwd`（`/data/adb` 本身 0700，普通 App 读不到） |
| 4 | 内核 | `lsmod`、`/proc/modules`、`/proc/kallsyms` 里的 `kernelsu` 符号 | 魔数/日志/author 已换；**建议内建模式**，LKM 模式仍有痕迹 |
| 5 | 日志 | `dmesg`/`logcat` 里的 `KernelSU`、`ksu` | `pr_fmt` 前缀清空、启动横幅删除、模块描述泛化 |
| 6 | 协议 | 扫 `0xDEADBEEF/0xCAFEBABE`、`_IOC('K')` 特征 | 魔数已替换；ioctl type 字符仍是 `'K'`（见未实现 #4） |
| 7 | 行为 | mount 表、overlay、`su` exec、SELinux domain | 上游 umount / selinux_hide 能力保留；domain 仍叫 `u:r:mw:s0` |
| 8 | 网络 | 管理器回连 GitHub / 遥测 | 已删除 website / Telegram 上报 / FUNDING；检查更新默认关 |

## 2. 已实现（本轮代码改动）

**标识层（全仓 800+ 文件）**

- `KernelSU`/`kernelsu`/`ksu`/`ksud` → `Mawei`/`mawei`/`mw`/`mwd`
- Kotlin 包 `me.weishu.kernelsu` → `com.mawei.manager`，目录、JNI 函数名（`Java_com_mawei_manager_*`）、AIDL 同步
- 模块名 `kernelsu.ko` → `mawei.ko`，Kconfig `CONFIG_KSU` → `CONFIG_MW`
- 守护进程 `ksud` → `mwd`，工作目录 `/data/adb/mw`

**内核层**

- `pr_fmt` 从 `"Mawei: "` 改为空——每行 dmesg 不再带产品名
- 删除 `welcome to Mawei version ...` 启动横幅
- `MODULE_AUTHOR("weishu")` 删除，`MODULE_DESCRIPTION` 改为中性文本
- 安装魔数 `0xDEADBEEF/0xCAFEBABE` → `0x5B9D4A17/0x3E86C2D9`（内核、mwinit 两处同步）

**管理器层**

- 新增「隐藏桌面图标」开关（Material/Miuix 双设置页）：
  禁用 `activity-alias` 前先钉快捷方式，钉不了就**拒绝隐藏**，不会把自己锁死；
  隐藏后 `am start -n com.mawei.manager/.ui.MainActivity` 仍可拉起
- `FLAG_SECURE`：禁止截屏、最近任务显示空白缩略图
- `allowBackup=false`：allowlist/授权记录不进云备份
- `excludeFromRecents=true`：不出现在最近任务

**工程层**

- 删除 `website/`、`fastlane/`、`js/`、Telegram 上传步骤、`FUNDING.yml`、上游多语言 README
- CI 只保留构建（APK / LKM / mwd / mwinit），不再对外推送

## 3. 未实现（诚实清单）

| # | 项目 | 为什么没做 | 代价 |
| --- | --- | --- | --- |
| 1 | `/proc/modules` 隐藏 | 需要动 `struct module` 链表，写错必崩；LKM 模式下才需要 | 内建模式可绕过 |
| 2 | kallsyms 符号随机化 | 要引入编译期 salt + 全量符号表重写，无编译器验证风险高 | 可后续用 LLVM pass / 后处理 |
| 3 | SELinux domain 改名 | `u:r:mw:s0` 散落在内核 selinux 策略与 App 常量，改错直接无法提权 | 检测方仍可看 domain |
| 4 | ioctl type `'K'` → 随机字符 | C 与 Rust bindgen 两侧硬编码，必须同改，无编译验证 | 扫 `'K'` 特征可命中 |
| 5 | 工作目录随机化（`/data/adb/.xxx`） | `/data/adb` 0700，普通 App 不可读，收益低 | 低 |
| 6 | 反 root 检测 hook（对目标 App 隐藏 su/mount） | 属于「对抗日更」功能，需按 App 适配，且上游已有 umount/selinux_hide 基础 | 大 |
| 7 | 可卸载 LKM / 卸载清理 | 上游设计为不可卸载，改生命周期风险高 | - |

## 4. 结论

当前版本把**静态可扫描特征**（包名、文件名、符号名、魔数、日志、桌面图标）基本清干净；
**动态/行为特征**（domain、mount、进程、LKM 痕迹）保留了上游能力，属于下一阶段工作。

真正要「不可检测」，方向不是继续改名字，而是：
1. 内建进内核（消灭 LKM 痕迹）；
2. 对目标进程做 per-UID 的 mount/文件视图隔离；
3. 定期轮换魔数与签名。
