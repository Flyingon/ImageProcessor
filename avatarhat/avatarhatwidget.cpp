#include "avatarhatwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>  // 添加菜单头文件

AvatarHatWidget::AvatarHatWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 左侧帽子列表
    QVBoxLayout *leftLayout = new QVBoxLayout();
    btnLoadHat = new QPushButton(tr("添加帽子"), this);
    hatList = new QListWidget(this);
    hatList->setIconSize(QSize(32, 32));  // 缩小图标尺寸
    hatList->setMinimumWidth(120);        // 减小最小宽度
    hatList->setMaximumWidth(150);        // 添加最大宽度限制
    hatList->setViewMode(QListWidget::ListMode);
    hatList->setSpacing(2);               // 设置项目间距
    
    // 设置列表样式
    hatList->setStyleSheet(
        "QListWidget { background-color: #ffffff; border: 1px solid #dee2e6; border-radius: 4px; }"
        "QListWidget::item { padding: 4px; }"
        "QListWidget::item:selected { background-color: #e3f2fd; color: #1976D2; }"
        "QListWidget::item:hover { background-color: #f5f5f5; }"
    );
    hatList->setContextMenuPolicy(Qt::CustomContextMenu);  // 启用自定义右键菜单
    
    hatList->setIconSize(QSize(60, 60));
    hatList->setMinimumWidth(200);
    hatList->setViewMode(QListWidget::ListMode);
    
    leftLayout->addWidget(btnLoadHat);
    leftLayout->addWidget(hatList);
    
    // 右侧布局
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnLoadAvatar = new QPushButton(tr("选择头像"), this);
    btnSave = new QPushButton(tr("保存结果"), this);
    
    buttonLayout->addWidget(btnLoadAvatar);
    buttonLayout->addWidget(btnSave);
    
    // 创建图片显示区域
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 400);
    imageLabel->setText(tr("请选择头像..."));
    
    rightLayout->addLayout(buttonLayout);
    rightLayout->addWidget(imageLabel);
    
    // 设置样式
    imageLabel->setStyleSheet("QLabel { background-color: #ffffff; border: 1px dashed #dee2e6; border-radius: 4px; padding: 10px; }");
    QString buttonStyle = "QPushButton { min-width: 80px; min-height: 30px; padding: 5px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px; }"
                         "QPushButton:hover { background-color: #1976D2; }"
                         "QPushButton:pressed { background-color: #0D47A1; }";
    btnLoadAvatar->setStyleSheet(buttonStyle);
    btnLoadHat->setStyleSheet(buttonStyle);
    btnSave->setStyleSheet(buttonStyle);
    
    // 设置主布局 - 删除重复的布局添加
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    
    // 连接信号
    connect(btnLoadAvatar, &QPushButton::clicked, this, &AvatarHatWidget::onLoadAvatarClicked);
    connect(btnLoadHat, &QPushButton::clicked, this, &AvatarHatWidget::onLoadHatClicked);
    connect(btnSave, &QPushButton::clicked, this, &AvatarHatWidget::onSaveClicked);
    connect(hatList, &QListWidget::currentRowChanged, this, &AvatarHatWidget::updatePreview);
    connect(hatList, &QListWidget::customContextMenuRequested, this, &AvatarHatWidget::onHatListContextMenu);  // 添加右键菜单信号
    
    // 初始状态
    btnSave->setEnabled(false);
}

void AvatarHatWidget::onLoadHatClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
        tr("选择帽子"), "", tr("PNG图片 (*.png)"));
    
    if (files.isEmpty()) return;
    
    for (const QString &file : files) {
        cv::Mat hat = cv::imread(file.toStdString(), cv::IMREAD_UNCHANGED);
        if (!hat.empty() && hat.channels() == 4) {
            hats.push_back(hat);
            QFileInfo fileInfo(file);
            
            // 创建缩略图
            cv::Mat thumbnail;
            cv::resize(hat, thumbnail, cv::Size(60, 60));
            QImage qimg = cvMatToQImage(thumbnail);
            
            // 添加到列表
            QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName());
            item->setIcon(QIcon(QPixmap::fromImage(qimg)));
            hatList->addItem(item);
        }
    }
    
    updatePreview();
}

