#!/usr/bin/env bash
set -euo pipefail

sdk_commit=f9d78aff31a5f2521ae7ddbdc97c4a8855808959
sdk_tag=26.02
root="$(git rev-parse --show-toplevel)"
target="$root/thirdparty/lzma-sdk-26.02/C"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

git clone --quiet --filter=blob:none --no-checkout https://github.com/ip7z/7zip.git "$tmp/7zip"
git -C "$tmp/7zip" checkout --quiet --detach "$sdk_commit"

rm -rf "$root/thirdparty/lzma-sdk-26.02"
mkdir -p "$target"

files=(
  7z.h
  7zAlloc.c 7zAlloc.h
  7zArcIn.c
  7zBuf.c 7zBuf.h 7zBuf2.c
  7zCrc.c 7zCrc.h 7zCrcOpt.c
  7zDec.c
  7zFile.c 7zFile.h
  7zStream.c 7zTypes.h
  Bcj2.c Bcj2.h
  Bra.c Bra.h Bra86.c BraIA64.c
  Compiler.h CpuArch.c CpuArch.h
  Delta.c Delta.h
  Lzma2Dec.c Lzma2Dec.h
  LzmaDec.c LzmaDec.h
  Ppmd.h Ppmd7.c Ppmd7.h Ppmd7Dec.c
  Precomp.h
)

for file in "${files[@]}"; do
  cp "$tmp/7zip/C/$file" "$target/$file"
done

cat > "$root/thirdparty/lzma-sdk-26.02/README.ares" <<EOF
7-Zip ANSI-C decoder subset
==========================

Source: https://github.com/ip7z/7zip
Release tag: $sdk_tag
Pinned upstream commit: $sdk_commit

The files in C/ are copied byte-for-byte from that upstream commit and are
public-domain code by Igor Pavlov as stated in the source headers. Only the
read-only 7z parser/decoder sources needed by ares are compiled.

This copy is intentionally vendored so Debian/Ubuntu source packages are fully
self-contained and Launchpad builds never need network access.
EOF

printf 'Vendored 7-Zip %s (%s) decoder subset.\n' "$sdk_tag" "$sdk_commit"
