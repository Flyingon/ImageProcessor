#include "splitmergewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

SplitMergeWidget::SplitMergeWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnLoad = new QPushButton(tr("加载图片"), this);
    btnSplit = new QPushButton(tr("分割图片"), this);
    btnMerge = new QPushButton(tr("合并图片"), this);
    btnRemoveBackground = new QPushButton(tr("去除背景"), this);
    
    buttonLayout->addWidget(btnLoad);
    buttonLayout->addWidget(btnSplit);
    buttonLayout->addWidget(btnMerge);
    buttonLayout->addWidget(btnRemoveBackground);
    
    // 创建图片显示区域
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 300);
    imageLabel->setText("请加载图片...");
    
    // 设置样式
    imageLabel->setStyleSheet("QLabel { background-color: #ffffff; border: 1px dashed #dee2e6; border-radius: 4px; padding: 10px; }");
    QString buttonStyle = "QPushButton { min-width: 80px; min-height: 30px; padding: 5px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px; }"
                         "QPushButton:hover { background-color: #1976D2; }"
                         "QPushButton:pressed { background-color: #0D47A1; }";
    btnLoad->setStyleSheet(buttonStyle);
    btnSplit->setStyleSheet(buttonStyle);
    btnMerge->setStyleSheet(buttonStyle);
    btnRemoveBackground->setStyleSheet(buttonStyle);
    
    // 添加到主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(imageLabel);
    
    // 连接信号
    connect(btnLoad, &QPushButton::clicked, this, &SplitMergeWidget::onLoadClicked);
    connect(btnSplit, &QPushButton::clicked, this, &SplitMergeWidget::onSplitClicked);
    connect(btnMerge, &QPushButton::clicked, this, &SplitMergeWidget::onMergeClicked);
    connect(btnRemoveBackground, &QPushButton::clicked, this, &SplitMergeWidget::onRemoveBackgroundClicked);

    // 初始只禁用分割按钮和去除背景按钮，合并按钮可用
    btnSplit->setEnabled(false);
    btnRemoveBackground->setEnabled(false);
    btnMerge->setEnabled(true);
}

void SplitMergeWidget::onLoadClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("打开图片"), "", tr("图片文件 (*.png *.jpg *.bmp)"));
    
    if (fileName.isEmpty())
        return;

    currentImage = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);
    if (currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载图片"));
        return;
    }

    if (currentImage.channels() == 3) {
        cv::Mat alpha;
        cv::cvtColor(currentImage, alpha, cv::COLOR_BGR2GRAY);
        cv::threshold(alpha, alpha, 0, 255, cv::THRESH_BINARY);
        std::vector<cv::Mat> channels;
        cv::split(currentImage, channels);
        channels.push_back(alpha);
        cv::merge(channels, currentImage);
    }

    QImage img = cvMatToQImage(currentImage);
    imageLabel->setPixmap(QPixmap::fromImage(img.scaled(imageLabel->size(), 
        Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    
    if (!currentImage.empty()) {
        btnSplit->setEnabled(true);
        btnMerge->setEnabled(true);
        btnRemoveBackground->setEnabled(true);  // 新增
    }
}

// 实现去除背景功能
void SplitMergeWidget::onRemoveBackgroundClicked()
{
    if (currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图片"));
        return;
    }

    // 转换为 BGR 格式
    cv::Mat bgr;
    if (currentImage.channels() == 4) {
        cv::cvtColor(currentImage, bgr, cv::COLOR_BGRA2BGR);
    } else {
        bgr = currentImage.clone();
    }

    // 转换到 HSV 颜色空间
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // 计算背景色（假设四个角落的颜色为背景色）
    cv::Vec3b topLeft = hsv.at<cv::Vec3b>(0, 0);
    cv::Vec3b topRight = hsv.at<cv::Vec3b>(0, hsv.cols-1);
    cv::Vec3b bottomLeft = hsv.at<cv::Vec3b>(hsv.rows-1, 0);
    cv::Vec3b bottomRight = hsv.at<cv::Vec3b>(hsv.rows-1, hsv.cols-1);

    // 计算背景色的平均值和容差范围
    int hTolerance = 10;
    int sTolerance = 50;
    int vTolerance = 50;

    // 创建掩码
    cv::Mat mask = cv::Mat::zeros(hsv.size(), CV_8UC1);
    
    // 遍历图像，标记非背景区域
    for(int i = 0; i < hsv.rows; i++) {
        for(int j = 0; j < hsv.cols; j++) {
            cv::Vec3b pixel = hsv.at<cv::Vec3b>(i, j);
            bool isBackground = false;
            
            // 检查像素是否接近任一背景色
            for(const cv::Vec3b& bgColor : {topLeft, topRight, bottomLeft, bottomRight}) {
                if(std::abs(pixel[0] - bgColor[0]) <= hTolerance &&
                   std::abs(pixel[1] - bgColor[1]) <= sTolerance &&
                   std::abs(pixel[2] - bgColor[2]) <= vTolerance) {
                    isBackground = true;
                    break;
                }
            }
            
            if(!isBackground) {
                mask.at<uchar>(i, j) = 255;
            }
        }
    }

    // 应用形态学操作来改善掩码
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    // 创建 4 通道结果图像
    cv::Mat result(bgr.size(), CV_8UC4);
    
    // 合并通道
    std::vector<cv::Mat> channels;
    cv::split(bgr, channels);
    channels.push_back(mask);
    cv::merge(channels, result);

    // 更新当前图像
    currentImage = result;

    // 显示结果
    QImage img = cvMatToQImage(currentImage);
    imageLabel->setPixmap(QPixmap::fromImage(img.scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    // 提示用户保存结果
    QString savePath = QFileDialog::getSaveFileName(this,
        tr("保存结果"), "", tr("PNG图片 (*.png)"));
    
    if (!savePath.isEmpty()) {
        cv::imwrite(savePath.toStdString(), result);
        QMessageBox::information(this, tr("成功"), tr("图片已保存"));
    }
}

void SplitMergeWidget::onSplitClicked()
{
    if (currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图片"));
        return;
    }

    QString saveDir = QFileDialog::getExistingDirectory(this, tr("选择保存目录"));
    if (saveDir.isEmpty())
        return;

    std::vector<cv::Mat> splitResult = splitImages(currentImage);
    
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    for (size_t i = 0; i < splitResult.size(); ++i) {
        QString fileName = QString("%1/split_%2.png").arg(saveDir).arg(i);
        cv::imwrite(fileName.toStdString(), splitResult[i], compression_params);
    }

    QMessageBox::information(this, tr("成功"), 
        tr("已保存 %1 张分割图片").arg(splitResult.size()));
}

void SplitMergeWidget::onMergeClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("选择图片"), "", tr("图片文件 (*.png)"));
    
    if (fileNames.isEmpty())
        return;

    std::vector<cv::Mat> images;
    for (const QString &fileName : fileNames) {
        cv::Mat img = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);
        if (!img.empty()) {
            images.push_back(img);
        }
    }

    if (images.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("没有有效的图片"));
        return;
    }

    int cols = 3;
    cv::Mat merged = mergeImages(images, cols);

    QString saveName = QFileDialog::getSaveFileName(this,
        tr("保存合并图片"), "", tr("PNG图片 (*.png)"));
    
    if (!saveName.isEmpty()) {
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
        cv::imwrite(saveName.toStdString(), merged, compression_params);
        QMessageBox::information(this, tr("成功"), tr("图片已合并并保存"));
    }
}

