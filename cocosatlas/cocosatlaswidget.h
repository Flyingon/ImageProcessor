#ifndef COCOSATLASWIDGET_H
#define COCOSATLASWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <opencv2/opencv.hpp>

struct SpriteFrame {
    QString name;
    cv::Rect rect;
    bool rotated;
    cv::Size originalSize;
    cv::Point2f offset;
};

class CocosAtlasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CocosAtlasWidget(QWidget *parent = nullptr);

private slots:
    void onLoadClicked();
    void onClearClicked();  // 新增
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onRemoveClicked();
    void onGenerateClicked();

private:
    QLabel *previewLabel;
    QListWidget *imageList;
    QPushButton *btnLoad;
    QPushButton *btnClear;  // 新增
    QPushButton *btnMoveUp;
    QPushButton *btnMoveDown;
    QPushButton *btnRemove;
    QPushButton *btnGenerate;
    
    std::vector<cv::Mat> images;
    std::vector<QString> imageNames;
    
    void updatePreview();
    QString generatePlist(const std::vector<SpriteFrame>& frames, const QString& textureName);
    cv::Mat packImages(std::vector<SpriteFrame>& frames);
    QImage cvMatToQImage(const cv::Mat &mat);
};

#endif // COCOSATLASWIDGET_H