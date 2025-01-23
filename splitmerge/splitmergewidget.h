#ifndef SPLITMERGEWIDGET_H
#define SPLITMERGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <opencv2/opencv.hpp>

class SplitMergeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SplitMergeWidget(QWidget *parent = nullptr);

private slots:
    void onLoadClicked();
    void onSplitClicked();
    void onMergeClicked();
    void onRemoveBackgroundClicked();  // 新增

private:
    QLabel *imageLabel;
    QPushButton *btnLoad;
    QPushButton *btnSplit;
    QPushButton *btnMerge;
    QPushButton *btnRemoveBackground;  // 新增
    cv::Mat currentImage;

    QImage cvMatToQImage(const cv::Mat &mat);
    cv::Mat QImageToCvMat(const QImage &image);
    std::vector<cv::Mat> splitImages(const cv::Mat &image);
    cv::Mat mergeImages(const std::vector<cv::Mat> &images, int cols);
};

#endif // SPLITMERGEWIDGET_H