#!/bin/sh
set -eu

GKI_ROOT=$(pwd)
# 默认取本仓库的 kernel/ 目录（脚本所在目录的上一级）；可用环境变量覆盖
MAWEI_SRC="${MAWEI_SRC:-$(cd "$(dirname "$0")/.." && pwd)}"

display_usage() {
    echo "Usage: $0 [--cleanup]"
    echo "  --cleanup:  还原本脚本对内核树做的修改"
    echo "  (no args):  把本仓库 kernel/ 接入当前内核树"
    echo "  环境变量 MAWEI_SRC 指向马尾仓库根目录"
}

initialize_variables() {
    if test -d "$GKI_ROOT/common/drivers"; then
         DRIVER_DIR="$GKI_ROOT/common/drivers"
    elif test -d "$GKI_ROOT/drivers"; then
         DRIVER_DIR="$GKI_ROOT/drivers"
    else
        echo '[ERROR] "drivers/" directory not found.'
        exit 127
    fi
    DRIVER_MAKEFILE=$DRIVER_DIR/Makefile
    DRIVER_KCONFIG=$DRIVER_DIR/Kconfig
}

perform_cleanup() {
    echo "[+] Cleaning up..."
    [ -L "$DRIVER_DIR/mawei" ] && rm "$DRIVER_DIR/mawei" && echo "[-] Symlink removed."
    grep -q "mawei" "$DRIVER_MAKEFILE" 2>/dev/null && sed -i '/mawei/d' "$DRIVER_MAKEFILE" && echo "[-] Makefile reverted."
    grep -q "drivers/mawei/Kconfig" "$DRIVER_KCONFIG" 2>/dev/null && sed -i '/drivers\/mawei\/Kconfig/d' "$DRIVER_KCONFIG" && echo "[-] Kconfig reverted."
    echo '[-] Done.'
}

setup_mawei() {
    test -d "$MAWEI_SRC/kernel" || { echo "[ERROR] kernel/ not found under $MAWEI_SRC"; exit 1; }
    echo "[+] Setting up Mawei from $MAWEI_SRC/kernel"
    cd "$DRIVER_DIR"
    ln -sfn "$(realpath --relative-to="$DRIVER_DIR" "$MAWEI_SRC/kernel")" "mawei" && echo "[+] Symlink created."

    grep -q "mawei" "$DRIVER_MAKEFILE" || { printf "\nobj-\$(CONFIG_MW) += mawei/\n" >> "$DRIVER_MAKEFILE"; echo "[+] Modified Makefile."; }
    grep -q "source \"drivers/mawei/Kconfig\"" "$DRIVER_KCONFIG" || { sed -i "/endmenu/i\source \"drivers/mawei/Kconfig\"" "$DRIVER_KCONFIG"; echo "[+] Modified Kconfig."; }
    echo '[+] Done.'
}

if [ "$#" -eq 0 ]; then
    initialize_variables
    setup_mawei
elif [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    display_usage
elif [ "$1" = "--cleanup" ]; then
    initialize_variables
    perform_cleanup
else
    display_usage
    exit 1
fi
