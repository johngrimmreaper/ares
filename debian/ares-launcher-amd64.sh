#!/bin/bash

SUPPORTS_SIMD=$(/lib64/ld-linux-x86-64.so.2 --help | grep "x86-64-v2.*supported")

if [ -n "$SUPPORTS_SIMD" ] ; then
	echo "x86-64-v2 detected. Running ares with SIMD..."
	exec /usr/libexec/ares/ares-simd $@
else
	echo "x86-64-v2 not supported. Running ares without SIMD..."
	exec /usr/libexec/ares/ares-no-simd $@
fi