QImage SplitMergeWidget::cvMatToQImage(const cv::Mat &mat)
{
    if (mat.empty())
        return QImage();

    if (mat.type() == CV_8UC4) {
        cv::Mat converted;
        cv::cvtColor(mat, converted, cv::COLOR_BGRA2RGBA);
        return QImage(converted.data, converted.cols, converted.rows, 
            converted.step, QImage::Format_RGBA8888).copy();
    }
    else if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, 
            rgb.step, QImage::Format_RGB888).copy();
    }
    
    return QImage();
}

cv::Mat SplitMergeWidget::QImageToCvMat(const QImage &image)
{
    if (image.isNull())
        return cv::Mat();

    switch(image.format()) {
        case QImage::Format_RGBA8888:
            return cv::Mat(image.height(), image.width(), CV_8UC4, 
                const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
        case QImage::Format_RGB888:
        {
            cv::Mat mat(image.height(), image.width(), CV_8UC3, 
                const_cast<uchar*>(image.bits()), image.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
            return result;
        }
        default:
            return cv::Mat();
    }
}

std::vector<cv::Mat> SplitMergeWidget::splitImages(const cv::Mat &image)
{
    std::vector<cv::Mat> result;
    
    cv::Mat alpha;
    if (image.channels() == 4) {
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        alpha = channels[3];
    } else {
        cv::cvtColor(image, alpha, cv::COLOR_BGR2GRAY);
    }
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(alpha, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto &contour : contours) {
        cv::Rect boundRect = cv::boundingRect(contour);
        if (boundRect.width < 5 || boundRect.height < 5) continue;
        
        int padding = 2;
        boundRect.x = std::max(0, boundRect.x - padding);
        boundRect.y = std::max(0, boundRect.y - padding);
        boundRect.width = std::min(image.cols - boundRect.x, boundRect.width + 2 * padding);
        boundRect.height = std::min(image.rows - boundRect.y, boundRect.height + 2 * padding);
        
        cv::Mat subImage = image(boundRect).clone();
        result.push_back(subImage);
    }
    
    return result;
}

cv::Mat SplitMergeWidget::mergeImages(const std::vector<cv::Mat> &images, int cols)
{
    if (images.empty())
        return cv::Mat();

    int rows = (images.size() + cols - 1) / cols;
    
    int maxWidth = 0, maxHeight = 0;
    for (const auto &img : images) {
        maxWidth = std::max(maxWidth, img.cols);
        maxHeight = std::max(maxHeight, img.rows);
    }
    
    cv::Mat result(maxHeight * rows, maxWidth * cols, CV_8UC4, cv::Scalar(0,0,0,0));
    
    for (size_t i = 0; i < images.size(); ++i) {
        int row = i / cols;
        int col = i % cols;
        
        cv::Mat roi = result(cv::Rect(col * maxWidth, row * maxHeight, 
            images[i].cols, images[i].rows));
            
        if (images[i].channels() == 4) {
            images[i].copyTo(roi);
        } else {
            cv::Mat temp;
            cv::cvtColor(images[i], temp, cv::COLOR_BGR2BGRA);
            temp.copyTo(roi);
        }
    }
    
    return result;
}