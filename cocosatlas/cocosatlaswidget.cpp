#include "cocosatlaswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QXmlStreamWriter>

CocosAtlasWidget::CocosAtlasWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 左侧列表和按钮
    QVBoxLayout *leftLayout = new QVBoxLayout();
    imageList = new QListWidget(this);
    btnLoad = new QPushButton(tr("添加图片"), this);
    btnMoveUp = new QPushButton(tr("上移"), this);
    btnMoveDown = new QPushButton(tr("下移"), this);
    btnRemove = new QPushButton(tr("删除"), this);
    btnGenerate = new QPushButton(tr("生成图集"), this);
    
    btnClear = new QPushButton(tr("清空列表"), this);
    
    leftLayout->addWidget(imageList);
    leftLayout->addWidget(btnLoad);
    leftLayout->addWidget(btnClear);  // 新增
    leftLayout->addWidget(btnMoveUp);
    leftLayout->addWidget(btnMoveDown);
    leftLayout->addWidget(btnRemove);
    leftLayout->addWidget(btnGenerate);
    
    // 右侧预览
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(400, 400);
    previewLabel->setText(tr("预览区域"));
    previewLabel->setStyleSheet("QLabel { background-color: #ffffff; border: 1px solid #cccccc; }");
    
    // 设置按钮样式
    QString buttonStyle = "QPushButton { min-width: 80px; min-height: 30px; padding: 5px 15px; "
                         "background-color: #2196F3; color: white; border: none; border-radius: 4px; }"
                         "QPushButton:hover { background-color: #1976D2; }"
                         "QPushButton:pressed { background-color: #0D47A1; }";
    btnLoad->setStyleSheet(buttonStyle);
    btnMoveUp->setStyleSheet(buttonStyle);
    btnMoveDown->setStyleSheet(buttonStyle);
    btnRemove->setStyleSheet(buttonStyle);
    btnGenerate->setStyleSheet(buttonStyle);
    
    // 布局
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addWidget(previewLabel, 2);
    
    // 连接信号
    connect(btnLoad, &QPushButton::clicked, this, &CocosAtlasWidget::onLoadClicked);
    connect(btnClear, &QPushButton::clicked, this, &CocosAtlasWidget::onClearClicked);  // 新增
    connect(btnMoveUp, &QPushButton::clicked, this, &CocosAtlasWidget::onMoveUpClicked);
    connect(btnMoveDown, &QPushButton::clicked, this, &CocosAtlasWidget::onMoveDownClicked);
    connect(btnRemove, &QPushButton::clicked, this, &CocosAtlasWidget::onRemoveClicked);
    connect(btnGenerate, &QPushButton::clicked, this, &CocosAtlasWidget::onGenerateClicked);
    
    // 初始状态
    btnMoveUp->setEnabled(false);
    btnMoveDown->setEnabled(false);
    btnRemove->setEnabled(false);
    btnClear->setEnabled(false);  // 新增
    btnGenerate->setEnabled(false);
}

void CocosAtlasWidget::onLoadClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
        tr("选择图片"), "", tr("图片文件 (*.png *.jpg)"));
        
    if (files.isEmpty()) return;
    
    for (const QString &file : files) {
        cv::Mat img = cv::imread(file.toStdString(), cv::IMREAD_UNCHANGED);
        if (!img.empty()) {
            QFileInfo fileInfo(file);
            imageList->addItem(fileInfo.fileName());
            images.push_back(img);
            imageNames.push_back(fileInfo.fileName());
        }
    }
    
    btnMoveUp->setEnabled(imageList->count() > 1);
    btnMoveDown->setEnabled(imageList->count() > 1);
    btnRemove->setEnabled(true);
    btnClear->setEnabled(true);  // 新增
    btnGenerate->setEnabled(!images.empty());
    
    updatePreview();
}

