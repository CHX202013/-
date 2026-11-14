//! Built-in payload modules (Zygisk Next / LSPosed).
//!
//! These modules are provisioned automatically so that a fresh Mawei
//! installation already has a working Zygisk provider and Xposed framework,
//! without the user having to flash anything else.

use crate::defs;
use crate::module;
use crate::utils;
use anyhow::{bail, Context, Result};
use log::{info, warn};
use sha256::digest;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

const PAYLOAD_DIR: &str = "/data/adb/mw/payload";
const PAYLOAD_DISABLE: &str = "/data/adb/mw/payload/disable";
const LAST_ATTEMPT: &str = "/data/adb/mw/payload/.last_attempt";
const RETRY_INTERVAL: Duration = Duration::from_secs(6 * 60 * 60);

struct BuiltinModule {
    id: &'static str,
    name: &'static str,
    urls: &'static [&'static str],
    sha256: &'static str,
}

const BUILTIN: &[BuiltinModule] = &[
    BuiltinModule {
        id: "zygisksu",
        name: "Zygisk Next",
        urls: &["https://github.com/LSPosed/ZygiskNext/releases/download/v1.5.3/Zygisk-Next-1.5.3-880-22a1bf5-release.zip"],
        sha256: "186a0a26c9fcb895af1954b42de831433c23d72d1bd0758b986069154359db67",
    },
    BuiltinModule {
        id: "zygisk_lsposed",
        name: "LSPosed",
        urls: &[
            "https://github.com/JingMatrix/Vector/releases/download/v1.11.0/LSPosed-v1.11.0-7209-zygisk-release.zip",
            "https://lsposed.cn/lsposed-down/LSPosed-v1.11.0-7209-zygisk-release.zip",
        ],
        sha256: "6b8455e08dfa13a8121619fe013a79b40e3e5a1291555c91cc7ee1870a56b65c",
    },
];

static BYPASS_BOOT_CHECK: AtomicBool = AtomicBool::new(false);

/// Allows the module installer to run before `sys.boot_completed` is set.
#[must_use]
pub fn bypass_boot_check() -> bool {
    BYPASS_BOOT_CHECK.load(Ordering::Relaxed)
}

fn set_bypass(value: bool) {
    BYPASS_BOOT_CHECK.store(value, Ordering::Relaxed);
}

fn removed_marker(id: &str) -> PathBuf {
    Path::new(PAYLOAD_DIR).join(format!("{id}.removed"))
}

/// Remember that the user uninstalled a built-in module, so it is not
/// reinstalled on the next boot.
pub fn mark_user_removed(id: &str) {
    if !BUILTIN.iter().any(|m| m.id == id) {
        return;
    }
    if let Err(e) = fs::create_dir_all(PAYLOAD_DIR) {
        warn!("payload: create dir failed: {e}");
        return;
    }
    if let Err(e) = fs::write(removed_marker(id), b"1") {
        warn!("payload: write removed marker failed: {e}");
    }
}

fn should_attempt() -> bool {
    if Path::new(PAYLOAD_DISABLE).exists() {
        return false;
    }
    let Ok(meta) = fs::metadata(LAST_ATTEMPT) else {
        return true;
    };
    let Ok(modified) = meta.modified() else {
        return true;
    };
    SystemTime::now()
        .duration_since(modified)
        .is_ok_and(|elapsed| elapsed >= RETRY_INTERVAL)
}

fn touch_attempt() {
    let _ = fs::create_dir_all(PAYLOAD_DIR);
    let epoch = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_secs())
        .unwrap_or(0);
    let _ = fs::write(LAST_ATTEMPT, epoch.to_string());
}

fn wget(url: &str, out: &Path) -> Result<()> {
    let busybox = format!("{}busybox", defs::BINARY_DIR);
    let status = Command::new(&busybox)
        .args(["wget", "-q", "--no-check-certificate", "-T", "60", "-t", "1", "-O"])
        .arg(out)
        .arg(url)
        .status()
        .with_context(|| format!("failed to spawn {busybox}"))?;
    if !status.success() {
        bail!("wget exited with {status} for {url}");
    }
    Ok(())
}

fn file_sha256(path: &Path) -> Result<String> {
    let bytes = fs::read(path).with_context(|| format!("read {}", path.display()))?;
    Ok(digest(bytes))
}

