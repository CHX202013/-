#!/bin/sh
# 生成「马尾」管理器签名密钥，并把证书指纹写进 kernel/Kbuild。
#
# 内核通过 v2 签名证书的 (size, sha256) 识别管理器：签名对不上，
# 内核不会把它当 manager（su 仍可用，但模块/授权管理会受限）。
#
# 用法:
#   ./scripts/gen-manager-key.sh [keystore路径]
# 环境变量:
#   ALIAS=mawei  STOREPASS=...  KEYPASS=...
set -eu

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
KEYSTORE="${1:-$REPO_ROOT/mawei-release.jks}"
ALIAS="${ALIAS:-mawei}"
STOREPASS="${STOREPASS:-$(openssl rand -hex 16 2>/dev/null || echo changeit)}"
KEYPASS="${KEYPASS:-$STOREPASS}"
DNAME="${DNAME:-CN=Mawei, OU=System, O=System, L=NA, ST=NA, C=NA}"

command -v keytool >/dev/null 2>&1 || { echo "keytool not found: install a JDK 21"; exit 1; }

if [ -e "$KEYSTORE" ]; then
    echo "[!] $KEYSTORE already exists, reusing it"
else
    keytool -genkeypair \
        -keystore "$KEYSTORE" -storetype PKCS12 \
        -alias "$ALIAS" -keyalg RSA -keysize 2048 -validity 36500 \
        -storepass "$STOREPASS" -keypass "$KEYPASS" -dname "$DNAME"
    echo "[+] keystore created: $KEYSTORE"
fi

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT
keytool -exportcert -keystore "$KEYSTORE" -alias "$ALIAS" \
    -storepass "$STOREPASS" -file "$TMPDIR/cert.der" >/dev/null

SIZE_BYTES=$(wc -c < "$TMPDIR/cert.der" | tr -d ' ')
SIZE_HEX=$(printf '0x%04x' "$SIZE_BYTES")
if command -v sha256sum >/dev/null 2>&1; then
    HASH=$(sha256sum "$TMPDIR/cert.der" | cut -d' ' -f1)
else
    HASH=$(openssl dgst -sha256 "$TMPDIR/cert.der" | awk '{print $NF}')
fi

echo "[+] cert size : $SIZE_HEX ($SIZE_BYTES bytes)"
echo "[+] cert sha256: $HASH"

KBUILD="$REPO_ROOT/kernel/Kbuild"
sed -i "s|^MW_EXPECTED_SIZE ?=.*|MW_EXPECTED_SIZE ?= $SIZE_HEX|" "$KBUILD"
sed -i "s|^MW_EXPECTED_HASH ?=.*|MW_EXPECTED_HASH ?= $HASH|" "$KBUILD"
echo "[+] kernel/Kbuild updated"

cat <<MSG

Next steps:
  1. Build the manager with this key:
       cd manager
       ./gradlew assembleRelease \\
         -PKEYSTORE_FILE=$KEYSTORE -PKEY_ALIAS=$ALIAS \\
         -PKEYSTORE_PASSWORD=$STOREPASS -PKEY_PASSWORD=$KEYPASS
  2. Build the kernel with the same fingerprint:
       cd kernel && ./build-all.sh   # Kbuild now contains it
  3. Keep $KEYSTORE safe. Every future build must use it,
     otherwise the kernel will reject the manager.
MSG