// 新增清空功能
void CocosAtlasWidget::onClearClicked()
{
    if (QMessageBox::question(this, tr("确认"), tr("确定要清空所有图片吗？"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        imageList->clear();
        images.clear();
        imageNames.clear();
        
        btnMoveUp->setEnabled(false);
        btnMoveDown->setEnabled(false);
        btnRemove->setEnabled(false);
        btnClear->setEnabled(false);
        btnGenerate->setEnabled(false);
        
        updatePreview();
    }
}

void CocosAtlasWidget::onGenerateClicked()
{
    if (images.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先添加图片"));
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this,
        tr("保存图集"), "", tr("PNG文件 (*.png)"));
    
    if (savePath.isEmpty()) return;
    
    // 生成图集
    std::vector<SpriteFrame> frames;
    cv::Mat atlas = packImages(frames);
    
    // 检查图集是否生成成功
    if (atlas.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("生成图集失败"));
        return;
    }
    
    // 保存图集
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    bool success = cv::imwrite(savePath.toStdString(), atlas, compression_params);
    if (!success) {
        QMessageBox::warning(this, tr("错误"), tr("保存图集失败"));
        return;
    }
    
    // 生成并保存plist文件
    QString plistPath = savePath;
    plistPath.replace(".png", ".plist");
    QFile file(plistPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QFileInfo fileInfo(savePath);
        QString plistContent = generatePlist(frames, fileInfo.fileName());
        file.write(plistContent.toUtf8());
        file.close();
        
        QMessageBox::information(this, tr("成功"), 
            tr("图集已生成：\n%1\n%2").arg(savePath).arg(plistPath));
    }
}

cv::Mat CocosAtlasWidget::packImages(std::vector<SpriteFrame>& frames)
{
    // 计算总面积和最大尺寸
    int totalArea = 0;
    int maxWidth = 0, maxHeight = 0;
    for (const cv::Mat& img : images) {
        totalArea += img.cols * img.rows;
        maxWidth = std::max(maxWidth, img.cols);
        maxHeight = std::max(maxHeight, img.rows);
    }
    
    // 估算图集大小，确保足够大
    int atlasWidth = 2;
    while (atlasWidth < maxWidth * 2) atlasWidth *= 2;
    int atlasHeight = 2;
    while (atlasWidth * atlasHeight < totalArea * 2.0) atlasHeight *= 2;
    
    // 如果高度太大，增加宽度重新计算
    if (atlasHeight > 4096) {
        atlasWidth *= 2;
        atlasHeight = 2;
        while (atlasWidth * atlasHeight < totalArea * 2.0) atlasHeight *= 2;
    }
    
    // 检查最终尺寸是否合理
    if (atlasWidth > 8192 || atlasHeight > 4096) {
        QMessageBox::warning(nullptr, tr("错误"), 
            tr("图集尺寸过大（%1x%2），请减少图片数量或尺寸").arg(atlasWidth).arg(atlasHeight));
        return cv::Mat();
    }
    
    // 创建图集
    cv::Mat atlas(atlasHeight, atlasWidth, CV_8UC4, cv::Scalar(0,0,0,0));
    
    // 改进的装箱算法
    int currentX = 0;
    int currentY = 0;
    int rowHeight = 0;
    
    for (size_t i = 0; i < images.size(); ++i) {
        const cv::Mat& img = images[i];
        
        // 检查是否需要换行
        if (currentX + img.cols > atlasWidth) {
            currentX = 0;
            currentY += rowHeight;
            rowHeight = 0;
        }
        
        // 检查是否超出高度
        if (currentY + img.rows > atlasHeight) {
            QMessageBox::warning(nullptr, tr("错误"), tr("图片太多，无法放入图集"));
            return cv::Mat();
        }
        
        // 复制图片到图集
        cv::Mat roi = atlas(cv::Rect(currentX, currentY, img.cols, img.rows));
        if (img.channels() == 4) {
            img.copyTo(roi);
        } else {
            cv::Mat temp;
            cv::cvtColor(img, temp, cv::COLOR_BGR2BGRA);
            temp.copyTo(roi);
        }
        
        // 记录精灵帧信息
        SpriteFrame frame;
        frame.name = imageNames[i];
        frame.rect = cv::Rect(currentX, currentY, img.cols, img.rows);
        frame.rotated = false;
        frame.originalSize = img.size();
        frame.offset = cv::Point2f(0, 0);
        frames.push_back(frame);
        
        // 更新位置
        currentX += img.cols;
        rowHeight = std::max(rowHeight, img.rows);
    }
    
    return atlas;
}

QString CocosAtlasWidget::generatePlist(const std::vector<SpriteFrame>& frames, const QString& textureName)
{
    QString result;
    QXmlStreamWriter writer(&result);
    writer.setAutoFormatting(true);
    
    writer.writeStartDocument();
    writer.writeStartElement("plist");
    writer.writeAttribute("version", "1.0");
    
    writer.writeStartElement("dict");
    
    // frames
    writer.writeTextElement("key", "frames");
    writer.writeStartElement("dict");
    
    for (const SpriteFrame& frame : frames) {
        writer.writeTextElement("key", frame.name);
        writer.writeStartElement("dict");
        
        writer.writeTextElement("key", "frame");
        writer.writeTextElement("string", 
            QString("{{%1,%2},{%3,%4}}").arg(frame.rect.x).arg(frame.rect.y)
                                       .arg(frame.rect.width).arg(frame.rect.height));
        
        writer.writeTextElement("key", "offset");
        writer.writeTextElement("string", 
            QString("{%1,%2}").arg(frame.offset.x).arg(frame.offset.y));
        
        writer.writeTextElement("key", "rotated");
        writer.writeTextElement("false", "");
        
        writer.writeTextElement("key", "sourceColorRect");
        writer.writeTextElement("string", 
            QString("{{%1,%2},{%3,%4}}").arg(frame.rect.x).arg(frame.rect.y)
                                       .arg(frame.rect.width).arg(frame.rect.height));
        
        writer.writeTextElement("key", "sourceSize");
        writer.writeTextElement("string", 
            QString("{%1,%2}").arg(frame.originalSize.width).arg(frame.originalSize.height));
        
        writer.writeEndElement(); // dict
    }
    
    writer.writeEndElement(); // dict
    
    // metadata
    writer.writeTextElement("key", "metadata");
    writer.writeStartElement("dict");
    writer.writeTextElement("key", "format");
    writer.writeTextElement("integer", "2");
    writer.writeTextElement("key", "textureFileName");
    writer.writeTextElement("string", textureName);
    writer.writeEndElement(); // dict
    
    writer.writeEndElement(); // dict
    writer.writeEndElement(); // plist
    writer.writeEndDocument();
    
    return result;
}

void CocosAtlasWidget::onMoveUpClicked()
{
    int currentRow = imageList->currentRow();
    if (currentRow <= 0) return;
    
    // 交换列表项
    QListWidgetItem* item = imageList->takeItem(currentRow);
    imageList->insertItem(currentRow - 1, item);
    imageList->setCurrentItem(item);
    
    // 交换图片
    std::swap(images[currentRow], images[currentRow - 1]);
    std::swap(imageNames[currentRow], imageNames[currentRow - 1]);
    
    updatePreview();
}

void CocosAtlasWidget::onMoveDownClicked()
{
    int currentRow = imageList->currentRow();
    if (currentRow < 0 || currentRow >= imageList->count() - 1) return;
    
    // 交换列表项
    QListWidgetItem* item = imageList->takeItem(currentRow);
    imageList->insertItem(currentRow + 1, item);
    imageList->setCurrentItem(item);
    
    // 交换图片
    std::swap(images[currentRow], images[currentRow + 1]);
    std::swap(imageNames[currentRow], imageNames[currentRow + 1]);
    
    updatePreview();
}

void CocosAtlasWidget::onRemoveClicked()
{
    int currentRow = imageList->currentRow();
    if (currentRow < 0) return;
    
    // 删除列表项和图片
    delete imageList->takeItem(currentRow);
    images.erase(images.begin() + currentRow);
    imageNames.erase(imageNames.begin() + currentRow);
    
    btnMoveUp->setEnabled(imageList->count() > 1);
    btnMoveDown->setEnabled(imageList->count() > 1);
    btnRemove->setEnabled(imageList->count() > 0);
    btnGenerate->setEnabled(!images.empty());
    
    updatePreview();
}

void CocosAtlasWidget::updatePreview()
{
    if (images.empty()) {
        previewLabel->setText(tr("预览区域"));
        return;
    }
    
    std::vector<SpriteFrame> frames;
    cv::Mat preview = packImages(frames);
    
    // 检查预览图是否为空
    if (preview.empty()) {
        previewLabel->setText(tr("预览区域"));
        return;
    }
    
    // 确保预览标签有有效的尺寸
    if (previewLabel->width() <= 0 || previewLabel->height() <= 0) {
        return;
    }
    
    // 缩放预览图到合适大小
    double scale = std::min(
        double(previewLabel->width()) / preview.cols,
        double(previewLabel->height()) / preview.rows
    );
    
    cv::Mat scaled;
    cv::resize(preview, scaled, cv::Size(preview.cols * scale, preview.rows * scale), 0, 0, cv::INTER_AREA);
    
    QImage img = cvMatToQImage(scaled);
    if (!img.isNull()) {
        previewLabel->setPixmap(QPixmap::fromImage(img));
    } else {
        previewLabel->setText(tr("预览区域"));
    }
}

QImage CocosAtlasWidget::cvMatToQImage(const cv::Mat &mat)
{
    if (mat.empty()) return QImage();
    
    if (mat.type() == CV_8UC4) {
        cv::Mat converted;
        cv::cvtColor(mat, converted, cv::COLOR_BGRA2RGBA);
        return QImage(converted.data, converted.cols, converted.rows, 
            converted.step, QImage::Format_RGBA8888).copy();
    }
    
    return QImage();
}