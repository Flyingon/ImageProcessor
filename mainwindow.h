#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include "splitmerge/splitmergewidget.h"
#include "cocosatlas/cocosatlaswidget.h"
#include "styleconvert/styleconvertwidget.h"
#include "avatarhat/avatarhatwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTabWidget *tabWidget;
};
#endif // MAINWINDOW_H