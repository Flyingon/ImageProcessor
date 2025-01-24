
## 介绍
- 该代码主要靠 AI 生成，用到 QT + OpenCv
- 主要不断实现自己常用的工具，方便生活

## 说明
- 中文：https://github.com/Flyingon/ImageProcessor/wiki/%E8%AF%B4%E6%98%8E%E4%B9%A6%E2%80%90%E4%B8%AD%E6%96%87
- 英文：https://github.com/Flyingon/ImageProcessor/wiki/Introducation%E2%80%90english
<img width="998" alt="image" src="https://github.com/user-attachments/assets/de9130b5-daa2-434a-a81e-36400fa78887" />


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
会把 QT + OpenCv 所有依赖都打包进去，所以显得比较大
```
./deploy
```

### 检查打包结果
```
./check_app.sh build/ImageProcessor.app
```
