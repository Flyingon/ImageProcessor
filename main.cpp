#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用图标
    a.setWindowIcon(QIcon(":/icons/app.png"));

    MainWindow w;
    w.show();
    return a.exec();
}