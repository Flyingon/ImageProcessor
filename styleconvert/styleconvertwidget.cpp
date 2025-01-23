#include "styleconvertwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

StyleConvertWidget::StyleConvertWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 左侧布局
    QVBoxLayout *leftLayout = new QVBoxLayout();
    btnLoadImage = new QPushButton(tr("选择图片"), this);
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 400);
    imageLabel->setText(tr("请选择图片..."));
    
    leftLayout->addWidget(btnLoadImage);
    leftLayout->addWidget(imageLabel);
    
    // 右侧布局
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    // 创建证件照按钮组的水平布局
    QHBoxLayout *photoLayout = new QHBoxLayout();
    btnChangeColor = new QPushButton(tr("证件照换背景色(只支持蓝底)"), this);
    photoLayout->addWidget(btnChangeColor);
    
    // 创建动漫风格按钮组的水平布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnCartoon = new QPushButton(tr("动漫1"), this);
    btnCartoonV2 = new QPushButton(tr("动漫2"), this);
    btnPixelate = new QPushButton(tr("像素1"), this);
    buttonLayout->addWidget(btnCartoon);
    buttonLayout->addWidget(btnCartoonV2);
    buttonLayout->addWidget(btnPixelate);
    
    btnSave = new QPushButton(tr("保存结果"), this);
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(400, 400);
    previewLabel->setText(tr("预览区域"));
    
    // 添加到右侧布局
    rightLayout->addLayout(photoLayout);
    rightLayout->addLayout(buttonLayout);
    rightLayout->addWidget(previewLabel);
    rightLayout->addWidget(btnSave);
    
    // 设置样式
    QString labelStyle = "QLabel { background-color: #ffffff; border: 1px dashed #dee2e6; border-radius: 4px; padding: 10px; }";
    imageLabel->setStyleSheet(labelStyle);
    previewLabel->setStyleSheet(labelStyle);
    
    QString buttonStyle = "QPushButton { min-width: 120px; min-height: 30px; padding: 5px 10px; "
                         "background-color: #2196F3; color: white; border: none; border-radius: 4px; }"
                         "QPushButton:hover { background-color: #1976D2; }"
                         "QPushButton:pressed { background-color: #0D47A1; }";
    btnLoadImage->setStyleSheet(buttonStyle);
    btnCartoon->setStyleSheet(buttonStyle);
    btnCartoonV2->setStyleSheet(buttonStyle);
    btnPixelate->setStyleSheet(buttonStyle);
    btnChangeColor->setStyleSheet(buttonStyle);
    btnSave->setStyleSheet(buttonStyle);
    
    // 设置主布局
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    
    // 连接信号
    connect(btnLoadImage, &QPushButton::clicked, this, &StyleConvertWidget::onLoadImageClicked);
    connect(btnCartoon, &QPushButton::clicked, this, &StyleConvertWidget::onCartoonClicked);
    connect(btnCartoonV2, &QPushButton::clicked, this, [this]() {
        if (currentImage.empty()) return;
        cv::Mat result;
        cartoonV3(currentImage, result);
        updatePreview(result);
        previewImage = result;
        btnSave->setEnabled(true);
    });
    connect(btnPixelate, &QPushButton::clicked, this, &StyleConvertWidget::onPixelateClicked);
    connect(btnSave, &QPushButton::clicked, this, &StyleConvertWidget::onSaveClicked);
    connect(btnChangeColor, &QPushButton::clicked, this, &StyleConvertWidget::onChangeColorClicked);
    
    // 初始状态
    btnCartoon->setEnabled(false);
    btnCartoonV2->setEnabled(false);
    btnPixelate->setEnabled(false);
    btnChangeColor->setEnabled(false);
    btnSave->setEnabled(false);
}

