#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 创建各个功能页面
    tabWidget = new QTabWidget(this);
    
    // 设置 tab 样式
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #dee2e6; }"
        "QTabBar::tab { min-width: 120px; min-height: 40px; padding: 8px 16px; "
        "               font-size: 14px; margin-right: 4px; }"
        "QTabBar::tab:selected { background: #2196F3; color: white; }"
        "QTabBar::tab:!selected { background: #f8f9fa; color: #495057; }"
        "QTabBar::tab:hover:!selected { background: #e9ecef; }"
    );
    
    tabWidget->addTab(new SplitMergeWidget(this), tr("图片分割合并"));
    tabWidget->addTab(new CocosAtlasWidget(this), tr("生成Cocos图集"));
    tabWidget->addTab(new StyleConvertWidget(this), tr("风格转换"));
    tabWidget->addTab(new AvatarHatWidget(this), tr("头像戴帽子"));
    
    // 设置为中心部件
    setCentralWidget(tabWidget);

    // 设置应用图标
    setWindowIcon(QIcon(":/icons/app.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}