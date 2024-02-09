#!/bin/sh
cd "$(dirname "$0")"
LOG_PREFIX="[RGB-D overlay module setup]"

echo "$LOG_PREFIX Starting the module."

os=$(uname -s)
arch=$(uname -m)
appimage_path="./viam-camera-rgb-d-overlay--aarch64.AppImage"
# Run appimage if Linux aarch64
if [ "$os" = "Linux" ] && [ "$arch" = "aarch64" ] && [ -f "$appimage_path" ]; then
    echo "$LOG_PREFIX Detected system Linux AArch64 and appimage. Attempting to start appimage."
    chmod +x "$appimage_path"
    exec "$appimage_path" "$@"
else
    echo "$LOG_PREFIX No usable appimage was found. Attempting to execute binary"

    macos_binary_path="./rgb-d-overlay"
    chmod +x "$macos_binary_path"
    exec "$macos_binary_path" "$@"
fi