void StyleConvertWidget::onLoadImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择图片"), "", tr("图片文件 (*.png *.jpg *.jpeg)"));
    
    if (fileName.isEmpty()) return;
    
    currentImage = cv::imread(fileName.toStdString());
    if (currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载图片"));
        return;
    }
    
    // 显示原图
    QImage qimg = cvMatToQImage(currentImage);
    if (!qimg.isNull()) {
        imageLabel->setPixmap(QPixmap::fromImage(qimg.scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        previewLabel->setPixmap(QPixmap::fromImage(qimg.scaled(
            previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
    
    btnCartoon->setEnabled(true);
    btnCartoonV2->setEnabled(true);
    btnPixelate->setEnabled(true);
    btnChangeColor->setEnabled(true);  // 启用证件照换背景色按钮
    
    btnSave->setEnabled(true);
}

void StyleConvertWidget::onCartoonClicked()
{
    if (currentImage.empty()) return;
    
    cv::Mat result;
    cartoonV2(currentImage, result);
    updatePreview(result);
    previewImage = result;
    btnSave->setEnabled(true);
}

void StyleConvertWidget::onPixelateClicked()
{
    if (currentImage.empty()) return;
    
    cv::Mat result;
    pixelateImage(currentImage, result);
    updatePreview(result);
    previewImage = result;
    btnSave->setEnabled(true);
}

void StyleConvertWidget::onSaveClicked()
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

void StyleConvertWidget::updatePreview(const cv::Mat &img)
{
    QImage qimg = cvMatToQImage(img);
    if (!qimg.isNull()) {
        previewLabel->setPixmap(QPixmap::fromImage(qimg.scaled(
            previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

QImage StyleConvertWidget::cvMatToQImage(const cv::Mat &mat)
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

void StyleConvertWidget::cartoonV2(const cv::Mat &img, cv::Mat &result)
{
    // Step 1: Edge detection
    cv::Mat grayImage, edges;
    cv::cvtColor(img, grayImage, cv::COLOR_BGR2GRAY);
    cv::medianBlur(grayImage, grayImage, 7);
    cv::Canny(grayImage, edges, 50, 150);
    cv::threshold(edges, edges, 100, 255, cv::THRESH_BINARY_INV);

    // Step 2: Image smoothing with bilateral filter
    cv::Mat smoothedImage;
    cv::bilateralFilter(img, smoothedImage, 9, 75, 75);

    // Step 3: Combine edges with smoothed image
    cv::Mat edgesColor;
    cv::cvtColor(edges, edgesColor, cv::COLOR_GRAY2BGR);
    result = smoothedImage & edgesColor;
}

void StyleConvertWidget::pixelateImage(const cv::Mat &src, cv::Mat &result, int pixelSize)
{
    cv::Mat smallImage;
    cv::resize(src, smallImage, cv::Size(src.cols / pixelSize, src.rows / pixelSize), 0, 0, cv::INTER_LINEAR);
    cv::resize(smallImage, result, cv::Size(src.cols, src.rows), 0, 0, cv::INTER_NEAREST);
}

// 添加新的动漫风格处理函数
void StyleConvertWidget::cartoonV3(const cv::Mat &img, cv::Mat &result)
{
    // 将输入图像转换为灰度图
    cv::Mat grayImage;
    cv::cvtColor(img, grayImage, cv::COLOR_BGR2GRAY);

    // 使用 Canny 算子进行边缘检测
    cv::Mat edges;
    double threshold1 = 100; // 您可以根据需要调整阈值
    double threshold2 = 150; // 您可以根据需要调整阈值
    cv::Canny(grayImage, edges, threshold1, threshold2);

    // 反转边缘图像的颜色
    cv::Mat edgesInverted;
    cv::bitwise_not(edges, edgesInverted);

    // 将单通道的边缘图像转换为三通道
    cv::Mat edgesInvertedColor;
    cv::cvtColor(edgesInverted, edgesInvertedColor, cv::COLOR_GRAY2BGR);

    // 将边缘叠加到原始图像上
    cv::Mat imgWithEdges;
    cv::bitwise_and(img, edgesInvertedColor, imgWithEdges);

    // 对图像进行双边滤波以保留边缘并平滑区域
    cv::Mat bilateralFiltered;
    int d = 9; // 邻域直径，您可以根据需要调整
    double sigmaColor = 75; // 色彩空间的标准差，您可以根据需要调整
    double sigmaSpace = 75; // 坐标空间的标准差，您可以根据需要调整
    cv::bilateralFilter(imgWithEdges, bilateralFiltered, d, sigmaColor, sigmaSpace);

    // 将图像从 BGR 转换为 HSV
    cv::Mat hsvImage;
    cv::cvtColor(bilateralFiltered, hsvImage, cv::COLOR_BGR2HSV);

    // 增强饱和度
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvImage, hsvChannels);
    double saturationScale = 1.5; // 饱和度增强系数，您可以根据需要调整
    hsvChannels[1] *= saturationScale;
    cv::merge(hsvChannels, hsvImage);

    // 将图像从 HSV 转换回 BGR
    cv::cvtColor(hsvImage, result, cv::COLOR_HSV2BGR);
}

void StyleConvertWidget::onChangeColorClicked()
{
    if (currentImage.empty()) return;
    
    QColor color = QColorDialog::getColor(Qt::white, this, tr("选择背景色"));
    if (!color.isValid()) return;
    
    cv::Mat result;
    changePhotoBackground(currentImage, result, cv::Scalar(color.blue(), color.green(), color.red()));
    updatePreview(result);
    previewImage = result;
    btnSave->setEnabled(true);
}

void StyleConvertWidget::changePhotoBackground(const cv::Mat &src, cv::Mat &dst, const cv::Scalar &bgColor)
{
   // 检查输入图像是否为空
    if (src.empty())
    {
        throw std::invalid_argument("Input image is empty.");
    }

    // 转换为 HSV 色彩空间
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

    // 定义蓝色的范围
    cv::Scalar lowerBlue(78, 43, 46);
    cv::Scalar upperBlue(110, 255, 255);

    // 创建掩膜
    cv::Mat mask;
    cv::inRange(hsv, lowerBlue, upperBlue, mask);

    // 腐蚀和膨胀操作
    cv::Mat eroded, dilated;
    cv::erode(mask, eroded, cv::Mat(), cv::Point(-1, -1), 1);
    cv::dilate(eroded, dilated, cv::Mat(), cv::Point(-1, -1), 1);

    // 创建输出图像副本
    dst = src.clone();

    // 遍历每个像素，替换背景颜色
    for (int i = 0; i < dst.rows; ++i)
    {
        for (int j = 0; j < dst.cols; ++j)
        {
            if (dilated.at<uchar>(i, j) == 255)
            {
                // 替换背景颜色
                dst.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)bgColor[0], (uchar)bgColor[1], (uchar)bgColor[2]);
            }
        }
    }
}