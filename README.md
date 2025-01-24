
## 介绍
- 该代码主要靠 AI 生成，用到 QT + OpenCv
- 目标是不断实现自己常用的工具，打造最好用、最简单的工具

## 说明
- 中文：[中文说明](https://github.com/Flyingon/ImageProcessor/wiki/%E8%AF%B4%E6%98%8E%E4%B9%A6%E2%80%90%E4%B8%AD%E6%96%87)
- English：[English Introduction](https://github.com/Flyingon/ImageProcessor/wiki/Introducation%E2%80%90english)

## 下载
- https://github.com/Flyingon/ImageProcessor/releases/tag/v1.0.0 当前只支持 Mac OS
- 没有签名，直接下载编译好的需要允许打开所有来源：[参考](https://www.insta360.com/cn/support/supportcourse?post_id=16882)
- 或者自己编译
<img width="994" alt="image" src="https://github.com/user-attachments/assets/7bb3df02-bc3c-4137-9a11-4917bac253cb" />


## 编译
### 编译
cmake 你懂的～
```
mkdir build
cd build
cmake ..
make
```

### 打包
当前只支持 Mac Os
会把 QT + OpenCv 所有依赖都打包进去，所以结果包比较大
```
./deploy
```

### 检查打包结果(一般不用检查)
```
./check_app.sh build/ImageProcessor.app
```
