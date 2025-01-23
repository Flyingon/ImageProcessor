#!/bin/bash

# 创建构建目录
mkdir -p build
cd build

# 构建项目
cmake ..
make -j$(sysctl -n hw.ncpu)

# macOS 特定处理
if [ "$(uname)" == "Darwin" ]; then
    # 清理旧的打包文件
    echo "清理旧的打包文件..."
    rm -rf "ImageProcessor.app" "ImageProcessor.dmg"
    
    # 创建应用包
    APP_NAME="ImageProcessor.app"
    mkdir -p "$APP_NAME/Contents/"{MacOS,Frameworks,Resources}
    
    # 复制可执行文件
    cp ImageProcessor "$APP_NAME/Contents/MacOS/"

        # 复制图标文件
    cp ../resources/AppIcon.icns "$APP_NAME/Contents/Resources/"
    
    # 创建基本的 Info.plist
    cat > "$APP_NAME/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>ImageProcessor</string>
    <key>CFBundleIdentifier</key>
    <string>com.flyingon.imageprocessor</string>
    <key>CFBundleName</key>
    <string>ImageProcessor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
</dict>
</plist>
EOF
    
    # 使用 macdeployqt 处理 Qt 依赖，macdeployqt 会帮忙把所有依赖 copy 进去，包括 opencv 的，但是不能完整修改引用路径
    echo "执行 macdeployqt ..."
    macdeployqt "$APP_NAME"
    
    # 复制 OpenCV 动态库并修复路径
    # 定义一个函数来处理库的依赖
    process_library_dependencies() {
        local lib_path="$1"
        local lib_name=$(basename "$lib_path")
        echo "处理库: $lib_name"
        
        # 复制库文件（如果还没有复制）
        if [ ! -f "$APP_NAME/Contents/Frameworks/$lib_name" ]; then
            echo "补充复制依赖库: $lib_path" 
            cp "$lib_path" "$APP_NAME/Contents/Frameworks/"
        fi

        # 修改库文件自身的安装名称
        if [ ! -w "$APP_NAME/Contents/Frameworks/$lib_name" ]; then
            chmod u+w "$APP_NAME/Contents/Frameworks/$lib_name"
        fi
        install_name_tool -id "@executable_path/../Frameworks/$lib_name" "$APP_NAME/Contents/Frameworks/$lib_name"
        
        # 处理所有依赖
        echo "检查 $lib_name 的依赖..."
        otool -L "$APP_NAME/Contents/Frameworks/$lib_name" | tail -n +2 | while read -r line; do
            dep=$(echo $line | awk '{print $1}')
            original_dep="$dep"  # 保存原始路径用于 install_name_tool
            
            # 跳过系统库和已处理的库
            if [[ "$dep" == /usr/lib/* ]] || [[ "$dep" == /System/* ]] || [[ "$dep" == "@executable_path"* ]]; then
                continue
            fi
            
            dep_name=$(basename "$dep")
            echo "处理依赖: $dep_name"
            
            # 处理 @rpath 路径，仅用于查找实际文件
            if [[ "$dep" == @rpath/* ]]; then
                actual_path=""
                for search_path in "/opt/homebrew/lib" "/opt/homebrew/opt/vtk/lib" "/opt/homebrew/opt/protobuf/lib" "/usr/local/lib"; do
                    if [ -f "$search_path/$dep_name" ]; then
                        actual_path="$search_path/$dep_name"
                        break
                    fi
                done
                
                if [ -n "$actual_path" ]; then
                    # 只用实际路径来复制文件，不修改 dep
                    if [ ! -f "$APP_NAME/Contents/Frameworks/$dep_name" ]; then
                        echo "复制依赖库: $actual_path -> $APP_NAME/Contents/Frameworks/$dep_name"
                        cp "$actual_path" "$APP_NAME/Contents/Frameworks/"
                        # 递归处理新复制的依赖库
                        process_library_dependencies "$actual_path"
                    fi
                    
                    # 修改当前库中对这个依赖的引用，使用原始的 @rpath 路径
                    echo "修改依赖引用: $original_dep -> @executable_path/../Frameworks/$dep_name"
                            # Ensure the file is writable before modification
                    if [ ! -w "$APP_NAME/Contents/Frameworks/$lib_name" ]; then
                        chmod u+w "$APP_NAME/Contents/Frameworks/$lib_name"
                    fi
                    install_name_tool -change "$original_dep" "@executable_path/../Frameworks/$dep_name" "$APP_NAME/Contents/Frameworks/$lib_name"
                fi
            else
                # 处理非 @rpath 路径的情况
                if [ -f "$dep" ]; then
                    if [ ! -f "$APP_NAME/Contents/Frameworks/$dep_name" ]; then
                        echo "复制依赖库: $dep -> $APP_NAME/Contents/Frameworks/$dep_name"
                        cp "$dep" "$APP_NAME/Contents/Frameworks/"
                        process_library_dependencies "$dep"
                    fi
                    
                    echo "修改依赖引用: $original_dep -> @executable_path/../Frameworks/$dep_name"
                    if [ ! -w "$APP_NAME/Contents/Frameworks/$lib_name" ]; then
                        chmod u+w "$APP_NAME/Contents/Frameworks/$lib_name"
                    fi
                    install_name_tool -change "$original_dep" "@executable_path/../Frameworks/$dep_name" "$APP_NAME/Contents/Frameworks/$lib_name"
                fi
            fi
        done
    }
    
    # 处理主程序的直接依赖
    echo "处理 OpenCV 依赖..."
    for lib in $(otool -L "$APP_NAME/Contents/MacOS/ImageProcessor" | grep opencv | awk '{print $1}'); do
        process_library_dependencies "$lib"
        # 修改主程序中的库路径
        install_name_tool -change "$lib" "@executable_path/../Frameworks/$(basename $lib)" "$APP_NAME/Contents/MacOS/ImageProcessor"
    done

    # 处理 Frameworks 目录下的其他库
    echo "处理 Frameworks 目录下的其他库..."
    for lib in "$APP_NAME/Contents/Frameworks/"*.dylib; do
        lib_name=$(basename "$lib")
        # 跳过已处理的 OpenCV 库
        if [[ "$lib_name" == *"opencv"* ]]; then
            continue
        fi
        echo "处理其他库: $lib_name"
        process_library_dependencies "$lib"
    done

    # 添加代码签名步骤
    echo "正在进行代码签名..."
    # 先签名所有框架
    find "$APP_NAME/Contents/Frameworks" -type f -name "*.dylib" -o -name "*.framework" | while read -r framework; do
        codesign --force --sign - "$framework"
    done
    
    # 签名主程序
    codesign --force --deep --sign - "$APP_NAME"
    
    # 创建 dmg
    if [ -f "ImageProcessor.dmg" ]; then
        rm ImageProcessor.dmg
    fi
    hdiutil create -volname "ImageProcessor" -srcfolder "$APP_NAME" -ov -format UDZO ImageProcessor.dmg
    
    echo "打包完成：ImageProcessor.dmg"
fi