fn fetch(m: &BuiltinModule) -> Result<PathBuf> {
    let path = Path::new(PAYLOAD_DIR).join(format!("{}.zip", m.id));
    if path.exists()
        && file_sha256(&path).is_ok_and(|sum| sum.eq_ignore_ascii_case(m.sha256))
    {
        return Ok(path);
    }
    fs::create_dir_all(PAYLOAD_DIR).with_context(|| "create payload dir")?;
    for url in m.urls {
        let _ = fs::remove_file(&path);
        info!("payload: downloading {} from {url}", m.name);
        if let Err(e) = wget(url, &path) {
            warn!("payload: {} download failed: {e}", m.name);
            continue;
        }
        match file_sha256(&path) {
            Ok(sum) if sum.eq_ignore_ascii_case(m.sha256) => return Ok(path),
            Ok(sum) => warn!("payload: {} checksum mismatch: {sum}", m.name),
            Err(e) => warn!("payload: {} checksum read failed: {e}", m.name),
        }
    }
    bail!("could not download {}", m.name)
}

fn activate(m: &BuiltinModule) -> Result<()> {
    let staged = Path::new(defs::MODULE_UPDATE_DIR).join(m.id);
    let live = Path::new(defs::MODULE_DIR).join(m.id);
    if !staged.exists() {
        bail!("staged module {} missing", m.id);
    }
    if live.exists() {
        fs::remove_dir_all(&live).with_context(|| format!("remove {}", live.display()))?;
    }
    fs::rename(&staged, &live).with_context(|| format!("activate {}", m.id))?;
    let _ = fs::remove_file(live.join(defs::UPDATE_FILE_NAME));
    let _ = fs::remove_file(live.join("disable"));
    let _ = fs::remove_file(live.join("remove"));
    let _ = crate::restorecon::lsetfilecon(&live, "u:object_r:system_file:s0");
    if !live.join("module.prop").exists() {
        bail!("module {} has no module.prop", m.id);
    }
    Ok(())
}

fn install(m: &BuiltinModule) -> Result<()> {
    let zip = fetch(m)?;
    set_bypass(true);
    let result = module::install_module(zip.to_string_lossy().as_ref());
    set_bypass(false);
    result.with_context(|| format!("install {}", m.name))?;
    activate(m).with_context(|| format!("activate {}", m.name))?;
    info!("payload: {} installed and activated", m.name);
    Ok(())
}

fn missing_builtin() -> Vec<&'static BuiltinModule> {
    BUILTIN
        .iter()
        .filter(|m| {
            !Path::new(defs::MODULE_DIR).join(m.id).exists()
                && !removed_marker(m.id).exists()
        })
        .collect()
}

/// Install missing built-in modules. Called during `post-fs-data`, before the
/// module scripts run, so a freshly flashed device has Zygisk/LSPosed active
/// from the very first boot.
pub fn provision() {
    if utils::is_safe_mode() {
        return;
    }
    let missing = missing_builtin();
    if missing.is_empty() || !should_attempt() {
        return;
    }
    info!("payload: {} built-in module(s) missing", missing.len());
    for m in &missing {
        if let Err(e) = install(m) {
            warn!("payload: {} provisioning failed: {e:#}", m.name);
        }
    }
    touch_attempt();
    if let Err(e) = module::regenerate_preinit() {
        warn!("payload: regenerate preinit failed: {e:#}");
    }
}

/// Install all missing built-in modules now (manual/forced variant).
pub fn install_all() -> Result<()> {
    for m in BUILTIN {
        let live = Path::new(defs::MODULE_DIR).join(m.id);
        if live.exists() && !should_attempt() {
            println!("- {} already installed", m.name);
            continue;
        }
        install(m)?;
        println!("- {} installed", m.name);
    }
    touch_attempt();
    module::regenerate_preinit()?;
    Ok(())
}

pub fn status() -> Result<()> {
    for m in BUILTIN {
        let live = Path::new(defs::MODULE_DIR).join(m.id);
        let cached = Path::new(PAYLOAD_DIR).join(format!("{}.zip", m.id));
        let state = if live.exists() {
            "installed"
        } else if removed_marker(m.id).exists() {
            "removed by user"
        } else if cached.exists() {
            "cached, not installed"
        } else {
            "not installed"
        };
        println!("- {} ({}): {}", m.name, m.id, state);
    }
    Ok(())
}
