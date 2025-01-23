#ifndef STYLECONVERTWIDGET_H
#define STYLECONVERTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <opencv2/opencv.hpp>
#include <QColorDialog>

class StyleConvertWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StyleConvertWidget(QWidget *parent = nullptr);

private slots:
    void onLoadImageClicked();
    void onCartoonClicked();
    void onPixelateClicked();
    void onSaveClicked();

private:
    QLabel *imageLabel;
    QLabel *previewLabel;
    QPushButton *btnLoadImage;
    QPushButton *btnCartoon;
    QPushButton *btnPixelate;
    QPushButton *btnSave;
    QPushButton *btnCartoonV2;  // 添加新按钮
    void cartoonV3(const cv::Mat &img, cv::Mat &result);  // 添加新的处理函数
    QPushButton *btnChangeColor;  // 添加新按钮
    void onChangeColorClicked();  // 添加新的槽函数

    cv::Mat currentImage;
    cv::Mat previewImage;

    void updatePreview(const cv::Mat &img);
    QImage cvMatToQImage(const cv::Mat &mat);
    void cartoonV2(const cv::Mat &img, cv::Mat &result);
    void pixelateImage(const cv::Mat &src, cv::Mat &result, int pixelSize = 5);
    private:
        void changePhotoBackground(const cv::Mat &src, cv::Mat &dst, const cv::Scalar &bgColor);
};

#endif // STYLECONVERTWIDGET_H