# 马尾（Mawei）

安卓内核级 root 管理器。界面对齐 KernelSU，隐藏性做加强版。

本项目是 **KernelSU 3.3.1 的完整派生版**：内核驱动 + 用户态守护进程（`mwd`）+ 管理器 App（Jetpack Compose），
把标识、包名、路径、符号、日志、协议魔数全部换成「马尾」体系，并在上游基础上加了一层隐藏与防检测设计。

> **为什么是派生而不是从零写**
> 一个可用的内核级 root 管理器 = 内核驱动（syscall hook、SELinux、mount namespace、allowlist）+
> 启动镜像 patch + 守护进程 + 管理器 App，上游累计约 800 个文件、数万行。
> 从零写等于重写整个 KernelSU，没有工程意义；KernelSU 是 GPL，派生 + 重构 + 加固是正确做法。
> 派生不改变许可证：内核部分 GPL-2.0，用户态/管理器 GPL-3.0，见 `LICENSE`、`kernel/LICENSE`、`NOTICE`。

![马尾架构](docs/architecture.svg)

## 组成

| 组件 | 目录 | 说明 |
| --- | --- | --- |
| 内核驱动 | `kernel/` | `CONFIG_MW`，KMI 覆盖 android12-5.10 ~ android17-6.18；可内建（推荐）或 LKM |
| 守护进程 | `userspace/mwd/` | Rust 编写，安装到 `/data/adb/mwd`，负责模块、su、boot patch |
| 早期注入 | `userspace/mwinit/` | 内核 preinit 阶段拉起 |
| 管理器 App | `manager/` | Compose + Material3/Miuix 双主题，界面同 KSU |
| 对外 UAPI | `uapi/` | 内核与用户态共享的 ioctl 协议 |

## 快速开始

```bash
# 1. 生成自己的签名密钥（内核按证书 SHA256 认管理器，必须和 APK 签名一致）
./scripts/gen-manager-key.sh

# 2. 构建 App（需要 JDK 21 + Android SDK）
cd manager && ./gradlew assembleRelease

# 3. 构建内核模块（需要 DDK 或完整内核源码）
cd kernel && ./build-all.sh

# 4. 刷入：用 App 的「安装」功能 patch boot.img，或 LKM 模式加载 .ko
```

没有本地工具链时交给 CI：`.github/workflows/build-manager.yml` 会构建 APK / LKM / mwd 三件套。

## 隐藏性设计（摘要）

完整威胁模型与「已实现 / 未实现」清单见 [`docs/stealth.md`](docs/stealth.md)。

- **包名随机化**：`-PMW_PACKAGE_NAME=com.xxx.yyy`，内核 `MW_PACKAGE_NAME` 同步；App 与内核双向绑定包名 + 证书指纹
- **桌面图标可隐藏**：设置页一键隐藏（activity-alias 禁用 + 自动钉快捷方式），`FLAG_SECURE` 防截屏/防最近任务缩略图
- **协议魔数替换**：`0xDEADBEEF/0xCAFEBABE` → 非特征值，扫描器按旧魔数找不到
- **内核日志去标识**：`pr_fmt` 前缀清空、启动横幅删除、模块 author 去除
- **推荐内建模式**：`CONFIG_MW=y` 进内核，`lsmod` 里根本没有这个模块
- **上游网络行为默认关闭**：无遥测、无检查更新回连、无 Telegram 上报（已从 CI 移除）

## 目录

```
kernel/      内核驱动（GPL-2.0）
userspace/   mwd / mwinit（GPL-3.0）
manager/     管理器 App（GPL-3.0）
uapi/        共享 ioctl 协议
scripts/     key 生成、DDK 准备
docs/        stealth.md / build.md
```

## 状态与风险

- 本仓库是在**无 JDK / 无 Android SDK / 无 clang 的手机环境**里完成的源码级派生与改造，
  没有在本机编译验证；构建请走 CI 或桌面环境。`docs/build.md` 有完整依赖清单。
- 刷 boot.img 有变砖风险，务必先备份 stock image。
- 「隐藏」是军备竞赛：内核模块、mount、SELinux domain、行为特征都可能被新版检测抓到，
  本项目不承诺「全平台永久不可检测」。
