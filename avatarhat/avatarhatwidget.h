#ifndef AVATARHATWIDGET_H
#define AVATARHATWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>  // 添加头文件
#include <opencv2/opencv.hpp>

class AvatarHatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AvatarHatWidget(QWidget *parent = nullptr);
    
private slots:
    void onLoadAvatarClicked();
    void onLoadHatClicked();
    void onSaveClicked();
    void updatePreview();
    void onHatListContextMenu(const QPoint &pos);  // 只在这里声明一次

private:
    QLabel *imageLabel;
    QPushButton *btnLoadAvatar;
    QPushButton *btnLoadHat;
    QPushButton *btnSave;
    QListWidget *hatList;

    cv::Mat currentAvatar;
    std::vector<cv::Mat> hats;
    cv::Mat previewImage;

    void addHatToFaces(const cv::Mat &avatar, const cv::Mat &hat, cv::Mat &result);
    QImage cvMatToQImage(const cv::Mat &mat);
    // 删除这里的重复声明
};

#endif // AVATARHATWIDGET_H