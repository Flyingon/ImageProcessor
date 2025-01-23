#!/bin/bash

# 创建临时图标目录
ICON_SET="AppIcon.iconset"
mkdir -p "$ICON_SET"

# 从原始 PNG 生成不同尺寸的图标
sips -z 16 16     icons/app.png --out "$ICON_SET/icon_16x16.png"
sips -z 32 32     icons/app.png --out "$ICON_SET/icon_16x16@2x.png"
sips -z 32 32     icons/app.png --out "$ICON_SET/icon_32x32.png"
sips -z 64 64     icons/app.png --out "$ICON_SET/icon_32x32@2x.png"
sips -z 128 128   icons/app.png --out "$ICON_SET/icon_128x128.png"
sips -z 256 256   icons/app.png --out "$ICON_SET/icon_128x128@2x.png"
sips -z 256 256   icons/app.png --out "$ICON_SET/icon_256x256.png"
sips -z 512 512   icons/app.png --out "$ICON_SET/icon_256x256@2x.png"
sips -z 512 512   icons/app.png --out "$ICON_SET/icon_512x512.png"
sips -z 1024 1024 icons/app.png --out "$ICON_SET/icon_512x512@2x.png"

# 生成 .icns 文件
iconutil -c icns "$ICON_SET"

# 清理临时文件
rm -rf "$ICON_SET"