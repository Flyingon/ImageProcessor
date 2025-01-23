#!/bin/bash

check_app() {
    local app_path="$1"
    echo "正在检查应用程序完整性..."
    
    # 检查基本结构
    if [ ! -f "$app_path/Contents/MacOS/ImageProcessor" ]; then
        echo "错误: 可执行文件不存在"
        return 1
    fi
    
    # 检查依赖库
    echo "检查依赖库..."
    local missing_libs=0
    while IFS= read -r lib; do
        if [[ $lib == *"@rpath"* ]]; then
            lib_name=$(basename "$lib")
            if [ ! -f "$app_path/Contents/Frameworks/$lib_name" ]; then
                echo "缺失: $lib_name"
                missing_libs=1
            fi
        fi
    done < <(otool -L "$app_path/Contents/MacOS/ImageProcessor" | tail -n +2 | awk '{print $1}')
    
    if [ $missing_libs -eq 1 ]; then
        echo "警告: 存在缺失的依赖库"
        return 1
    fi
    
    # 检查 Qt 插件
    if [ ! -d "$app_path/Contents/PlugIns" ]; then
        echo "警告: Qt 插件目录不存在"
        missing_libs=1
    fi
    
    # 检查 OpenCV 依赖
    echo "检查 OpenCV 依赖..."
    if ! otool -L "$app_path/Contents/MacOS/ImageProcessor" | grep -q "opencv"; then
        echo "警告: 未找到 OpenCV 依赖"
        missing_libs=1
    fi
    
    if [ $missing_libs -eq 0 ]; then
        echo "应用程序打包完整性检查通过"
        return 0
    else
        return 1
    fi
}

# 如果直接运行此脚本
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    if [ -z "$1" ]; then
        echo "用法: $0 <应用程序路径>"
        echo "例如: $0 build/ImageProcessor.app"
        exit 1
    fi
    
    if ! check_app "$1"; then
        echo "检查失败，请修复上述问题"
        exit 1
    fi
    
    echo "建议测试步骤："
    echo "1. 挂载 dmg 文件"
    echo "2. 将应用拖到其他位置（如桌面）"
    echo "3. 尝试运行应用"
fi