// 删除 onGenerateClicked 函数

void AvatarHatWidget::updatePreview()
{
    if (!currentAvatar.empty() && !hats.empty() && hatList->currentRow() >= 0) {
        cv::Mat result;
        addHatToFaces(currentAvatar, hats[hatList->currentRow()], result);
        
        if (!result.empty()) {
            previewImage = result;
            QImage img = cvMatToQImage(result);
            imageLabel->setPixmap(QPixmap::fromImage(img.scaled(
                imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            btnSave->setEnabled(true);
        }
    }
}

void AvatarHatWidget::onSaveClicked()
{
    if (previewImage.empty()) return;
    
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("保存图片"), "", tr("图片文件 (*.png *.jpg)"));
    
    if (fileName.isEmpty()) return;
    
    if (cv::imwrite(fileName.toStdString(), previewImage)) {
        QMessageBox::information(this, tr("成功"), tr("图片已保存"));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("保存失败"));
    }
}

void AvatarHatWidget::onLoadAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择头像"), "", tr("图片文件 (*.png *.jpg *.jpeg)"));
    
    if (fileName.isEmpty()) return;
    
    currentAvatar = cv::imread(fileName.toStdString());
    if (currentAvatar.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载图片"));
        return;
    }
    
    QImage img = cvMatToQImage(currentAvatar);
    imageLabel->setPixmap(QPixmap::fromImage(img.scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    
    updatePreview();
}

void AvatarHatWidget::addHatToFaces(const cv::Mat &avatar, const cv::Mat &hat, cv::Mat &result)
{
    // 加载人脸检测器
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml")) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载人脸检测器"));
        return;
    }
    
    // 检测人脸
    cv::Mat gray;
    cv::cvtColor(avatar, gray, cv::COLOR_BGR2GRAY);
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces);
    
    // 复制原图
    avatar.copyTo(result);
    
    // 为每个检测到的人脸添加帽子
    for (const auto &face : faces) {
        // 调整帽子大小
        cv::Mat resized_hat;
        double scale = face.width / static_cast<double>(hat.cols) * 1.5;
        cv::resize(hat, resized_hat, cv::Size(), scale, scale);
        
        // 计算帽子位置
        int x = face.x + face.width/2 - resized_hat.cols/2;
        int y = face.y - resized_hat.rows/2;
        
        // 确保坐标在图像范围内
        x = std::max(0, std::min(x, result.cols - resized_hat.cols));
        y = std::max(0, std::min(y, result.rows - resized_hat.rows));
        
        // 创建ROI
        cv::Mat roi = result(cv::Rect(x, y, 
            std::min(resized_hat.cols, result.cols - x),
            std::min(resized_hat.rows, result.rows - y)));
        
        // 合并帽子和原图
        for (int i = 0; i < roi.rows; ++i) {
            for (int j = 0; j < roi.cols; ++j) {
                if (i < resized_hat.rows && j < resized_hat.cols) {
                    cv::Vec4b hat_pixel = resized_hat.at<cv::Vec4b>(i, j);
                    if (hat_pixel[3] > 0) {  // 如果不完全透明
                        float alpha = hat_pixel[3] / 255.0f;
                        for (int c = 0; c < 3; ++c) {
                            roi.at<cv::Vec3b>(i, j)[c] = 
                                (1 - alpha) * roi.at<cv::Vec3b>(i, j)[c] + 
                                alpha * hat_pixel[c];
                        }
                    }
                }
            }
        }
    }
}

QImage AvatarHatWidget::cvMatToQImage(const cv::Mat &mat)
{
    if (mat.empty()) return QImage();
    
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, 
            rgb.step, QImage::Format_RGB888).copy();
    }
    
    return QImage();
}

// 添加右键菜单处理函数
void AvatarHatWidget::onHatListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = hatList->itemAt(pos);
    if (!item) return;
    
    QMenu menu(this);
    QAction *removeAction = menu.addAction(tr("删除"));
    
    if (menu.exec(hatList->viewport()->mapToGlobal(pos)) == removeAction) {
        int row = hatList->row(item);
        delete hatList->takeItem(row);
        if (row < hats.size()) {
            hats.erase(hats.begin() + row);
        }
        updatePreview();
    }
}