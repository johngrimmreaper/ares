#!/bin/sh

LC_ALL=C
export LC_ALL

if /lib64/ld-linux-x86-64.so.2 --help | grep -q 'x86-64-v2.*supported'; then
    exec /usr/libexec/ares/ares-simd "$@"
else
    exec /usr/libexec/ares/ares-no-simd "$@"
fi
