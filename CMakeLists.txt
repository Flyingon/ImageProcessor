cmake_minimum_required(VERSION 3.5)

project(ImageProcessor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        splitmerge/splitmergewidget.cpp
        splitmerge/splitmergewidget.h
        cocosatlas/cocosatlaswidget.cpp
        cocosatlas/cocosatlaswidget.h
        styleconvert/styleconvertwidget.cpp
        styleconvert/styleconvertwidget.h
        avatarhat/avatarhatwidget.cpp
        avatarhat/avatarhatwidget.h
)

# 添加资源文件
set(RESOURCE_FILES
    resources/resources.qrc
)

# 设置可执行文件名称为 ImageProcessor
add_executable(ImageProcessor
    ${PROJECT_SOURCES}
    ${RESOURCE_FILES}
)

# 链接所需的库
target_link_libraries(ImageProcessor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)