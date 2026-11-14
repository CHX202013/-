# 马尾 · 构建与签名

管理器以内核里的「证书指纹」认亲：**内核的 `MW_EXPECTED_HASH` 必须等于管理器的签名证书 SHA256**，
否则内核只提供 shell 级 root，不提供模块/授权管理。所以第一步永远是生成自己的密钥。

## 0. 依赖

| 组件 | 版本 | 用途 |
| --- | --- | --- |
| JDK | 21 | Gradle / AGP |
| Android SDK | compileSdk 37, build-tools 37.0.0 | 管理器 |
| NDK | 见 `manager/gradle/libs.versions.toml` `ndk` | JNI |
| Rust | stable + `aarch64-linux-android` 等 target | `mwd` / `mwinit` |
| DDK | 见 `scripts/prepare-ddk-x64.sh` | 编内核模块 |
| 内核源码 | GKI KMI 对应版本 | 内建模式 |

## 1. 生成密钥（只做一次）

```bash
./scripts/gen-manager-key.sh
# 输出 cert size / sha256，并自动改写 kernel/Kbuild 的
# MW_EXPECTED_SIZE / MW_EXPECTED_HASH
```

把 keystore 收好。换密钥 = 内核不认新管理器，必须一起重编内核。

## 2. 构建管理器

```bash
cd manager
./gradlew assembleRelease \
  -PKEYSTORE_FILE=../mawei-release.jks -PKEY_ALIAS=mawei \
  -PKEYSTORE_PASSWORD=xxx -PKEY_PASSWORD=xxx
```

产物：`manager/app/build/outputs/apk/release/`。

**改包名 / 应用名**（隐藏性核心，可每次发版不同）：

```bash
./gradlew assembleRelease -PMW_PACKAGE_NAME=com.example.svc -PMW_NAME=系统服务
```

内核必须同步：

```bash
cd kernel
./build-all.sh          # 或内建时 make ... MW_PACKAGE_NAME=com.example.svc
```

`MW_PACKAGE_NAME` 从 `kernel/Kbuild` 读取，`build-all.sh` 通过 `ddk build ... -e MW_PACKAGE_NAME=...` 传递。

## 3. 构建内核

**LKM（可加载模块，好调试）**

```bash
cd kernel
./build-all.sh                 # 需要 ddk，产出 mawei-<kmi>.ko
```

**内建（推荐，隐藏性最好：`lsmod` 里没有模块）**

```bash
cd <GKI 内核源码根>
MAWEI_SRC=/path/to/mawei ./path/to/mawei/kernel/setup.sh
# 打开 CONFIG_MW=y 后正常编内核
```

## 4. 构建 mwd

```bash
cargo build --release --target aarch64-linux-android
# 产物会作为 libmwd.so 打进 APK 的 jniLibs（见 justfile）
```

## 5. 没有本地工具链？

仓库自带 GitHub Actions：

- `build-manager.yml`：APK + LKM + mwd + mwinit，全部产出 artifact
- `ddk-lkm.yml`：按 KMI 批量出 `.ko`
- 不再有 Telegram / 外网上传

push 到自己的仓库，开 Actions 即可。**先改 `MW_PACKAGE_NAME` 和密钥**，否则用的还是默认身份。

## 6. 安装

1. App 首页「安装」→ 选择 boot.img / 或用 LKM 模式；
2. 刷入后首次开机，App 会自动把备份镜像放到 `/data/adb/mw`；
3. 出问题用 recovery 删模块目录，或 `fastboot flash boot stock.img` 回滚。

## 7. 常见坑

- **内核不认管理器**：证书指纹不一致。检查 `kernel/Kbuild` 的 HASH 与 APK 实际签名证书。
- **构建报 git 错误**：源码包无 `.git` 时已用回退版本号；若仍失败，`git init` 后提交一次。
- **LKM 加载失败**：KMI 不匹配，用 `ddk-lkm.yml` 出对应 KMI 的 `.ko`。
- **隐藏图标后找不回**：快捷方式没钉上时脚本会拒绝隐藏；真丢了就用
  `adb shell am start -n <包名>/.ui.MainActivity` 拉起。
